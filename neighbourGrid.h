#ifndef NEIGHBOURGRID_H_INCLUDED

#define NEIGHBOURGRID_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <map>
#include <iostream>
#include <fstream>
#include "uset.h"
#include "grids.h"
#include "rowminlex.h"
#include "grid.h"

using namespace std;

extern int neighbourhood(const char* fname, const char* knownsfname);

struct neighbourGrid {
	lightweightUsetList us;
	unsigned int minWormholeSize;
	unsigned int maxWormholeSize;
	neighbourGrid(void);
};

class neighbourGrids : public map<ch81, neighbourGrid> {
public:
	unsigned int maxUA;
	unsigned int nMaxUA;
	neighbourGrid myself;
	ch81 digitsCanon;
	int authoMorphisms;
	neighbourGrids(void);
	void write(ostream &o);
	void findNeighbourGrids(grid &g);
};

#endif // NEIGHBOURGRID_H_INCLUDED
