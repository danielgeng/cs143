/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
	fprintf(stdout, "Bruinbase> ");

	// set the command line input and start parsing user input
	sqlin = commandline;
	sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
	// SqlParser.y by bison (bison is GNU equivalent of yacc)

	return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
	RecordFile rf;   // RecordFile containing the table
	RecordId   rid;  // record cursor for table scanning
	BTreeIndex bt;
	IndexCursor cursor;

	RC rc;
	int key;
	string value;
	int count = 0;
	int diff;

	// open the table file
	if((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}

	// scan the table file from the beginning
	rid.pid = rid.sid = 0;

	int min = 0, max = 0;
	bool cond_eq = false;

	if(bt.open(table + ".idx", 'r') == 0) {
		// go through where statement to find min and max key values if applicable
		for(unsigned i = 0; i < cond.size(); i++) {
			if(cond[i].attr == 1 && !cond_eq) {
				switch(cond[i].comp) {
				// eq -> only one key value, min = max
				case SelCond::EQ:
					min = max = atoi(cond[i].value);
					cond_eq = true;
					break;
				case SelCond::GT:
					if(min == 0)
						min = atoi(cond[i].value) + 1;
					else {
						int tmp = atoi(cond[i].value) + 1;
						if(tmp > min)
							min = tmp;
					}
					break;
				case SelCond::GE:
					if(min == 0)
						min = atoi(cond[i].value);
					else {
						int tmp = atoi(cond[i].value);
						if(tmp > min)
							min = tmp;
					}
					break;
				case SelCond::LT:
					if(max == 0)
						max = atoi(cond[i].value) - 1;
					else {
						int tmp = atoi(cond[i].value) - 1;
						if(tmp < max)
							max = tmp;
					}
					break;
				case SelCond::LE:
					if(max == 0)
						max = atoi(cond[i].value);
					else {
						int tmp = atoi(cond[i].value);
						if(tmp < max)
							max = tmp;
					}
					break;
				}
			}
		}

		// initialize btree cursor to minimum value and read forward
		bt.locate(min, cursor);

		while(bt.readForward(cursor, key, rid) == 0) {
			// count(*)
			if(attr == 4) {
				if(cond_eq && key != min) // done
					goto bt_exit_select;
				if(max != 0 && key > max) // done
					goto bt_exit_select;
				if(min != 0 && key < min) // done
					goto bt_exit_select;
				count++;
				goto bt_next_tuple;
			}

			// rest is based off the provided non-indexed select with slight modifications
			if((rc = rf.read(rid, key, value)) < 0) {
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				goto exit_select;
			}

			for(unsigned i = 0; i < cond.size(); i++) {
				switch(cond[i].attr) {
				case 1:
					diff = key - atoi(cond[i].value);
					break;
				case 2:
					diff = strcmp(value.c_str(), cond[i].value);
					break;
				}

				switch(cond[i].comp) {
				case SelCond::EQ:
					if(diff != 0) {
						if(cond[i].attr == 1) goto exit_select;
						goto bt_next_tuple;
					}
					break;
				case SelCond::NE:
					if(diff == 0) goto bt_next_tuple;
					break;
				case SelCond::GT:
					if(diff <= 0) goto bt_next_tuple;
					break;
				case SelCond::LT:
					// no point in going further with <= condition, remaining tuples will not be valid
					if(diff >= 0) {
						if(cond[i].attr == 1) goto exit_select;
						goto bt_next_tuple;
					}
					break;
				case SelCond::GE:
					if(diff < 0) goto bt_next_tuple;
					break;
				case SelCond::LE:
					// no point in going further with < condition
					if(diff > 0) {
						if(cond[i].attr == 1) goto exit_select;
						goto bt_next_tuple;
					}
					break;
				}
			}

			switch(attr) {
			case 1:  // SELECT key
				fprintf(stdout, "%d\n", key);
				break;
			case 2:  // SELECT value
				fprintf(stdout, "%s\n", value.c_str());
				break;
			case 3:  // SELECT *
				fprintf(stdout, "%d '%s'\n", key, value.c_str());
				break;
			}

			bt_next_tuple: 
				;
		}

	}
	// no index -> use old select
	else {
		while(rid < rf.endRid()) {
			// read the tuple
			if((rc = rf.read(rid, key, value)) < 0) {
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				goto exit_select;
			}

			// check the conditions on the tuple
			for(unsigned i = 0; i < cond.size(); i++) {
				// compute the difference between the tuple value and the condition value
				switch(cond[i].attr) {
				case 1:
					diff = key - atoi(cond[i].value);
					break;
				case 2:
					diff = strcmp(value.c_str(), cond[i].value);
					break;
				}

				// skip the tuple if any condition is not met
				switch(cond[i].comp) {
				case SelCond::EQ:
					if(diff != 0) goto next_tuple;
					break;
				case SelCond::NE:
					if(diff == 0) goto next_tuple;
					break;
				case SelCond::GT:
					if(diff <= 0) goto next_tuple;
					break;
				case SelCond::LT:
					if(diff >= 0) goto next_tuple;
					break;
				case SelCond::GE:
					if(diff < 0) goto next_tuple;
					break;
				case SelCond::LE:
					if(diff > 0) goto next_tuple;
					break;
				}
			}

			// the condition is met for the tuple.
			// increase matching tuple counter
			count++;

			// print the tuple
			switch(attr) {
			case 1:  // SELECT key
				fprintf(stdout, "%d\n", key);
				break;
			case 2:  // SELECT value
				fprintf(stdout, "%s\n", value.c_str());
				break;
			case 3:  // SELECT *
				fprintf(stdout, "%d '%s'\n", key, value.c_str());
				break;
			}

			// move to the next tuple
			next_tuple:
				++rid;
		}
	}

	bt_exit_select:

	// print matching tuple count if "select count(*)"
	if(attr == 4) {
		fprintf(stdout, "%d\n", count);
	}
	rc = 0;

	// close the table file and return
	exit_select:
	rf.close();
	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	RecordFile rf;
	RecordId rid;
	RC rc;
	BTreeIndex bt;

	string line, value;
	int key;

	ifstream file(loadfile.c_str());
	if(!file) {
		cerr << "File could not be opened...\n";
	}

	rc = rf.open(table + ".tbl", 'w');
	if(rc != 0)
		return rc;

	if(index) {
		rc = bt.open(table + ".idx", 'w');
		if(rc != 0)
			return rc;
	}

	while(getline(file, line)) {
		parseLoadLine(line, key, value);
		// cout << "inserting " << key << " " << value << endl;
		rc = rf.append(key, value, rid);
		if(rc != 0)
			return rc;
		if(index) {
			rc = bt.insert(key, rid);
			if(rc != 0)
				return rc;
		}
	}

	if(index) {
		rc = bt.close();
		if(rc != 0)
			return rc;
	}

	file.close();
	rf.close();

	return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
	const char *s;
	char        c;
	string::size_type loc;

	// ignore beginning white spaces
	c = *(s = line.c_str());
	while(c == ' ' || c == '\t') { c = *++s; }

	// get the integer key value
	key = atoi(s);

	// look for comma
	s = strchr(s, ',');
	if(s == NULL) { return RC_INVALID_FILE_FORMAT; }

	// ignore white spaces
	do { c = *++s; } while(c == ' ' || c == '\t');

	// if there is nothing left, set the value to empty string
	if(c == 0) {
		value.erase();
		return 0;
	}

	// is the value field delimited by ' or "?
	if(c == '\'' || c == '"') {
		s++;
	} else {
		c = '\n';
	}

	// get the value string
	value.assign(s);
	loc = value.find(c, 0);
	if(loc != string::npos) { value.erase(loc); }

	return 0;
}
