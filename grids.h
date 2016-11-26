#ifndef GRIDS_H_INCLUDED

#define GRIDS_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <map>
#include <set>
#include <iostream>
//#include <fstream>
//#include "grid.h"
#include "ch81.h"

using namespace std;

class refferenceList : public set<void*> {};

class indexByInt : public map<int, refferenceList> {};

struct puzzlesInGrid : public set<ch81> {
};

struct gridsWithPuzzles : public map<ch81, puzzlesInGrid> {
	indexByInt indexBySize;
	void addPuzzle(const char *p);
	void reindexBySize();
	void loadFromFile(const char *fname);
	void loadFromFile(FILE *file);
	//void saveToFile(const char *fname, const bool noPuz = false);
	void saveToFile(FILE *file, const bool noPuz = false, const bool verbose = false);
};

#endif // GRIDS_H_INCLUDED
