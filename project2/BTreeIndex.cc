/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"
#include <cstring>
#include <cmath>

using namespace std;

#define PAIR_SIZE_L (sizeof(RecordId) + sizeof(int))

#define PAIR_SIZE_NL (sizeof(PageId) + sizeof(int))

#define MAX_KEYS_L floor((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int)) / PAIR_SIZE_L)

#define MAX_KEYS_NL floor((PageFile::PAGE_SIZE - sizeof(PageId) - sizeof(int)) / PAIR_SIZE_NL)

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
    treeHeight = 0;
    memset(buffer, 0, PageFile::PAGE_SIZE);
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	RC err = pf.open(indexname, mode);
	if(err != 0) return err;

	if(pf.endPid() == 0){
		rootPid = -1;
		treeHeight = 0;
		err = pf.write(0, buffer);
		if(err != 0) return err;
		return 0;
	}

	err = pf.read(0, buffer);
	if(err != 0) return err;

	memcpy(&rootPid, buffer, sizeof(PageId));
	memcpy(&treeHeight, buffer + sizeof(PageId), sizeof(int));

    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    memcpy(buffer, &rootPid, sizeof(PageId));
    memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));

    RC err = pf.write(0, buffer);
    if(err != 0) return err;

    return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC err;

	// first node
	if(treeHeight == 0){
		BTLeafNode leaf;
		err = leaf.insert(key, rid);
		if(err != 0) return err;
		rootPid = pf.endPid();
		treeHeight++;
		return leaf.write(rootPid, pf);
	}

	// helpers for insertAndSplit operations
	int tmp_key;
	PageId tmp_pid;

	// use to see if we need to split nodes in recursive function
	bool overflow = false;
	err = insert_helper(key, rid, 1, rootPid, tmp_key, tmp_pid, overflow);

	// overflow will only be true at this point if a new root is needed
	if(overflow){
		BTNonLeafNode parent;
		parent.initializeRoot(rootPid, tmp_key, tmp_pid);
		rootPid = pf.endPid();
		treeHeight++;
		return parent.write(rootPid, pf);
	}

	return err;
}

RC BTreeIndex::insert_helper(int &key, const RecordId &rid, int height, PageId curr_pid, int &tmp_key, PageId &tmp_pid, bool &overflow)
{	
	RC err;
	// leaf level
	if(height == treeHeight){
		BTLeafNode leaf;
		err = leaf.read(curr_pid, pf);
		if(err != 0) return err;

		// easy case: simple insert on leaf level
		if(leaf.getKeyCount() < MAX_KEYS_L){
			err = leaf.insert(key, rid);
			if(err != 0) return err;
			err = leaf.write(curr_pid, pf);
			if(err != 0) return err;

			return 0;
		}

		// overflow: call insertAndSplit, tmp_key and tmp_pid are stored to move up to parent level
		// the overflow variable will propagate up until it is resolved (either in insert_helper or insert)
		BTLeafNode sib;
		leaf.insertAndSplit(key, rid, sib, tmp_key);
		tmp_pid = pf.endPid();
		leaf.setNextNodePtr(tmp_pid);

		err = leaf.write(curr_pid, pf);
		if(err != 0) return err;
		err = sib.write(tmp_pid, pf);
		if(err != 0) return err;

		overflow = true;
		return 0;
	}
	// not at leaf level yet
	BTNonLeafNode nonleaf;
	err = nonleaf.read(curr_pid, pf);
	if(err != 0) return err;

	// move down to the correct child node of non-leaf node
	PageId child_pid;
	err = nonleaf.locateChildPtr(key, child_pid);
	if(err != 0) return err;
	// update height and recurse
	err = insert_helper(key, rid, height+1, child_pid, tmp_key, tmp_pid, overflow);
	if(err != 0) return err;

	// overflow case
	if(overflow){
		// overflow can be fixed at this level
		if(nonleaf.getKeyCount() < MAX_KEYS_NL){
			err = nonleaf.insert(tmp_key, tmp_pid);
			if(err != 0) return err;
			err = nonleaf.write(curr_pid, pf);
			if(err != 0) return err;

			overflow = false;
			return 0;
		}
		// overflow cannot be fixed at this level
		BTNonLeafNode sib;
		int sib_key;
		err = nonleaf.insertAndSplit(tmp_key, tmp_pid, sib, sib_key);
		if(err != 0) return err;

		// store tmp_key and tmp_pid to move up to parent level
		tmp_key = sib_key;
		tmp_pid = pf.endPid();
		err = nonleaf.write(curr_pid, pf);
		if(err != 0) return err;
		err = sib.write(tmp_pid, pf);
		if(err != 0) return err;
	}

	return 0;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	BTNonLeafNode nonleaf;
	RC err;
	PageId pid = rootPid;

	// iterate until leaf level
	for(int i = 1; i < treeHeight; i++){
		err = nonleaf.read(pid, pf);
		if(err != 0) return err;

		err = nonleaf.locateChildPtr(searchKey, pid);
		if(err != 0) return err;
	}

	BTLeafNode leaf;

	err = leaf.read(pid, pf);
	if(err != 0) return err;

	int eid;
	err = leaf.locate(searchKey, eid); // get max key if still smaller than searchKey
	// if(err != 0) return err;

	cursor.eid = eid;
	cursor.pid = pid;

    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	BTLeafNode leaf;

	RC err = leaf.read(cursor.pid, pf);
	if(err != 0) return err;

	if(cursor.pid <= 0)
		return RC_INVALID_CURSOR;

	err = leaf.readEntry(cursor.eid, key, rid);
	if(err != 0) return err;

	// move cursor to the next leaf node if at the end
	if(cursor.eid + 1 >= leaf.getKeyCount()){
		cursor.eid = 0;
		cursor.pid = leaf.getNextNodePtr();
	}
	else
		cursor.eid++;

    return 0;
}
