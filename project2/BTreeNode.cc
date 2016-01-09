#include "BTreeNode.h"
#include <cstring>
#include <cmath>
#include <cstdlib>

using namespace std;

// (recordid, key) pair in leaf nodes
#define PAIR_SIZE_L (sizeof(RecordId) + sizeof(int))

// (pageid, key) pair in non-leaf nodes
#define PAIR_SIZE_NL (sizeof(PageId) + sizeof(int))

// max number of keys in a leaf node (84)
// size - pageid to next leaf - key count divided by size of each entry
#define MAX_KEYS_L floor((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int)) / PAIR_SIZE_L)

// max number of keys in a non-leaf node (127)
// size - first page id - key count divided by size of each entry
#define MAX_KEYS_NL floor((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int)) / PAIR_SIZE_NL)

// structure: num_keys | key | recordid | key | recordid | ... | pageid to next leaf
BTLeafNode::BTLeafNode(){
	memset(buffer, 0, PageFile::PAGE_SIZE);
	int num_keys = 0;
	memcpy(buffer, &num_keys, sizeof(int));
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf){ 
	return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf){
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount(){
	int retval = 0;
 	memcpy(&retval, buffer, sizeof(int));
 	return retval;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid){ 
	int num_keys = getKeyCount();
	if(num_keys >= MAX_KEYS_L)
		return RC_NODE_FULL;

	// iterate through to find correct index for new entry
	char *itr = buffer + sizeof(int);
	int idx = sizeof(int);
	for(idx; idx < PageFile::PAGE_SIZE - PAIR_SIZE_L; idx+=PAIR_SIZE_L){
		int current_key = 0;
		memcpy(&current_key, itr, sizeof(int));
		if(current_key >= key || current_key == 0)
			break;
		itr += PAIR_SIZE_L;
	}

	// create temp buffer
	char *tmp = (char*)malloc(PageFile::PAGE_SIZE);
	memset(tmp, 0, PageFile::PAGE_SIZE);

	// copy over everything from 0 to the index of the new entry to temp
	memcpy(tmp, buffer, idx);

	// add the new entry to temp
	memcpy(tmp+idx, &key, sizeof(int));
	memcpy(tmp+idx+sizeof(int), &rid, sizeof(RecordId));

	// copy the second half of buffer after the new entry
	memcpy(tmp+idx+PAIR_SIZE_L, buffer+idx, num_keys * PAIR_SIZE_L + sizeof(int) - idx);

	// copy over the page id to buffer (not done before because it would be more difficult to calculate empty nodes)
	PageId next_node = getNextNodePtr();
	memcpy(tmp+PageFile::PAGE_SIZE - sizeof(PageId), &next_node, sizeof(PageId));

	// copy temp into buffer
	memcpy(buffer, tmp, PageFile::PAGE_SIZE);

	// increment count in buffer
	num_keys++;
	memcpy(buffer, &num_keys, sizeof(int));

	free(tmp);

	return 0; 
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey){ 
	int num_keys = getKeyCount();
	if(num_keys < MAX_KEYS_L || sibling.getKeyCount() != 0)
		return RC_INVALID_ATTRIBUTE;

	// clear rhs buffer as precaution
	memset(sibling.buffer, 0, PageFile::PAGE_SIZE);

	int half_count = ceil(num_keys/2); 	// number of entries in lhs buffer
	int mid_idx = half_count * PAIR_SIZE_L + sizeof(int); 	// index after the last node of lhs
	int sib_count = num_keys - half_count; 	// number of entries in rhs buffer

	// copy everything from mid index of lhs into rhs
	memcpy(sibling.buffer + sizeof(int), buffer + mid_idx, PageFile::PAGE_SIZE - mid_idx - sizeof(PageId));

	// set next node pointer
	sibling.setNextNodePtr(getNextNodePtr());

	// delete the copied items from lhs
	memset(buffer + mid_idx, 0, PageFile::PAGE_SIZE - mid_idx - sizeof(PageId));

	// update count for both lhs and rhs
	memcpy(buffer, &half_count, sizeof(int));
	memcpy(sibling.buffer, &sib_count, sizeof(int));

	// check which side to add the new entry
	int sib_first;
	memcpy(&sib_first, sibling.buffer + sizeof(int), sizeof(int));
	if(key >= sib_first)
		sibling.insert(key, rid);
	else
		insert(key, rid);

	// first key of rhs
	memcpy(&siblingKey, sibling.buffer + sizeof(int), sizeof(int));

	return 0;
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid){ 
	int num_keys = getKeyCount();
	char *itr = buffer + sizeof(int);

	// iterate through
	for(int i = 0; i < num_keys; i++){
		int current_key;
		memcpy(&current_key, itr, sizeof(int));
		if(current_key >= searchKey){
			eid = i;
			return 0;
		}
		itr += PAIR_SIZE_L;
	}

	eid = num_keys;
	return RC_NO_SUCH_RECORD;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid){ 
	if(eid >= getKeyCount() || eid < 0)
		return RC_NO_SUCH_RECORD;

	memcpy(&key, buffer + sizeof(int) + (eid * PAIR_SIZE_L), sizeof(int));
	memcpy(&rid, buffer + sizeof(int) + (eid * PAIR_SIZE_L) + sizeof(int), sizeof(RecordId));

	return 0; 
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr(){
	PageId retval;
	memcpy(&retval, buffer + PageFile::PAGE_SIZE - sizeof(PageId), sizeof(PageId));
	return retval; 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid){
	if (pid < 0)
		return RC_INVALID_PID;
	memcpy(buffer + PageFile::PAGE_SIZE - sizeof(PageId), &pid, sizeof(PageId));
	return 0;
}

// structure: num_keys | pageid | key | pageid | key | pageid .. etc ..
BTNonLeafNode::BTNonLeafNode(){
	memset(buffer, 0, PageFile::PAGE_SIZE);
	int num_keys = 0;
	memcpy(buffer, &num_keys, sizeof(int));
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf){ 
	return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf){ 
	return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount(){ 
	int retval = 0;
 	memcpy(&retval, buffer, sizeof(int));
 	return retval;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid){
	int num_keys = getKeyCount();
	if(num_keys >= MAX_KEYS_NL)
		return RC_NODE_FULL;

	// skip count and first pageid
	char *itr = buffer + PAIR_SIZE_NL;
	int idx = PAIR_SIZE_NL;

	// find where to insert
	for(idx; idx < PageFile::PAGE_SIZE - PAIR_SIZE_NL; idx += PAIR_SIZE_NL){
		int current_key = 0;
		memcpy(&current_key, itr, sizeof(int));
		if(current_key >= key || current_key == 0)
			break;
		itr += PAIR_SIZE_NL;
	}

	// temp buffer
	char *tmp = (char*)malloc(PageFile::PAGE_SIZE);
	memset(tmp, 0, PageFile::PAGE_SIZE);

	// copy all from 0 to idx to temp
	memcpy(tmp, buffer, idx);

	// add the new entry
	memcpy(tmp+idx, &key, sizeof(int));
	memcpy(tmp+idx+sizeof(int), &pid, sizeof(PageId));

	// copy the rest after the new entry
	memcpy(tmp+idx+PAIR_SIZE_NL, buffer+idx, ((num_keys + 1) * PAIR_SIZE_NL) - idx + sizeof(PageId));

	// copy temp to buffer
	memcpy(buffer, tmp, PageFile::PAGE_SIZE);

	// update count
	num_keys++;
	memcpy(buffer, &num_keys, sizeof(int));

	free(tmp);
	return 0; 
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey){ 
	int num_keys = getKeyCount();
	if(num_keys < MAX_KEYS_NL || sibling.getKeyCount() != 0)
		return RC_INVALID_ATTRIBUTE;

	memset(sibling.buffer, 0, PageFile::PAGE_SIZE);
	int half_count =  ceil(num_keys/2);
	int mid_idx = (half_count * PAIR_SIZE_NL) + PAIR_SIZE_NL;

	// store first key of rhs node and last key of lhs node
	int lhs_last, rhs_first;
	memcpy(&lhs_last, buffer + mid_idx - PAIR_SIZE_NL, sizeof(int));
	memcpy(&rhs_first, buffer + mid_idx, sizeof(int));

	// store lhs_last in midKey to move to parent, insert new key in buffer
	if(key < lhs_last){
		midKey = lhs_last;

		// copy to rhs
		memcpy(sibling.buffer + PAIR_SIZE_NL, buffer + mid_idx, PageFile::PAGE_SIZE - mid_idx);
		memcpy(sibling.buffer + sizeof(int), buffer + mid_idx - sizeof(PageId), sizeof(PageId)); // first pageid of sibling

		// delete midKey and everything after it from buffer
		memset(buffer + mid_idx - PAIR_SIZE_NL, 0, PageFile::PAGE_SIZE - mid_idx - PAIR_SIZE_NL);

		// update key counts
		int sib_count = num_keys - half_count;
		num_keys = half_count - 1;
		memcpy(sibling.buffer, &sib_count, sizeof(int));
		memcpy(buffer, &num_keys, sizeof(int));

		insert(key, pid);
	}

	// store rhs_first in midKey to move to parent, insert new key in sibling
	else if(key > rhs_first){
		midKey = rhs_first;

		// copy to rhs
		memcpy(sibling.buffer + PAIR_SIZE_NL, buffer + mid_idx + PAIR_SIZE_NL, PageFile::PAGE_SIZE - mid_idx - PAIR_SIZE_NL);
		memcpy(sibling.buffer + sizeof(int), buffer + mid_idx + sizeof(int), sizeof(PageId));

		// delete midKey and everything after it from buffer
		memset(buffer + mid_idx, 0, PageFile::PAGE_SIZE - mid_idx);

		// update key counts
		int sib_count = num_keys - half_count - 1;
		memcpy(sibling.buffer, &sib_count, sizeof(int));
		memcpy(buffer, &half_count, sizeof(int));

		sibling.insert(key, pid);
	}

	// new key should be moved to parent
	else{
		midKey = key;

		// copy to rhs
		memcpy(sibling.buffer + PAIR_SIZE_NL, buffer + mid_idx, PageFile::PAGE_SIZE - mid_idx);

		// first pageid of sibling is the newly inserted pageid
		memcpy(sibling.buffer + sizeof(int), &pid, sizeof(PageId));

		// delete copied data from buffer
		memset(buffer + mid_idx, 0, PageFile::PAGE_SIZE - mid_idx);

		// update key counts
		int sib_count = num_keys - half_count;
		memcpy(sibling.buffer, &sib_count, sizeof(int));
		memcpy(buffer, &half_count, sizeof(int));
	}

	return 0; 
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid){ 
	int num_keys = getKeyCount();
	// skip count and first pageid
	char *itr = buffer + PAIR_SIZE_NL;
	char idx = PAIR_SIZE_NL;
	for(int i = 0; i < num_keys; i++){
		int current_key;
		memcpy(&current_key, itr, sizeof(int));

		// return right page of last key
		if(i == num_keys - 1 && current_key < searchKey){
			memcpy(&pid, itr + sizeof(int), sizeof(PageId));
			return 0;
		}

		// return left page of current key
		if(current_key >= searchKey){
			memcpy(&pid, itr - sizeof(PageId), sizeof(PageId));
			return 0;
		}
		itr += PAIR_SIZE_NL;
	}
	return RC_NO_SUCH_RECORD;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2){ 
	// make sure the node doesn't have anything
	if(getKeyCount() != 0)
		return RC_INVALID_ATTRIBUTE;
	memset(buffer, 0, PageFile::PAGE_SIZE);

	// (size) (pageid 1) (key) (pageid 2)
	memcpy(buffer + sizeof(int), &pid1, sizeof(PageId));
	memcpy(buffer + PAIR_SIZE_NL, &key, sizeof(int));
	memcpy(buffer + PAIR_SIZE_NL + sizeof(int), &pid2, sizeof(PageId));
	int num_keys = 1;
	memcpy(buffer, &num_keys, sizeof(int));

	return 0; 
}
