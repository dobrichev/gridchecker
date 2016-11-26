#ifndef GRID_H_INCLUDED

#define GRID_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <time.h>
#include "uset.h"

#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

void findUaBySolutions(const char *solList, const int nSol);

class grid;

class uaCollector {
public:
	uaCollector(grid *theGrid, const unsigned char *unknowns, const int unknownsSize);
	//uaCollector(grid *theGrid);
	~uaCollector();
	//void byMorphs();
	bool addSolution(const char *sol);
	//bool addMorph(const char *sol);
	unsigned long long runSolver();
	void addUaToGrid();
	//static const int maxSolutions = 1000000;
	static const int maxSolutions = 200000;
	grid &g;
	const unsigned char *unknowns;
	const int unknownsSize;
	int uniqueSize;
	int uaMaxSize;
	int firstInvalidUa;
	int numInvalidUas;
	bool uaLimitedBySize;
	bm128 *uaArray;
};

class grid {
public:
	static const char* fileUASuffix;
	static const char* filePuzSuffix;
	static const char* fileDigitSuffix;
	static const char* file2dSuffix;
	static const char* file3dSuffix;
	static const char* fileBoxSuffix;
	static const char* file2bSuffix;
	static const char* file3bSuffix;
	static const char* fileBandSuffix;
	char digits[81];
	int gridBM[81];
	usetListBySize usetsBySize;
	clique maxClique;
	//uset digitSets[512]; //any combination of labels 1..9
	//lightweightCliqueList maximalCliques;
	//lightweightCliqueList maximumCliques;
	//int mcn;
	//uset *allUAarray;
	const char* fname;
	//grid();
	//~grid() {};
	void findUA2digits();
	void findUA4cells();
	void findUA6cells();
	unsigned long long findUaBySolving(const unsigned char *unknowns, const int unknownsSize);
	//void unused_findComplementaryUA();
	void findUA4digits();
	void findUA5digits();
	//void findUAbyMorph();
	void findUA5boxes();
	void findShortUA();
	unsigned long long findUAbyPuzzle(const char *puz);
	unsigned long long findUAinPuzzle(const char *puz);
	void findUAbyPuzzleBox(const char *puz);
	void findUArandom(const char *puz = NULL, int nCells = -1, int nAttempts = -1);
	void findUA2bands();
	void findUA12();
	void findInitialUA();
//	void unused_findMoreUA();
//	NOINLINE void findCliques(lightweightUsetList &foundUA);

//	NOINLINE void findCliques(const lightweightUsetList &introducedUA, lightweightUsetList &foundUA);
//	NOINLINE void findCliques_(const lightweightUsetList &introducedUA, lightweightUsetList &foundUA);

	void findMaximalCliques();
	void findMaximalCliques(clique &theClique, usetListBySize &usetsBySize);

	//bool unused_findMCN(const bm128 *ualist, const int uasize, const int above) const;

	void saveUA();
	void readUAFromFile();
	int readFromFile();
	void readUAFromFileOnly();
	int fromString(const char* buf);
	void MinimizeUA(uset &ua) const;
	//void unused_printStatistics();
	//int unused_getDigitMask(const uset &s) const;
	//void unused_setDigitMasks();
	//void unused_calculateDigitSets ();
	void ua2puzzle(const uset &s, char *puzzle) const;
	void ua2InvariantPuzzle(const uset &s, char *puzzle) const;
	unsigned long long getSolutionCount(const uset &s) const;
	unsigned long long getSolutionCount(const uset &s, const int maxSolutions) const;
	bool isUA(const uset &s) const;
	NOINLINE bool isMinimalUA(const uset &s) const;
	//void unused_checkUA();
	int checkPuzSetValidity(const bm128 &commonClues, const bm128 *p, const int nPuz, char *res) const;
	int checkPuzSetValidity(const bm128 *p, const int nPuz, char *res) const;
	void generatePuzzle(char *p) const;
	void getBackBone();
	static void toString(const char *dig, char *buf);
	void toString(char *buf) const;
	void setBM();
};

#endif // GRID_H_INCLUDED
