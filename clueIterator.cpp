#define _CRT_SECURE_NO_DEPRECATE

#include <emmintrin.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <set>
#include <iostream>
#include "solver.h"
#include "tables.h"
#include "mm_allocator.h"
#include "t_128.h"
#include "uset.h"
#include "grid.h"
#include "clueIterator.h"
#include "rowminlex.h"
#include "ch81.h"
#include "options.h"

using namespace std;

unsigned long long z0=0, z1=0, z2=0, z3=0, z4=0, z5=0;

#ifndef NUM_PACKS
   #define NUM_PACKS 6 // use up to 768 unavoidables by default
#endif

clueIterator::clueIterator(grid &g) : g(g) {
	initProgress();
	nPuzzles = 0;
	nChecked = 0;
	//progressUpdate = 0;
	//progressCounter = 0;
	//skipProgress = false;
	numUaSizeLimit = 1500;
	numUaTotalLimit = 2000;
	for(int i = 0; i < 81; i++) {
		state[i].maxPositionLimitsByUA = 81;
		//skipped[i] = 0; //debug
	}
}

void clueIterator::remap() {
	//setup the mapper so that cells become in the following order:
	//<fixed non-clues><fixed clues><non-clique cells><largest UA from the clique><smallest UA from the clique>
	//<        fixed part          ><                              part to iterate                            >

	for(int i = 0; i < 81; i++) {
		state[i].maxPositionLimitsByUA = 81 - theClique.fixedNonClues.nbits;
	}
	int pos = 0; //start from the rightmost bits
	for(int i = 0; i <= theClique.size; i++) {
		//the last element contain rest of the cells w/o fixed clues
		for(int j = 0; j < theClique.ua[i].nbits; j++) {
			theMapper.map(theClique.ua[i].positions[j], theClique.ua[i].mappedTo[j]);
			pos++;
		}
		if(i < theClique.size)
			state[i].maxPositionLimitsByUA = pos;
	}
	//place the fixed non-clues at leftmost
	for(int i = 0; i < theClique.fixedNonClues.nbits; i++) {
		theMapper.map(theClique.fixedNonClues.positions[i], 80 - i);
	}
	//then place the fixed clues
	for(int i = 0; i < theClique.fixedClues.nbits; i++) {
		theMapper.map(theClique.fixedClues.positions[i], 80 - theClique.fixedNonClues.nbits - i);
	}
	//finally remap the whole list of UA sets
	remapUA();
}

void clueIterator::remapUA() {
	usets.clear();
	for(usetListBySize::const_iterator s = usetsBySize.begin(); s != usetsBySize.end(); s++) {
		uset u;
		u.nbits = s->nbits;
		for(int k = 0; k < s->nbits; k++) {
			u.positions[k] = theMapper.g2i[s->positions[k]];
		}
		u.bitmapByPositions();
		u.positionsByBitmap(); //reorder positions
		usets.insert(u);
	}
}

void clueIterator::removeLargeUA() {
	//const int CountLimit = 200 * NUM_PACKS;
	////Choose UA size upper limit so the number of UA sets up to this size to be >= NUM_PACKS * 128
	//if(usetsBySize.size() <= CountLimit)
	//	return; //nothing to truncate
	int uaCount = 0;
	int maxSize = 1;
	for(int i = 1; i < 81; i++) {
		if(uaCount + usetsBySize.distributionBySize[i] > numUaSizeLimit) {
			break; //use uaCount and maxSize from previous iteration
		}
		maxSize = i;
		uaCount += usetsBySize.distributionBySize[i];
	}
	int rest = numUaTotalLimit - uaCount;
	int leftmost = 0;
	for(usetList::iterator s = usets.begin(); s != usets.end();) {
		if(s->nbits > maxSize) {
			////if(s->positions[0] >= state[4].maxPositionLimitsByUA) {
			if(rest > 0) {
				rest--;
				leftmost++;
				s++;
				continue;
			}
			//s = usets.erase(s);
			usetList::iterator ss = s;
			s++;
			usets.erase(ss);
		}
		else
			s++;
	}
	if(opt.verbose) {
		printf("\nUsing %d UA, %d of size up to %d, and leftmost %d regardless of the size.\n", static_cast <int>(usets.size()), uaCount, maxSize, leftmost);
	}
}

void clueIterator::iterateBoxes(const int numClues) {
	huntClues = numClues;
	int boxMinimalClues[9];
	int boxPuzzles[9];
	char buf[2000];
	sprintf(buf, "%s%s", g.fname, g.fileBoxSuffix);
	puzFile = fopen(buf, "at");
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	//skipProgress = true;
	for(int box = 0; box < 9; box++) {
		printf("\n========= Processing box %d ==========\n", box + 1);
		theClique.clear();
		theClique.fixedClues.clear();
		for(int i = 0; i < 81; i++) {
			if(boxByCellIndex[i] == box) {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();

		prepareGrid();

		//copy UA bitmaps and LSB to arrays for faster access
		int nsets = static_cast <int>(usets.size());
		nAllUA = 0;
		allUA = bm128::allocate(nsets);
		uaLSB = (int*)malloc(nsets * sizeof(int));
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			uaLSB[nAllUA] = s->positions[0];
			allUA[nAllUA++] = s->bitmap128;
		}

		//find the lower limit of clues to complete the grid to unique solution
		nClues = 10 + 9; //start with (10 + 1) + 9 = 20 clues
		nPuzzles = 0;
		while(nPuzzles == 0) {
			nClues++;
			nChecked = 0;
			//progressUpdate = 0;
			state[nClues].cluePosition = 81;
			//set the 9 fixed clues
			for(int i = 0; i < theClique.fixedClues.nbits; i++) {
				state[nClues - 1 - i].cluePosition = 80 - i;
			}
			printf("\nStarting enumeration of valid puzzles of size 9+%d for box %d fixed.\n", nClues - 9, box + 1);
			//init local timer
			start = clock();
			//start from leftmost (most significant) clue
			clueNumber = nClues - theClique.fixedClues.nbits;
			//enumerate all possibilities up to the fixed clues which are placed at left side
			fprintf(puzFile, "Box %d, Clues %d\n", box + 1, nClues - 9);
			processChunks();
			boxMinimalClues[box] = nClues - 9;
			boxPuzzles[box] = nPuzzles;
			printf("\nSearched for 9+%d, generated %llu puzzles, found %u valid.\n", nClues - 9, nChecked, nPuzzles);
			printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);
		}

		free(uaLSB);
		bm128::deallocate(allUA);
	}
	//print results
	printf("\n==========\n");
	fprintf(puzFile, "\n==========\n");
	for(int box = 0; box < 9; box++) {
		printf("Box %d, MinClues %d, NumPuzzles %d\n", box + 1, boxMinimalClues[box], boxPuzzles[box]);
		fprintf(puzFile, "Box %d, MinClues %d, NumPuzzles %d\n", box + 1, boxMinimalClues[box], boxPuzzles[box]);
	}
	if(minimizedPuzzles.size()) {
		printf("%d 17-s found.\n", (int)minimizedPuzzles.size());
		fprintf(puzFile, "%d 17-s found.\n", (int)minimizedPuzzles.size());
		for(puzTextSet::const_iterator p = minimizedPuzzles.begin(); p != minimizedPuzzles.end(); p++) {
			printf("%81.81s\n", p->chars);
			fprintf(puzFile, "%81.81s\n", p->chars);
		}
	}
	fclose(puzFile);
}
void clueIterator::iterateDigits(const int numClues) {
	huntClues = numClues;
	int boxMinimalClues[9];
	int boxPuzzles[9];
	char buf[2000];
	sprintf(buf, "%s%s", g.fname, g.fileDigitSuffix);
	puzFile = fopen(buf, "at");
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	//skipProgress = true;
	for(int box = 0; box < 9; box++) {
		printf("\n========= Processing digit %d ==========\n", box + 1);
		theClique.clear();
		theClique.fixedClues.clear();
		for(int i = 0; i < 81; i++) {
			if(g.digits[i] == box + 1) {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();

		prepareGrid();

		//copy UA bitmaps and LSB to arrays for faster access
		int nsets = static_cast <int>(usets.size());
		nAllUA = 0;
		allUA = bm128::allocate(nsets);
		//allUA = (bm128 *)_mm_malloc(nsets * sizeof(bm128), 16);
		uaLSB = (int*)malloc(nsets * sizeof(int));
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			uaLSB[nAllUA] = s->positions[0];
			allUA[nAllUA++] = s->bitmap128;
		}

		//find the lower limit of clues to complete the grid to unique solution
		nClues = 10 + 9; //start with (10 + 1) + 9 = 20 clues
		nPuzzles = 0;
		while(nPuzzles == 0) {
			nClues++;
			nChecked = 0;
			//progressUpdate = 0;
			state[nClues].cluePosition = 81;
			//set the 9 fixed clues
			for(int i = 0; i < theClique.fixedClues.nbits; i++) {
				state[nClues - 1 - i].cluePosition = 80 - i;
			}
			printf("\nStarting enumeration of valid puzzles of size 9+%d for digit %d fixed.\n", nClues - 9, box + 1);
			//init local timer
			start = clock();
			//start from leftmost (most significant) clue
			clueNumber = nClues - theClique.fixedClues.nbits;
			//enumerate all possibilities up to the fixed clues which are placed at left side
			fprintf(puzFile, "Digit %d, Clues %d\n", box + 1, nClues - 9);
			//iterateClue(allUA, uaLSB, nAllUA);
			processChunks();
			boxMinimalClues[box] = nClues - 9;
			boxPuzzles[box] = nPuzzles;
			printf("\nSearched for 9+%d, generated %llu puzzles, found %u valid.\n", nClues - 9, nChecked, nPuzzles);
			printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);
		}

		free(uaLSB);
		//_mm_free(allUA);
		bm128::deallocate(allUA);
	}
	//print results
	printf("\n==========\n");
	fprintf(puzFile, "\n==========\n");
	for(int box = 0; box < 9; box++) {
		printf("Digit %d, MinClues %d, NumPuzzles %d\n", box + 1, boxMinimalClues[box], boxPuzzles[box]);
		fprintf(puzFile, "Digit %d, MinClues %d, NumPuzzles %d\n", box + 1, boxMinimalClues[box], boxPuzzles[box]);
	}
	if(minimizedPuzzles.size()) {
		printf("%d 17-s found.\n", (int)minimizedPuzzles.size());
		fprintf(puzFile, "%d 17-s found.\n", (int)minimizedPuzzles.size());
		for(puzTextSet::const_iterator p = minimizedPuzzles.begin(); p != minimizedPuzzles.end(); p++) {
			printf("%81.81s\n", p->chars);
			fprintf(puzFile, "%81.81s\n", p->chars);
		}
	}
	fclose(puzFile);
}

void clueIterator::iterate2Boxes(const int numClues) {
	huntClues = numClues;
	int dMinimalClues[9][9];
	int dPuzzles[9][9];
	char buf[2000];
	sprintf(buf, "%s%s", g.fname, g.file2bSuffix);
	puzFile = fopen(buf, "at");
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	minimizedPuzzles.clear();
	for(int d1 = 0; d1 < 8; d1++) for(int d2 = d1 + 1; d2 < 9; d2++) {
		printf("\n========= Processing box pair (%d,%d) ==========\n", d1 + 1, d2 + 1);
		theClique.clear();
		theClique.fixedClues.clear();
		for(int i = 0; i < 81; i++) {
			if(boxByCellIndex[i] == d1 || boxByCellIndex[i] == d2) {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();

		prepareGrid();

		//copy UA bitmaps and LSB to arrays for faster access
		int nsets = static_cast <int>(usets.size());
		nAllUA = 0;
		allUA = bm128::allocate(nsets);
		//allUA = (bm128 *)_mm_malloc(nsets * sizeof(bm128), 16);
		uaLSB = (int*)malloc(nsets * sizeof(int));
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			uaLSB[nAllUA] = s->positions[0];
			allUA[nAllUA++] = s->bitmap128;
		}

		//find the lower limit of clues to complete the grid to unique solution
		nClues = 9 + 18; //start with (9 + 1) + 18 = 28 clues
		//nClues = 12 + 18; //start with (9 + 1) + 18 = 28 clues
		nPuzzles = 0;
		while(nPuzzles == 0 /*&& nClues < 18+13*/) {
			nClues++;
			nChecked = 0;
			//progressUpdate = 0;
			state[nClues].cluePosition = 81;
			//set the 18 fixed clues
			for(int i = 0; i < theClique.fixedClues.nbits; i++) {
				state[nClues - 1 - i].cluePosition = 80 - i;
			}
			printf("\nStarting enumeration of valid puzzles of size 18+%d for boxes (%d,%d) fixed.\n", nClues - 18, d1 + 1, d2 + 1);
			//init local timer
			start = clock();
			//start from leftmost (most significant) clue
			clueNumber = nClues - theClique.fixedClues.nbits;
			//enumerate all possibilities up to the fixed clues which are placed at left side
			fprintf(puzFile, "Boxes (%d,%d), Clues %d\n", d1 + 1, d2 + 1, nClues - 18);
			processChunks();
			dMinimalClues[d1][d2] = nClues - 18;
			dPuzzles[d1][d2] = nPuzzles;
			printf("\nSearched for 18+%d, generated %llu puzzles, found %u valid.\n", nClues - 18, nChecked, nPuzzles);
			printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);
		}
		free(uaLSB);
		//_mm_free(allUA);
		bm128::deallocate(allUA);
	}
	//print results
	printf("\n==========\n");
	fprintf(puzFile, "\n==========\n");
	for(int d1 = 0; d1 < 8; d1++) for(int d2 = d1 + 1; d2 < 9; d2++) {
		printf("Box pair (%d,%d), MinClues %d, NumPuzzles %d\n", d1 + 1, d2 + 1, dMinimalClues[d1][d2], dPuzzles[d1][d2]);
		fprintf(puzFile, "Box pair (%d,%d), MinClues %d, NumPuzzles %d\n", d1 + 1, d2 + 1, dMinimalClues[d1][d2], dPuzzles[d1][d2]);
	}

	if(minimizedPuzzles.size()) {
		printf("%d 17-s found.\n", (int)minimizedPuzzles.size());
		fprintf(puzFile, "%d 17-s found.\n", (int)minimizedPuzzles.size());
		for(puzTextSet::const_iterator p = minimizedPuzzles.begin(); p != minimizedPuzzles.end(); p++) {
			printf("%81.81s\n", p->chars);
			fprintf(puzFile, "%81.81s\n", p->chars);
		}
	}

	fclose(puzFile);
}

void clueIterator::iterate2digits(const int numClues) {
	huntClues = numClues;
	int dMinimalClues[9][9];
	int dPuzzles[9][9];
	char buf[2000];
	sprintf(buf, "%s%s", g.fname, g.file2dSuffix);
	//pseudoPuzFile = fopen(buf, "at");
	puzFile = fopen(buf, "at");
	//fprintf(puzFile, "\nStart processing at %s\n", opt.getStartTime());
	//sprintf(buf, "%s.%d%s", g.fname, nClues, g.filePuzSuffix);
	//puzFile = fopen(buf, "at");
	g.toString(buf);
	fprintf(puzFile, "\nGrid:\n%81.81s\n", buf);
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	minimizedPuzzles.clear();
	int maxClues = 14 + 18;
	if(opt.scanOpt->veryfast17) {
		maxClues = 11 + 18;
	}
	for(int d1 = 0; d1 < 8; d1++) for(int d2 = d1 + 1; d2 < 9; d2++) {
		//printf("\n========= Processing digit pair (%d,%d) ==========\n", d1 + 1, d2 + 1);
		theClique.clear();
		theClique.fixedClues.clear();
		for(int i = 0; i < 81; i++) {
			if(g.digits[i] == d1 + 1 || g.digits[i] == d2 + 1) {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();

		prepareGrid();

		//copy UA bitmaps and LSB to arrays for faster access
		int nsets = static_cast <int>(usets.size());
		nAllUA = 0;
		//allUA = (bm128 *)_mm_malloc(nsets * sizeof(bm128), 16);
		allUA = bm128::allocate(nsets);
		uaLSB = (int*)malloc(nsets * sizeof(int));
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			uaLSB[nAllUA] = s->positions[0];
			allUA[nAllUA++] = s->bitmap128;
		}

		//find the lower limit of clues to complete the grid to unique solution
		nClues = 9 + 18; //start with (9 + 1) + 18 = 28 clues
		nPuzzles = 0;
		while(nPuzzles == 0 && nClues < maxClues) {
			nClues++;
			nChecked = 0;
			//progressUpdate = 0;
			state[nClues].cluePosition = 81;
			//set the 18 fixed clues
			for(int i = 0; i < theClique.fixedClues.nbits; i++) {
				state[nClues - 1 - i].cluePosition = 80 - i;
			}
			//printf("\nStarting enumeration of valid puzzles of size 18+%d for digits (%d,%d) fixed.\n", nClues - 18, d1 + 1, d2 + 1);
			//init local timer
			start = clock();
			//start from leftmost (most significant) clue
			clueNumber = nClues - theClique.fixedClues.nbits;
			//enumerate all possibilities up to the fixed clues which are placed at left side
			//fprintf(puzFile, "Digits (%d,%d), Clues %d\n", d1 + 1, d2 + 1, nClues - 18);
			processChunks();

			dMinimalClues[d1][d2] = nClues - 18;
			dPuzzles[d1][d2] = nPuzzles;
			//printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);
		}
		printf("Digits (%d,%d), 18+%2d=%5u, minimals=%d\n", d1 + 1, d2 + 1, nClues - 18, nPuzzles, (int)minimizedPuzzles.size());
		//maxClues = nClues; //for next digit pairs stop earlier
		free(uaLSB);
		//_mm_free(allUA);
		bm128::deallocate(allUA);
	}
	//print results
	//printf("\n==========\n");
	for(int d1 = 0; d1 < 8; d1++) for(int d2 = d1 + 1; d2 < 9; d2++) {
		//printf("Digit pair (%d,%d), MinClues %d, NumPuzzles %d\n", d1 + 1, d2 + 1, dMinimalClues[d1][d2], dPuzzles[d1][d2]);
		fprintf(puzFile, "Digit pair (%d,%d), MinClues %d, NumPseudoPuzzles %d\n", d1 + 1, d2 + 1, dMinimalClues[d1][d2], dPuzzles[d1][d2]);
	}
	int nMinimals = (int)minimizedPuzzles.size();
	printf("%d %d-s found.\n", (int)minimizedPuzzles.size(), huntClues);
	fprintf(puzFile, "%d %d-s found.\n", (int)minimizedPuzzles.size(), huntClues);
	for(puzTextSet::const_iterator p = minimizedPuzzles.begin(); p != minimizedPuzzles.end(); p++) {
		printf("%81.81s\n", p->chars);
		fprintf(puzFile, "%81.81s\n", p->chars);
	}

	//fprintf(puzFile, "\nProcessing finished.\n");
	fclose(puzFile);
}

void clueIterator::iterate3digits(const int numClues) {
	huntClues = numClues;
	int dMinimalClues[9][9][9];
	int dPuzzles[9][9][9];
	char buf[2000];
	sprintf(buf, "%s%s", g.fname, g.file3dSuffix);
	puzFile = fopen(buf, "at");
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	minimizedPuzzles.clear();
	for(int d1 = 0; d1 < 7; d1++) for(int d2 = d1 + 1; d2 < 8; d2++) for(int d3 = d2 + 1; d3 < 9; d3++) {
		printf("\n========= Processing digit triplet (%d,%d,%d) ==========\n", d1 + 1, d2 + 1, d3 + 1);
		theClique.clear();
		theClique.fixedClues.clear();
		for(int i = 0; i < 81; i++) {
			if(g.digits[i] == d1 + 1 || g.digits[i] == d2 + 1 || g.digits[i] == d3 + 1) {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();

		prepareGrid();

		//copy UA bitmaps and LSB to arrays for faster access
		int nsets = static_cast <int>(usets.size());
		nAllUA = 0;
		allUA = bm128::allocate(nsets);
		//allUA = (bm128 *)_mm_malloc(nsets * sizeof(bm128), 16);
		uaLSB = (int*)malloc(nsets * sizeof(int));
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			uaLSB[nAllUA] = s->positions[0];
			allUA[nAllUA++] = s->bitmap128;
		}

		//find the lower limit of clues to complete the grid to unique solution
		//nClues = 9 + 18; //start with (9 + 1) + 18 = 28 clues
		nClues = 7 + 27; //start with (7 + 1) + 27 = 35 clues
		nPuzzles = 0;
		while(nPuzzles == 0) {
			nClues++;
			nChecked = 0;
			//progressUpdate = 0;
			state[nClues].cluePosition = 81;
			//set the 18 fixed clues
			for(int i = 0; i < theClique.fixedClues.nbits; i++) {
				state[nClues - 1 - i].cluePosition = 80 - i;
			}
			printf("\nStarting enumeration of valid puzzles of size 27+%d for digits (%d,%d,%d) fixed.\n", nClues - 27, d1 + 1, d2 + 1, d3 + 1);
			//init local timer
			start = clock();
			//start from leftmost (most significant) clue
			clueNumber = nClues - theClique.fixedClues.nbits;
			//enumerate all possibilities up to the fixed clues which are placed at left side
			fprintf(puzFile, "Digits (%d,%d,%d), Clues %d\n", d1 + 1, d2 + 1, d3 + 1, nClues - 27);
			processChunks();
			dMinimalClues[d1][d2][d3] = nClues - 27;
			dPuzzles[d1][d2][d3] = nPuzzles;
			printf("\nSearched for 27+%d, generated %llu puzzles, found %u valid.\n", nClues - 27, nChecked, nPuzzles);
			printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);
		}
		free(uaLSB);
		//_mm_free(allUA);
		bm128::deallocate(allUA);
	}
	//print results
	printf("\n==========\n");
	fprintf(puzFile, "\n==========\n");
	for(int d1 = 0; d1 < 7; d1++) for(int d2 = d1 + 1; d2 < 8; d2++) for(int d3 = d2 + 1; d3 < 9; d3++) {
		printf("Digit triplet (%d,%d,%d), MinClues %d, NumPuzzles %d\n", d1 + 1, d2 + 1, d3 + 1, dMinimalClues[d1][d2][d3], dPuzzles[d1][d2][d3]);
		fprintf(puzFile, "Digit triplet (%d,%d,%d), MinClues %d, NumPuzzles %d\n", d1 + 1, d2 + 1, d3 + 1, dMinimalClues[d1][d2][d3], dPuzzles[d1][d2][d3]);
	}

	if(minimizedPuzzles.size()) {
		printf("%d 17-s found.\n", (int)minimizedPuzzles.size());
		fprintf(puzFile, "%d 17-s found.\n", (int)minimizedPuzzles.size());
		for(puzTextSet::const_iterator p = minimizedPuzzles.begin(); p != minimizedPuzzles.end(); p++) {
			printf("%81.81s\n", p->chars);
			fprintf(puzFile, "%81.81s\n", p->chars);
		}
	}

	fclose(puzFile);
}

//void clueIterator::iterate4digits(const int numClues) {
//	//const int nDigits = 3;
//	//const int nTemplates = 84;
//	//const int (*choices)[nTemplates][nDigits] = &choice3of9;
//	const int nDigits = 4;
//	const int nTemplates = 126;
//	const int (*choices)[nTemplates][nDigits] = &choice4of9;
//	huntClues = numClues;
//	opt.scanOpt->scanpuzzleset = true; //force storing the puzzles
//	minimizedPuzzles.clear();
//	puzzleSet templateList;
//	int minCluesToComplete = 100;
//	for(int t = 0; t < nTemplates; t++) { //template index
//		ch81 puz;
//		puz.clear();
//		//set the template's cells as givens
//		for(int i = 0; i < 81; i++) {
//			for(int j = 0; j < nDigits; j++) {
//				if(g.digits[i] - 1 == (*choices)[t][j]) {
//					puz.chars[i] = g.digits[i];
//					break;
//				}
//			}
//		}
//		//find minimal-clue completions of this template
//		clueIterator ci(g);
//		ci.iterateFixedCells(puz.chars, 0, minCluesToComplete + 9 * nDigits);
//		ch81 pp;
//		puz.toString(pp.chars); //debug
//		printf("%81.81s\t%d\t%d\n", pp.chars, ci.nClues - 9 * nDigits, ci.minimizedPuzzles.size()); //debug
//		fflush(NULL);
//		//filter out the expensive templates
//		int minCluesToCompleteTemplate = ci.nClues - 9 * nDigits;
//		if(minCluesToComplete < minCluesToCompleteTemplate) {
//			continue;
//		}
//		else if(minCluesToComplete > minCluesToCompleteTemplate) {
//			minCluesToComplete = minCluesToCompleteTemplate;
//			templateList.clear();
//		}
//		for(puzTextSet::const_iterator p = ci.minimizedPuzzles.begin(); p != ci.minimizedPuzzles.end(); p++) {
//			for(int i = 0; i < 81; i++) {
//				if(puz.chars[i])
//					pp.chars[i] = 0;
//				else
//					pp.chars[i] = p->chars[i] - '0';
//			}
//			templateList.insert(pp);
//		}
//	}
//	//find minimal-clue completions by adding givens (not only from template)
//	int n = 0; //debug
//	for(puzzleSet::const_iterator pp = templateList.begin(); pp != templateList.end(); pp++) {
//		clueIterator cci(g);
//		cci.iterateFixedCells(pp->chars, numClues, numClues);
//		minimizedPuzzles.insert(cci.minimizedPuzzles.begin(), cci.minimizedPuzzles.end());
//		//print results (debug)
//		if(cci.minimizedPuzzles.size()) {
//			//printf("For %d clues, %d puzzles has been found.\n", nClues - nFixed, minimizedPuzzles.size());
//			for(puzTextSet::const_iterator p2 = cci.minimizedPuzzles.begin(); p2 != cci.minimizedPuzzles.end(); p2++) {
//				printf("%81.81s\t%d\n", p2->chars, cci.nClues);
//			}
//		}
//	}
//	//print results
//	if(minimizedPuzzles.size()) {
//		printf("For %d clues, %d puzzles has been found:\n", numClues, minimizedPuzzles.size());
//		for(puzTextSet::const_iterator p2 = minimizedPuzzles.begin(); p2 != minimizedPuzzles.end(); p2++) {
//			printf("%81.81s\n", p2->chars);
//		}
//	}
//}

void clueIterator::iterate4digits(const int numClues) {
	//split grid into 4+5 digits in all 126 possible ways
	const int nDigits = 4;
	const int nTemplates = 126;
	const int (*choices)[nTemplates][nDigits] = &choice4of9;
	//huntClues = numClues;
	opt.scanOpt->scanpuzzleset = true; //do not store the puzzles
	opt.scanOpt->progressSeconds = 100000;
	//opt.uaOpt->maxuasize = 20;
	//opt.uaOpt->nAttempts = 135;
	//opt.uaOpt->nCells = 35;
	//int oldMaxUaSize = opt.uaOpt->maxuasize;
	int count = 0;
	ch81 pp;
	pp.toString(g.digits, pp.chars); //debug
	printf("%81.81s (the grid)\n", pp.chars); //debug
	for(int t = 0; t < nTemplates; t++) { //template index
		ch81 puz;
		puz.clear();
		//set the template's cells as givens
		for(int i = 0; i < 81; i++) {
			for(int j = 0; j < nDigits; j++) {
				if(g.digits[i] - 1 == (*choices)[t][j]) {
					puz.chars[i] = g.digits[i];
					break;
				}
			}
		}
		//find minimal-clue completions of this template
		//int minClues1, minClues2, totalClues;
		clueIterator ci4(g);
		//g.usetsBySize.clear();
		//opt.uaOpt->nAttempts = 135;
		//g.findUArandom(puz.chars);
		ci4.iterateFixedCells(puz.chars);
		count += (int)ci4.minimizedPuzzles.size(); //debug
		////iterate trough valid completions
		//for(puzTextSet::const_iterator p4 = ci4.minimizedPuzzles.begin(); p4 != ci4.minimizedPuzzles.end(); p4++) {
		//	//compose complementary puzzle
		//	for(int i = 0; i < 81; i++) {
		//		if(p4->chars[i] != '0' && puz.chars[i] == 0)
		//			completion.chars[i] = p4->chars[i] - '0';
		//		else
		//			completion.chars[i] = 0;
		//	}
		//	completion.toString(pp.chars); //debug
		//	printf("%81.81s (complementary)\n", pp.chars); //debug
		//	clueIterator ci5(g);
		//	//g.usetsBySize.clear();
		//	//opt.uaOpt->nAttempts = 1500;
		//	//g.findUArandom(completion.chars);
		//	ci5.iterateFixedCells(completion.chars, numClues, numClues);
		//	for(puzTextSet::const_iterator p5 = ci5.minimizedPuzzles.begin(); p5 != ci5.minimizedPuzzles.end(); p5++) {
		//		printf("%81.81s\n", p5->chars);
		//	}
		//}
		//fflush(NULL);
	}
	printf("%d\n", count);
}

void clueIterator::iterateFixedCells(const char *fixed, const int minClues, const int maxClues) {
	huntClues = 0;
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	//opt.scanOpt->storepseudos = true;
	minimizedPuzzles.clear();
	theClique.clear();
	theClique.fixedClues.clear();
	int nFixed = 0;
	for(int i = 0; i < 81; i++) {
		if(fixed[i]) {
			theClique.fixedClues.setBit(i);
			nFixed++;
		}
	}
	theClique.fixedClues.positionsByBitmap();

	prepareGrid();

	//copy UA bitmaps and LSB to arrays for faster access
	int nsets = static_cast <int>(usets.size());
	nAllUA = 0;
	allUA = bm128::allocate(nsets);
	uaLSB = (int*)malloc(nsets * sizeof(int));
	for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
		uaLSB[nAllUA] = s->positions[0];
		allUA[nAllUA++] = s->bitmap128;
	}

	//find the lower limit of clues to complete the grid to unique solution
	nClues = nFixed + theClique.size - 1; //start with minimal number of clues guaranted to solve at least the clique
	if(nClues < minClues - 1)
		nClues = minClues - 1;
	nPuzzles = 0;
	while(nPuzzles == 0 && nClues < maxClues) {
		nClues++;
		nChecked = 0;
		state[nClues].cluePosition = 81;
		//set the clues
		for(int i = 0; i < theClique.fixedClues.nbits; i++) {
			state[nClues - 1 - i].cluePosition = 80 - i;
		}
		//start from leftmost (most significant) clue
		clueNumber = nClues - theClique.fixedClues.nbits;
		processChunks();
	}
	free(uaLSB);
	bm128::deallocate(allUA);
}

void clueIterator::iterateBand(const int numClues) {
	nPuzzles = 0;
	if(numClues < 2)
		return;
	//huntClues = numClues;
	nClues = numClues + 54;
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	theClique.fixedClues.clear();
	for(int i = 27; i < 81; i++) {
		theClique.fixedClues.setBit(i);
	}
	theClique.fixedClues.positionsByBitmap();

	prepareGrid();

	//find the lower limit of clues to complete the grid to unique solution
	if(nClues < theClique.size + 54)
		return;
	//copy UA bitmaps and LSB to arrays for faster access
	int nsets = static_cast <int>(usets.size());
	nAllUA = 0;
	allUA = bm128::allocate(nsets);
	uaLSB = (int*)malloc(nsets * sizeof(int));
	for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
		uaLSB[nAllUA] = s->positions[0];
		allUA[nAllUA++] = s->bitmap128;
	}

	nChecked = 0;
	state[nClues].cluePosition = 81;
	//set the 54 fixed clues
	for(int i = 0; i < theClique.fixedClues.nbits; i++) {
		state[nClues - 1 - i].cluePosition = 80 - i;
	}
	//start from leftmost (most significant) clue
	clueNumber = nClues - theClique.fixedClues.nbits;
	//enumerate all possibilities up to the fixed clues which are placed at left side
	processChunks();

	free(uaLSB);
	bm128::deallocate(allUA);
}

void clueIterator::iterateFixed(const int numClues) {
	//theClique must be initialized with fixedClues and fixedNonClues
	huntClues = numClues;
	numUaSizeLimit = 3000; //1000
	numUaTotalLimit = 4000; //1000

	if(prepareGrid()) //no solution
		return;

	//copy UA bitmaps and LSB to arrays for faster access
	int nsets = static_cast <int>(usets.size());
	nAllUA = 0;
	allUA = bm128::allocate(nsets);
	uaLSB = (int*)malloc(nsets * sizeof(int));
	for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
		uaLSB[nAllUA] = s->positions[0];
		allUA[nAllUA++] = s->bitmap128;
	}

	nClues = numClues;
	nPuzzles = 0;
	nChecked = 0;
	state[nClues].cluePosition = 81;
	//set the fixed clues
	for(int i = 0; i < theClique.fixedClues.nbits; i++) {
		state[nClues - 1 - i].cluePosition = 80 - theClique.fixedNonClues.nbits - i;
	}
	//start from leftmost (most significant) clue
	clueNumber = nClues - theClique.fixedClues.nbits;
	//enumerate all possibilities up to the fixed clues which are placed at left side
	processChunks();

	free(uaLSB);
	bm128::deallocate(allUA);
}

void clueIterator::iterateBands(const int numClues) {
	huntClues = numClues;
	int bandMinimalClues[6];
	int bandPuzzles[6];
	char buf[2000];
	sprintf(buf, "%s%s", g.fname, g.fileBandSuffix);
	puzFile = fopen(buf, "at");
	numUaSizeLimit = 1000;
	numUaTotalLimit = 1000;
	//skipProgress = true;
	for(int band = 0; band < 6; band++) {
		printf("\n========= Processing band %d ==========\n", band + 1);
		theClique.clear();
		theClique.fixedClues.clear();
		for(int i = 0; i < 81; i++) {
			if((band < 3 ? bandByCellIndex[i] : stackByCellIndex[i]) == (band % 3)) {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();

		prepareGrid();

		//copy UA bitmaps and LSB to arrays for faster access
		int nsets = static_cast <int>(usets.size());
		nAllUA = 0;
		allUA = bm128::allocate(nsets);
		uaLSB = (int*)malloc(nsets * sizeof(int));
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			uaLSB[nAllUA] = s->positions[0];
			allUA[nAllUA++] = s->bitmap128;
		}

		//find the lower limit of clues to complete the grid to unique solution
		nClues = 3 + 27; //start with (3 + 1) + 27 = 31 clues
		nPuzzles = 0;
		while(nPuzzles == 0) {
			nClues++;
			nChecked = 0;
			//progressUpdate = 0;
			state[nClues].cluePosition = 81;
			//set the 27 fixed clues
			for(int i = 0; i < theClique.fixedClues.nbits; i++) {
				state[nClues - 1 - i].cluePosition = 80 - i;
			}
			printf("\nStarting enumeration of valid puzzles of size 27+%d for band %d fixed.\n", nClues - 27, band + 1);
			//init local timer
			start = clock();
			//start from leftmost (most significant) clue
			clueNumber = nClues - theClique.fixedClues.nbits;
			//enumerate all possibilities up to the fixed clues which are placed at left side
			fprintf(puzFile, "Band %d, Clues %d\n", band + 1, nClues - 27);
			processChunks();
			bandMinimalClues[band] = nClues - 27;
			bandPuzzles[band] = nPuzzles;
			printf("\nSearched for 27+%d, generated %llu puzzles, found %u valid.\n", nClues - 27, nChecked, nPuzzles);
			printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);
		}

		free(uaLSB);
		bm128::deallocate(allUA);
	}
	//print results
	printf("\n==========\n");
	fprintf(puzFile, "\n==========\n");
	for(int band = 0; band < 6; band++) {
		printf("Band %d, MinClues %d, NumPuzzles %d\n", band + 1, bandMinimalClues[band], bandPuzzles[band]);
		fprintf(puzFile, "Band %d, MinClues %d, NumPuzzles %d\n", band + 1, bandMinimalClues[band], bandPuzzles[band]);
	}
	fclose(puzFile);
}

int clueIterator::prepareGrid() {
	//find clique, filter and reorder the UA, remap the grid
	//still indepenent of nClues

	////remove UA sets leading to isomorphs
	////TODO: find appropriate place for this piece of code
	//for(usetListBySize::iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end();) {
	//	char buf[88];
	//	usetListBySize::iterator nextUA = ua;
	//	nextUA++;
	//	if(ua->nbits == 18) {
	//		g.ua2puzzle(*ua, buf);
	//		if(!hasEDSolution(g.gridBM, buf))
	//			g.usetsBySize.erase(ua);
	//	}
	//	ua = nextUA;
	//}
	//opt.uaOpt->mcnNoAutoUA = true;

	usetsBySize.clear();
	//for(usetListBySize::const_iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end(); ua++) {
	//	if(ua->isDisjoint(theClique.fixedClues)) {
	//		usetsBySize.insert(*ua);
	//	}
	//}
	uset forcedClues;
	forcedClues.clear();
	for(usetListBySize::const_iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end(); ua++) {
		if(ua->isDisjoint(theClique.fixedClues)) { // ignore UA hit by a fixed clue
			if(ua->isDisjoint(theClique.fixedNonClues)) {
				usetsBySize.insert(*ua);
			}
			else { // reduce the size of the UA
				uset u = *ua;
				u.clearBits(theClique.fixedNonClues);
				u.positionsByBitmap();
				if(u.nbits == 0) { // UA within forced non-clues
					printf("UA within forced non-clues\n"); //debug
					return 2;
				}
				if(u.nbits <= 1) { // forced clue
					//printf("Forced clue %d, UA of size %d\n", u.positions[0], u.nbits);
					forcedClues |= u;
				}
				usetsBySize.insert(u);

			}
		}
	}
	forcedClues.positionsByBitmap();
	if(forcedClues.nbits) {
		printf("%d forced clues:", forcedClues.nbits); //debug
		for(int i = 0; i < forcedClues.nbits; i++) { //debug
			printf(" %d", forcedClues.positions[i]); //debug
		}
		printf("\n"); //debug
		if(forcedClues.nbits + theClique.fixedClues.nbits > huntClues) {
			printf("too many forced non-clues\n"); //debug
			return 1;
		}
		//remove the UA hit by forced clues
		for(usetListBySize::iterator ua = usetsBySize.begin(); ua != usetsBySize.end();) {
			if(ua->isDisjoint(forcedClues)) {
				ua++;
			}
			else {
				//ua = usetsBySize.erase(ua);
                usetListBySize::iterator ss = ua;
                ua++;
				usetsBySize.erase(ss);
            }
		}
		//add the forced clues to fixed clues
		theClique.fixedClues |= forcedClues;
		theClique.fixedClues.positionsByBitmap();
		//ensure consistency in fixed/non-fixed clues
		theClique.fixedNonClues.clearBits(theClique.fixedClues);
		theClique.fixedNonClues.positionsByBitmap();
		printf("%d fixed clues\n", theClique.fixedClues.nbits); //debug
	}

	//collect some information useful for later decisions
	usetsBySize.setDistributions();
	//find a maximal clique having minimum combinations
	g.findMaximalCliques(theClique, usetsBySize);
	//if(opt.verbose) {
	//	theClique.toString(buf);
	//	printf("Chosen Maximal Clique:\n%s", buf);
	//}
	theClique.sortMembers(usetsBySize);
	//switch to clique's coordinate system; transform usetsBySize to usets
	remap();
	if(opt.verbose) {
		printf("\nIterator to grid map:\n");
		for(int i = 0; i < 81; i++) {
			//printf("%2.2d ", theMapper.i2g[i]);
			printf("%2.2d ", 10 * (1 + theMapper.i2g[i] / 9) + 1 + theMapper.i2g[i] % 9);
		}
		printf("\nGrid to iterator map:\n");
		for(int i = 0; i < 81; i++) {
			//printf("%2.2d ", theMapper.g2i[i]);
			printf("%2.2d ", 10 * (1 + theMapper.g2i[i] / 9) + 1 + theMapper.g2i[i] % 9);
		}
		printf("\n");

		printf("\nmaxPositionLimitsByUA\n");
		for(int j = 0; j < nClues; j++)
			printf("%2.2d ", state[j].maxPositionLimitsByUA);
		printf("\n");
	}

	//find optimal UA collection, erase the rest UA
	removeLargeUA();


	//findTresholds(); //experimental

	return 0;
}

void clueIterator::findTresholds() {
	//on fixed clue mapping find cliques of semi-disjoint UA so that:
	// - treshold position is N
	// - N(M) is between M and state[M].maxPositionLimitsByUA]
	// - number of cliques is <= 768, possibly truncate if the next possibility is ~0
	// - all clique members have clue(s) at right of N
	// - all members are of size <= maxsize
	// - clique size is M so that it is checked after M'th clue is placed
	// - M > 3, last 2 clues are optimized in other way
	// - M < 14, large cliques are difficult to find

	//098765432109876543210987654321098765432109876543210987654321098765432109876543210
	//8         7         6         5         4         3         2         1
	//.........................<       UA4       ><     UA3    ><  UA2  >< UA1 >< UA0 >
	//                        ^state[4].maxPositionLimitsByUA]              minPos^
	//..............................N..................................................
	//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

	//we are placing the clue M=4
	//each clue moves from minPos, limited by the larger of
	// a) clueNumber, so that there is space for the rest clues at right (for clue 4 the rest clues are 3,2,1,0 and we start from position 4)
	// b) the rightmost cell of all unhit UA (actually the first since they are ordered in this way)
	
	//The maxPos is limited by the smaller of
	// a) the position of the previous clue
	// b) state[M].maxPositionLimitsByUA] where we know the right M UA from the mapping clique must be hit by at least one clue per UA

	//We want to narrow the search space by obtaining one more treshold below maxPos

	//partition clues at "a" and "b"
	//semi-disjoint UA are these having 0 or more disjoint clues in "b" partition
	//if there exists clique of size M+2 and it is not hit by any of the previous clues, then we know this
	//branch is doomed since we need at leat one clue per UA to hit this clique, but there are only M+1 clues to play with

	//we have 3 parameters to deal with: the clue number M; its threshold position N; and the UA sizes to use in cliques composition.
	
	const int maxUAsize = 9; //ignore larger UA
	const int maxCliquesInTreshold = 100000; //ignore cliques with large amount of clues

	lightweightUsetList complexUA[16];
	//lightweightUsetList largeUA;

	for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
		if(s->nbits > maxUAsize) {
			//largeUA.insert(s->bitmap128);
		}
		else {
			complexUA[0].insert(s->bitmap128);
		}
	}
	//int clNum = theClique.size - 2; //start from zero-based clue = MCN - 2, search for MCN - 1
	int clNum = 10;
	for(int t = clNum; t >= 3; t--) {
		//int critPos = state[t - 1].maxPositionLimitsByUA;
		int critPos = 31;
		const bm128 &mask = maskLSB[critPos];
		for(int n = 1; n < 16; n++) { //cleanup the data collected from previous target clue
			complexUA[n].clear();
		}
		int n = 0;
		do {
			n++;
			for(lightweightUsetList::const_iterator s = complexUA[0].begin(); s != complexUA[0].end(); s++) {
				bm128 u1(*s);
				if(u1.isDisjoint(mask)) {
					continue; //surely will be hit earlier
				}
				u1 &= mask;
				for(lightweightUsetList::const_iterator ss = complexUA[n - 1].begin(); ss != complexUA[n - 1].end(); ss++) {
					bm128 u2 = *ss;
					if(u2.isDisjoint(mask)) {
						continue; //surely will be hit earlier
					}
					if(u2.isDisjoint(u1)) {
						u2 |= *s;
						complexUA[n].insert(u2);
					}
next:				;
				}
			}
			printf("Complex UA[%d,%d,%d]\t%d\n", t, critPos, n, (int)complexUA[n].size());
			if(n > 1) complexUA[n - 1].clear();
		} while(!complexUA[n].empty() && n < t + 1);
		if(complexUA[n].empty()) {
			//stopped before the target clue so do nothing
			continue;
		}
		//erase cliques having known UA in "a" partition, they will be hit earlier
		for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
			if(s->isDisjoint(mask)) { //the UA lies entirely in "a" partition
				for(lightweightUsetList::iterator ss = complexUA[n].begin(); ss != complexUA[n].end();) {
					if(s->isSubsetOf(*ss)) { //clique covers the UA
						//we know the UA will be hit earlier, therefore the clique will be hit too, therefore it is redundant
						lightweightUsetList::iterator sss = ss;
						ss++;
						complexUA[n].erase(sss);
					}
					else
						ss++;
				}
			}
		}
		//printf("w/o covered UA\t%d\n", (int)complexUA[n].size());

		//remove cells in partition "b" and sort by cell numbers
		sizedUsetList thList;
		for(lightweightUsetList::const_iterator s = complexUA[n].begin(); s != complexUA[n].end(); s++) {
			bm128 u1(*s);
			u1.clearBits(mask);
			thList.insert(u1); //during the conversion the size is calculated
		}
		complexUA[n].clear();
		printf("%d UA of sizes between %d and %d\n", (int)thList.size(), thList.begin()->getSize(), thList.rbegin()->getSize());
		UATable &th = tresholds[t];
		int finalSize = (int)thList.size();
		if(finalSize > maxCliquesInTreshold) {
			//truncate
			finalSize = maxCliquesInTreshold;
		}
		th.setSize(finalSize);
		int i = 0;
		for(sizedUsetList::const_iterator s = thList.begin(); i < finalSize /* && s != thList.end()*/; s++, i++) {
			sizedUset u = *s;
			u.setSize(0);
			th.rows[i] = u.bitmap128;
		}

		////how the top cliques are joined to each other?
		//for(int i = 0; i < th.size - 1; i++) {
		//	for(int j = i + 1; j < th.size; j++) {
		//		bm128 common(th.rows[i]);
		//		common.clearBits(th.rows[j]);
		//		int x = common.popcount_128();
		//		if(x <= 1) {
		//			printf("%d\t%d\t%d\t%d\n", n, i, j, x);
		//		}
		//	}
		//}
	}
}

void clueIterator::iterateWhole(const int numClues) {
	nClues = numClues;
	char buf[2000];
	sprintf(buf, "%s.%d%s", g.fname, nClues, g.filePuzSuffix);
	puzFile = fopen(buf, "at");
	if(opt.scanOpt->cluemask) {
		//prepare fixed clues and fixed non-clues
		for(int i = 0; i < 81 && opt.scanOpt->cluemask[i]; i++) {
			if(opt.scanOpt->cluemask[i] == '0') {
				theClique.fixedNonClues.setBit(i);
			}
			else if(opt.scanOpt->cluemask[i] == '1') {
				theClique.fixedClues.setBit(i);
			}
		}
		theClique.fixedClues.positionsByBitmap();
		theClique.fixedNonClues.positionsByBitmap();
		iterateFixed(numClues);
		printf("\t%d puzzles\n", nPuzzles);
		if(minimizedPuzzles.size()) {
			printf("\n");
			for(puzTextSet::const_iterator p = minimizedPuzzles.begin(); p != minimizedPuzzles.end(); p++) {
				printf("%81.81s\n", p->chars);
			}
		}
		printf("\n");
		fflush(NULL);
	}
	else {
		//fprintf(puzFile, "\nStart processing at %s\n", opt.getStartTime());
		iterate();
	}
	//fprintf(puzFile, "\nProcessing finished.\n");
	//fclose(puzFile);
}

void clueIterator::iterate() {
	prepareGrid();
	//if(opt.verbose) {
	//	printf("MCN=%d.\n", theClique.size);
	//}

	//copy UA bitmaps and LSB to arrays for faster access
	int nsets = static_cast <int>(usets.size());
	nAllUA = 0;
	allUA = bm128::allocate(nsets);
	uaLSB = (int*)malloc(nsets * sizeof(int));
	for(usetList::const_iterator s = usets.begin(); s != usets.end(); s++) {
		//if(state[0].maxPositionLimitsByUA > s->positions[s->nbits - 1] + 1) {
		//	printf("Unexpected correction of state[0].maxPositionLimitsByUA from %d to %d\n", state[0].maxPositionLimitsByUA, s->positions[s->nbits - 1] + 1);
		//	state[0].maxPositionLimitsByUA = s->positions[s->nbits - 1] + 1;
		//}
		uaLSB[nAllUA] = s->positions[0];
		allUA[nAllUA++] = s->bitmap128;
	}

	state[nClues].cluePosition = 81;
	//set the fixed clues if any
	for(int i = 0; i < theClique.fixedClues.nbits; i++) {
		state[nClues - 1 - i].cluePosition = 80 - i;
	}
	printf("\nStarting enumeration of all valid puzzles of size up to %d.\n", nClues);
	//init local timer
	start = clock();
	//start from leftmost (most significant) clue
	clueNumber = nClues - theClique.fixedClues.nbits;
	//enumerate all possibilities up to the fixed clues which are placed at left side
	processChunks();
	printf("\nSearched for %d, generated %llu puzzles, found %u valid.\n", nClues, nChecked, nPuzzles);
	printf("Enumeration done in %2.3f seconds.\n", (double)(clock() - start) / CLOCKS_PER_SEC);

	free(uaLSB);
	bm128::deallocate(allUA);
}

NOINLINE void clueIterator::getChunks(const bm128 *sets, const int *lsb, const int nsets) {
	bm128 *subsets;
	int *subsetLsbs;
	int maxPos = state[clueNumber].cluePosition;
	int lastClue = nClues - theClique.fixedClues.nbits - chunk::maxNumClues;
	int maxChunkClues = chunk::maxNumClues;
	if(lastClue < 1) { //decrease the chunk fixed clues
		maxChunkClues -= (1 - lastClue);
		if(maxChunkClues == 0) {
			//add the only chunk
			chunk theChunk;
			theChunk.numClues = 0;
			theChunk.portion = 1.0;
			theChunks.push_back(theChunk);
			return;
		}
		lastClue = 1;
	}
	clueNumber--;
	int nsubsets;
	int minPos = clueNumber;
	//adjust maxPos which is independent of UA list
	if(maxPos > state[clueNumber].maxPositionLimitsByUA) {
		//there is a disjoint set at right which must be hit by this clue
		maxPos = state[clueNumber].maxPositionLimitsByUA;
	}
	if(nsets) {
		//Adjust the minimal position according to the first unavoidable 
		if(minPos < *lsb) {
			//this unavoidable couldn't be hit by the clues right of minPos
			minPos = *lsb;
		}
	}
	//subsets = (bm128 *)_mm_malloc(nsets * (sizeof(bm128) + sizeof(int)), 16);
	subsets = bm128::allocate(nsets + nsets / 2 + 1); //assume sizeof(int) <= 64
	subsetLsbs = (int*)&subsets[nsets];
	//iterate trough all possible positions
	for(int pos = minPos; pos < maxPos; pos++) {
		state[clueNumber].cluePosition = pos;
		if(clueNumber == lastClue) {
			//add a chunk
			chunk theChunk;
			theChunk.numClues = maxChunkClues;
			for(int i = 0; i < theChunk.numClues; i++) {
				theChunk.cluePositions[i] = state[clueNumber - i + theChunk.numClues - 1].cluePosition;
			}
			theChunk.portion = progress();
			theChunks.push_back(theChunk);
		}
		else {
			//filter the UA for next clue
			nsubsets = 0;
			for(int s = 0; s < nsets; s++) {
				if(!sets[s].isBitSet(pos)) {
					subsetLsbs[nsubsets] = lsb[s];
					subsets[nsubsets++] = sets[s];
				}
			}
			getChunks(subsets, subsetLsbs, nsubsets);
		}
	}
	//_mm_free(subsets);
	bm128::deallocate(subsets);
	clueNumber++;
}

NOINLINE void clueIterator::processChunks() {
	//clock_t minTime = 30 * CLOCKS_PER_SEC;
	clock_t minTime = opt.scanOpt->progressSeconds * CLOCKS_PER_SEC;
	theChunks.clear();
	nPuzzles = 0;
	nChecked = 0;
	if(opt.verbose) {
		printf("\nGenerating chunks...");
	}
	getChunks(allUA, uaLSB, nAllUA);
	int nChunks = (int)theChunks.size();
	if(opt.verbose) {
		printf(" %d chunks generated.\n", nChunks);
	}
	if(nChunks == 0)
		return;
	theChunks.rescalePortions();
	theChunks.randomize();
	double progress = 0.0;
	clock_t nextTime = clock() + minTime;
	//run in parallel
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif //_OPENMP
	for(int i = 0; i < nChunks; i++) {
		if(nPuzzles && opt.scanOpt->stopatfirst)
			continue; //perform empty loop
		const chunk &theChunk = theChunks[i];
		chunkProcessor cp = chunkProcessor(this, theChunk);
		cp.process();
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
		{
			if(cp.nPuzzles) {
				if(opt.scanOpt->bandcompletions || opt.scanOpt->scanpuzzleset) { //collect completions
					for(int j = 0; j < (int)cp.nPuzzles; j++) {
						minimizedPuzzles.insert(cp.validPuzzles[j]);
					}
				}
				else if(theClique.fixedClues.nbits == 0 || opt.scanOpt->storepseudos) {
					for(int j = 0; j < (int)cp.nPuzzles; j++) {
						fprintf(puzFile, "%81.81s\n", cp.validPuzzles[j].chars);
					}
					fflush(puzFile);
				}
				if(!cp.minimizedPuzzles.empty()) {
					minimizedPuzzles.insert(cp.minimizedPuzzles.begin(), cp.minimizedPuzzles.end());
				}
			}
			nPuzzles += cp.nPuzzles;
			nChecked += cp.nChecked;
			progress += theChunk.portion;
			clock_t now = clock();
			if(now > nextTime) {
				showProgress(progress);
				nextTime = now + minTime;
			}
		}
	}
	theChunks.clear();
}

void clueIterator::initProgress() {
	//(n over k) = ((n-1) over (k-1)) + ((n-1) over k)
	//(n over 0) = 1
	int k, n;
	for(n = 0; n <= 81; n++) {
		NoK[n][0] = 1.0;
		NoK[n][n] = 1.0;
	}
	for(k = 1; k <= 81; k++) {
		for(n = k + 1; n <= 81; n++) {
			NoK[n][k] = NoK[n - 1][k - 1] + NoK[n - 1][k];
		}
	}
}

NOINLINE double clueIterator::progress() {
	int n;
	double p = 0.0;
	for(n = 0; n < clueNumber; n++) {
		state[n].cluePosition = n;
	}
	for(n = 0; n < nClues - theClique.fixedClues.nbits; n++) {
		p += NoK[state[n].cluePosition][n];
	}
	p /= NoK[81 - theClique.fixedClues.nbits][nClues - 1 - theClique.fixedClues.nbits];
	return p;
}

NOINLINE void clueIterator::showProgress() {
	showProgress(progress());
}

NOINLINE void clueIterator::showProgress(const double pr) {
	//progressUpdate = 0;
	//progressCounter++;
	double rounded_pr = pr > 0.999999 ? 0.999999 : pr;
	double elTime = (double)(clock() - start) / CLOCKS_PER_SEC;
	printf("%07.4f%% at %.3fh,Checked=%llu,Found=%u,ETTF=%.3fh\n",
		100.0 * rounded_pr,
		elTime / 3600,
		nChecked,
		nPuzzles,
		elTime * (1 / rounded_pr - 1) / 3600);
	cout.flush();
	//printf("%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\t%llu\n", skipped[0], skipped[1], skipped[2], skipped[3], skipped[4], skipped[5], skipped[6], skipped[7], skipped[8], skipped[9]);
}

chunkProcessor::chunkProcessor(const clueIterator *clueIterator, const chunk &chunk) : theChunk(chunk) {
	theClueIterator = clueIterator;
	nClues = theClueIterator->nClues;
	memcpy(state, theClueIterator->state, sizeof(state));
	nPuzzles = 0;
	nChecked = 0;
	nFixed = theClueIterator->theClique.fixedClues.nbits;
	stopAtFirst = opt.scanOpt->stopatfirst;
}
void chunkProcessor::process(void) {
	clueNumber = nClues - theClueIterator->theClique.fixedClues.nbits - theChunk.numClues;
	bm128 mask;
	mask.clear();
	for(int i = 0; i < theChunk.numClues; i++) {
		state[clueNumber - i + theChunk.numClues - 1].cluePosition = theChunk.cluePositions[i];
		mask.setBit(theChunk.cluePositions[i]);
	}
	bm128 *subsets;
	int *subsetLsbs;
	int nsubsets = 0;
	subsets = bm128::allocate(theClueIterator->nAllUA + theClueIterator->nAllUA / 2 + 1); //assume sizeof(int) <= 64
	subsetLsbs = (int*)&subsets[theClueIterator->nAllUA];
	for(int s = 0; s < theClueIterator->nAllUA; s++) {
		if(theClueIterator->allUA[s].isDisjoint(mask)) {
			subsetLsbs[nsubsets] = theClueIterator->uaLSB[s];
			subsets[nsubsets++] = theClueIterator->allUA[s];
		}
	}
	if(nsubsets <= (128 * NUM_PACKS)) {
		if(nsubsets)
			switch2bm(subsets, subsetLsbs, nsubsets);
		else
			checkPuzzle();
	}
	else {
		iterateClue(subsets, subsetLsbs, nsubsets);
	}
	bm128::deallocate(subsets);
}
void chunkProcessor::switch2bm(const bm128 *sets, const int *lsb, int nsets) {
	//http://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/
	//http://mischasan.wordpress.com/2011/07/24/what-is-sse-good-for-transposing-a-bit-matrix/
	//http://hackers-delight.org.ua/048.htm
	//https://www.google.bg/#q=bit+matrix+transpose
	
	//0 <= positions_to_index < state[clueNumber].cluePosition

	//printf("Switching to Bitmap at clue number %d, nsets=%d.\n", clueNumber, nsets);
	setsBM = sets;
	//for(int t = 0; t < 100; t++) {
	if(1) {
		//printf("Switching to Bitmap at clue number %d, nsets=%d.\n", clueNumber, nsets);
		//printf(".");
		BitMask768::fromBm128(nsets, sets, hittingMasks);
		//bm128::transpose(768, sets, hittingMasks[0].aBits);
		//for(int i = 0; i < nsets; i++) {
		//	char c[128];
		//	sets[i].toMask81(c);
		//	printf("%81.81s\t%d\n", c, i);
		//}
		//for(int i = 0; i < 81; i++) {
		//	for(int j = 0; j < 6; j++) {
		//		char c[128];
		//		hittingMasks[i].aBits[j].toMask128(c);
		//		printf("%128.128s ", c);
		//	}
		//	printf("%d\n", i);
		//}
		goto done;
	}
	//for(int c = 0; c < 81; c++)
	//	hittingMasks[c].clear();
	//for(int p = 0, k = 0; /*p < NUM_PACKS*/; p++) {
	//	for(int b = 0; b < 128; b++) {
	//		if(k >= nsets)
	//			goto done;
	//		const bm128 &m = bitSet[b];
	//		bm128 s = sets[k++];
	//		unsigned int m16;
	//		m16 = s.mask8(); //127 119 111 103 95 87 79 71 63 55 47 39 31 23 15 07
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[7].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[15].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[23].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[31].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[39].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[47].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[55].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[63].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[71].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[79].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //126 .. 6
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[6].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[14].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[22].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[30].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[38].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[46].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[54].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[62].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[70].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[78].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //125 .. 5
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[5].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[13].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[21].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[29].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[37].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[45].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[53].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[61].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[69].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[77].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //124 .. 4
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[4].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[12].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[20].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[28].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[36].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[44].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[52].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[60].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[68].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[76].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //123 .. 3
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[3].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[11].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[19].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[27].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[35].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[43].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[51].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[59].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[67].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[75].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //122 .. 2
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[2].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[10].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[18].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[26].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[34].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[42].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[50].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[58].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[66].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[74].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //121 .. 1
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[1].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[9].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[17].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[25].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[33].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[41].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[49].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[57].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[65].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[73].aBits[p] |= m;
	//		}
	//		s <<= 1;
	//		m16 = s.mask8(); //120 .. 80 .. 0
	//		if(m16) {
	//			if(m16 & 1) hittingMasks[0].aBits[p] |= m;
	//			if(m16 & 2) hittingMasks[8].aBits[p] |= m;
	//			if(m16 & 4) hittingMasks[16].aBits[p] |= m;
	//			if(m16 & 8) hittingMasks[24].aBits[p] |= m;
	//			if(m16 & 16) hittingMasks[32].aBits[p] |= m;
	//			if(m16 & 32) hittingMasks[40].aBits[p] |= m;
	//			if(m16 & 64) hittingMasks[48].aBits[p] |= m;
	//			if(m16 & 128) hittingMasks[56].aBits[p] |= m;
	//			if(m16 & 256) hittingMasks[64].aBits[p] |= m;
	//			if(m16 & 512) hittingMasks[72].aBits[p] |= m;
	//			if(m16 & 1024) hittingMasks[80].aBits[p] |= m;
	//		}
	//	}
	//}
done:
	//} //for t (timings)
	lsbBM = lsb;
	BitMask768 setMask;
	setMask.clear();

	int j = 128 * NUM_PACKS - nsets;
	for(int i = NUM_PACKS - 1; j > 0 ; i--, j -= 128) {
		if(j >= 128) {
			setMask.aBits[i] |= maskffff.m128i_m128i;
		}
		else {
			setMask.aBits[i] |= _mm_andnot_si128(maskLSB[128 - 1 - j].m128i_m128i, maskffff.m128i_m128i);
		}
	}
	iterateClueBM(setMask, 0);
}

void chunkProcessor::iterateClue(const bm128 *sets, const int *lsb, const int nsets) {
	bm128 *subsets;
	int *subsetLsbs;
	int maxPos = state[clueNumber].cluePosition;
	clueNumber--;
	int nsubsets;
	int minPos = clueNumber;
	if(nsets) {
		if(clueNumber == 0) { //last clue, rare
			uset u;
			u = maskLSB[state[0].maxPositionLimitsByUA];
			for(int s = 0; s < nsets; s++) {
				u &= sets[s];
				if(u.isZero()) {
					//no common clues => couldn't be hit at once => sorry
					clueNumber++;
					return;
				}
				//here u contains all possibilities for the last clue
				u.positionsByBitmap();
				for(int pos = 0; pos < u.nbits; pos++) {
					state[0].cluePosition = u.positions[pos];
					printf("checkPuzzle() from iterateClue()\n");
					checkPuzzle();
					if(nPuzzles && stopAtFirst) {
						clueNumber++;
						return;
					}
				}
			}
		}
		else {
			//Adjust the minimal position according to the first unavoidable 
			if(minPos < *lsb) {
				//this unavoidable couldn't be hit by the clues right of minPos
				minPos = *lsb;
			}
			if(maxPos > state[clueNumber].maxPositionLimitsByUA) {
				//there is a disjoint set at right which must be hit by this clue
				//skipped[clueNumber] += (maxPos - state[clueNumber].maxPositionLimitsByUA) * maxPositionLimitsByUAweight[clueNumber]; //debug
				maxPos = state[clueNumber].maxPositionLimitsByUA;
			}
			//subsets = (bm128 *)_mm_malloc(nsets * (sizeof(bm128) + sizeof(int)), 16);
			subsets = bm128::allocate(nsets + nsets / 2 + 1); //assume sizeof(int) <= 64
			subsetLsbs = (int*)&subsets[nsets];
			//iterate trough all possible positions
			for(int pos = minPos; pos < maxPos; pos++) {
				state[clueNumber].cluePosition = pos;
				nsubsets = 0;
				for(int s = 0; s < nsets; s++) {
					if(!sets[s].isBitSet(pos)) {
						subsetLsbs[nsubsets] = lsb[s];
						subsets[nsubsets++] = sets[s];
						//if(clueNumber == nClues - 3 && nsubsets == 128 * NUM_PACKS)
						//	break;
					}
				}
				if(nsubsets <= (128 * NUM_PACKS)) {
					switch2bm(subsets, subsetLsbs, nsubsets);
				}
				else {
					iterateClue(subsets, subsetLsbs, nsubsets);
				}
			}
			//_mm_free(subsets);
			bm128::deallocate(subsets);
		}
	}
	else { //all unavodables are hit
		printf("generatePuzzles() from iterateClue()\n");
		generatePuzzles();
	}
	clueNumber++;
}

void chunkProcessor::iterateClueBM(const BitMask768 &setMask, const int nFirst) {
	if(nFirst != INT_MAX) {
		//use next clue to hit the first unavoidable
		if(clueNumber == 1) { //last clue
			//hit the first unavoidable
			int pos = lsbBM[nFirst];
			int maxPos = state[0].maxPositionLimitsByUA;
			if(pos >= maxPos) {
				//we are sure the rightmost UA is not hit
				//skipped[0] += (pos - state[0].maxPositionLimitsByUA); //debug
				return;
			}

			if(pos >= state[1].cluePosition) {
				return;
			}
			if(maxPos > state[1].cluePosition) {
				maxPos = state[1].cluePosition;
			}
			//if(pos >= maxPos) {
			//	return;
			//}
			int restBits = setsBM[nFirst].toInt32() >> pos;
			for(int i = pos; i < maxPos; i++, restBits >>= 1) {
				if(restBits & 1) {
					if(setMask.isHittingAll(hittingMasks[i])) {
						state[0].cluePosition = i;
						checkPuzzle();
						if(nPuzzles && stopAtFirst) {
							return;
						}
					}
				}
			}
			return;
		}
		else {
			if(nPuzzles && stopAtFirst) {
				//clueNumber++;
				return;
			}
			//iterate next clue and recurse
			int maxPos = state[clueNumber--].cluePosition; //touch previous clue
			int minPos = clueNumber; //leave room for the rest clues
			//Adjust the minimal position according to the first unavoidable
			if(minPos < lsbBM[nFirst]) {
				//this unavoidable couldn't be hit by the clues right of minPos
				minPos = lsbBM[nFirst];
			}
			if(maxPos > state[clueNumber].maxPositionLimitsByUA) {
				//there is a disjoint set at right which must be hit by this clue
				//skipped[clueNumber] += (maxPos - state[clueNumber].maxPositionLimitsByUA) * maxPositionLimitsByUAweight[clueNumber]; //debug
				maxPos = state[clueNumber].maxPositionLimitsByUA;
			}

			////for the last three clues search for clique of size clueNumber disjoint to the reduced at left current UA
			//if(clueNumber == 2) { //only three clues (2, 1, and 0) remain
			//	if(setMask.hasUnhitClique4(setsBM, maxPos)) { //search for any clique of size 3
			//		clueNumber++;
			//		return;
			//	}
			//}

			////for the last two clues search for clique of size clueNumber disjoint to the reduced at left current UA
			//if(clueNumber == 1) { //only two clues (1 and 0) remain
			//	if(setMask.hasUnhitClique3(setsBM, maxPos)) { //search for any clique of size 3
			//		clueNumber++;
			//		return;
			//	}
			//}

#if 0
			//bool checkTreshold = clueNumber <= theClueIterator->theClique.size - 2 && clueNumber >= 3;
			//bool checkTreshold = clueNumber <= theClueIterator->theClique.size - 2 && clueNumber >= 5;
			bool checkTreshold = clueNumber >= 8;
#endif
			for(int pos = minPos; pos < maxPos; pos++) {
#if 0
				//if(checkTreshold && pos >= state[clueNumber - 1].maxPositionLimitsByUA) { //check once, as earlier as possible
				if(checkTreshold && pos >= 31) { //check once, as earlier as possible
					const UATable &tr = theClueIterator->tresholds[clueNumber];
					if(&tr && tr.size) {
						bm128 done;
						done.clear();
						for(int i = clueNumber + 1; i < nClues; i++) {
							done.setBit(state[i].cluePosition);
						}
						for(int i = 0; i < tr.size; i++) {
							if(done.isDisjoint(tr.rows[i])) {
								//printf("(%d)", clueNumber);
								//printf("%d,", i);
								clueNumber++;
								return;
							}
						}
						//printf("(-%d,%d,%d)", clueNumber, pos, tr.size);
						//printf("-");
					}
					checkTreshold = false;
				}
#endif

				state[clueNumber].cluePosition = pos;
				BitMask768 newSetMask;
				int nFirst;
				////_mm_prefetch((char *)&newSetMask, _MM_HINT_T0); //slower
				_mm_prefetch(((char *)&nFirst), _MM_HINT_T0); //insignificant improvement
				nFirst = newSetMask.hit(setMask, hittingMasks[pos]);
				iterateClueBM(newSetMask, nFirst);
			}
		}
	}
	else { //all unavodables are hit, rare
		clueNumber--;
		generatePuzzles();
	}
	clueNumber++;
}

void chunkProcessor::minimizeFixed(char *puzzle) {
	//return;
	//static const int maxSetsToUse = 2000;
	static const int maxSetsToUse = 64;
	//if(nClues - nFixed > 11) return;
	//convert known givens to bitmap
	bm128 knownsBM;
	knownsBM.clear();
	for(int i = 0; i < nClues - nFixed; i++) {
		knownsBM.setBit(theClueIterator->theMapper.i2g[state[i].cluePosition]);
	}
	//compose a list of UA not hit by the knowns
	bm128 *sets = bm128::allocate(maxSetsToUse);
	int nSets = 0;
	for(usetListBySize::const_iterator s = theClueIterator->g.usetsBySize.begin(); nSets < maxSetsToUse && s != theClueIterator->g.usetsBySize.end(); s++) {
		if(s->isDisjoint(knownsBM)) {
			sets[nSets++] = *s;
		}
	}
	//printf("nSets=%d\n", nSets);
	int fixedPositions[81];
	for(int i = 0; i < nFixed; i++) { //TODO: move it out of the loop
		fixedPositions[i] = theClueIterator->theMapper.i2g[80 - i];
	}
	int maxPlaced = theClueIterator->huntClues - nClues + nFixed;
	simpleIterator si(nFixed, maxPlaced);
	bm128 cluesBM;
	do {
		cluesBM.clear();
		for(int i = 0; i < maxPlaced; i++) {
			cluesBM.setBit(fixedPositions[si.positions[i]]);
		}
		for(int i = 0; i < nSets; i++) {
			if(cluesBM.isDisjoint(sets[i])) {
				goto next; //skip this clue combination
			}
		}
		//all UA are hit
		//prepare the puzzle
		for(int i = 0; i < nFixed; i++) {
			puzzle[fixedPositions[i]] = 0;
		}
		for(int i = 0; i < maxPlaced; i++) {
			puzzle[fixedPositions[si.positions[i]]] = theClueIterator->g.digits[fixedPositions[si.positions[i]]];
		}
		nChecked++;
		if((solve(theClueIterator->g.gridBM, puzzle, 2)) == 1) {
			puzText pt;
			for(int i = 0; i < 81; i++) {
				pt.chars[i] = puzzle[i] + '0';
			}
			minimizedPuzzles.insert(pt);
		}
next:
		;
	} while(si.increment());

	bm128::deallocate(sets);
}
bool chunkProcessor::checkPuzzle() {
	char puzzle[82];
	for(int i = 0; i < 82; i++) {
		puzzle[i] = 0;
	}
	for(int i = 0; i < nClues; i++) {
	//for(int i = clueNumber; i < nClues; i++) { //wrong --> don't expect all nClues set; work with shorter ones when they hit all UA sets
		int j = theClueIterator->theMapper.i2g[state[i].cluePosition];
		puzzle[j] = theClueIterator->g.digits[j];
		//if(j == 74) { //debug
		//	fprintf(stderr, "\nerr:\t%d\t%d\t%d\t%d\n", theChunk.numClues, theChunk.cluePositions[0], theChunk.cluePositions[1], i);
		//	int zzz = 2;
		//}
	}
	nChecked++;
	//if(opt.verbose) { //print any puzzle, including invalid ones
	//	char anyPuz[81];
	//	for(int i = 0; i < 81; i++) {
	//		anyPuz[i] = puzzle[i] + '0';
	//	}
	//	printf("puz %81.81s\n", anyPuz);
	//}
	if((solve(theClueIterator->g.gridBM, puzzle, 2)) == 1) {
	//if(!hasEDSolution(theClueIterator->g.gridBM, puzzle)) {
		if(opt.scanOpt->minimalsOnly) { //skip non-minimal puzzles
			for(int i = 0; i < 81; i++) {
				char c;
				if((c = puzzle[i])) {
					puzzle[i] = 0;
					if((solve(theClueIterator->g.gridBM, puzzle, 2)) == 1) { //non-minimal
						return false;
					}
					puzzle[i] = c;
				}
			}
		}
		nPuzzles++;
		puzText pt;
		for(int i = 0; i < 81; i++) {
			pt.chars[i] = puzzle[i] + '0';
		}
		//printf("%81.81s\n", pt.chars);
		if(nFixed) {
			if(opt.scanOpt->storepseudos || opt.scanOpt->scanpuzzleset) {
				validPuzzles.push_back(pt);
			}
			else {
				minimizeFixed(puzzle);
			}
		}
		else {
			validPuzzles.push_back(pt);
		}
		return true;
	}
	return false;
}

void chunkProcessor::generatePuzzles() {
	//printf("generate puzzles, clue number=%d", clueNumber);
	if(nPuzzles && stopAtFirst) {
		return;
	}
	//if(opt.scanOpt->shorter) {
	//	if(checkPuzzle()) {	//valid puzzle of size < numClues is found
	//		return;
	//	}
	//}
	if(clueNumber == 0) {
		for(int i = 0; i < state[1].cluePosition; i++) {
			state[0].cluePosition = i;
			checkPuzzle();
			if(nPuzzles && stopAtFirst) {
				return;
			}
		}
	}
	else {
		int maxpos = state[clueNumber--].cluePosition;
		for(int i = clueNumber; i < maxpos; i++) {
			state[clueNumber].cluePosition = i;
			generatePuzzles();
		}
		clueNumber++;
	}
}
#if 0
int fractalPattern() {
	char completion[81];
	for(int i = 0; i < 27; i++) {
		completion[i] = 0;
	}
	opt.scanOpt->storepseudos = true;
	unsigned int *bandCompletions[416];
	int nCompletions[416];
	//fractal
	//1.1......
	//.1.......
	//1.1......
	//.........
	//.........
	//.........
	//.........
	//.........
	//.........
	//..............................................................1......11.......11. //minlex
	char fFractalCanon[81];
	for(int i = 0; i < 81; i++) {
		fFractalCanon[i] = "000000000000000000000000000000000000000000000000000000000000001000000110000000110"[i] - '0';
	}
	const char fPermBase[3][3] = {
		{-1, 0,-1},
		{ 0,-1, 0},
		{-1, 0,-1}
	};
	const char fRestPerm[3][3] = {{0,1,0},{1,0,2},{2,2,1}}; //target index = fRestPerm[source index][permutation]
	char fPerm[81][6][6];
	bm128 fPermBM[81];
	for(int i = 0; i < 81; i++) {
		fPermBM[i].clear();
	}
	for(int rp1 = 0, p = 0; rp1 < 3; rp1++) {
		for(int rp2 = 0; rp2 < 3; rp2++) {
			for(int cp1 = 0; cp1 < 3; cp1++) {
				for(int cp2 = 0; cp2 < 3; cp2++) {
					for(int r = 0; r < 3; r++) {
						for(int c = 0; c < 3; c++) {
							fPerm[p][r][c] = fPermBase[fRestPerm[r][rp1]][fRestPerm[c][cp1]];
							fPerm[p][r][c + 3] = fPermBase[fRestPerm[r][rp1]][fRestPerm[c][cp2]];
							fPerm[p][r + 3][c] = fPermBase[fRestPerm[r][rp2]][fRestPerm[c][cp1]];
							fPerm[p][r + 3][c + 3] = fPermBase[fRestPerm[r][rp2]][fRestPerm[c][cp2]];
							if(fPermBase[fRestPerm[r][rp1]][fRestPerm[c][cp1]]) fPermBM[p].setBit(3 + (r + 3) * 9 + c);
							if(fPermBase[fRestPerm[r][rp1]][fRestPerm[c][cp2]]) fPermBM[p].setBit(3 + (r + 3) * 9 + c + 3);
							if(fPermBase[fRestPerm[r][rp2]][fRestPerm[c][cp1]]) fPermBM[p].setBit(3 + (r + 6) * 9 + c);
							if(fPermBase[fRestPerm[r][rp2]][fRestPerm[c][cp2]]) fPermBM[p].setBit(3 + (r + 6) * 9 + c + 3);
						}
					}
					p++;
				}
			}
		}
	}

	//for(int band = 0; band < 416; band++) {
	for(int band = 260; band < 261; band++) { //debug
	//	if(band != 130 && band != 260) continue; //debug
		grid g;
		memcpy(g.digits, bands[band], 81);
		digit2bitmap(g.digits, g.gridBM);
		//prepare a pseudo-puzzle for UA search
		for(int i = 27; i < 81; i++) {
			completion[i] = bands[band][i];
		}
		unsigned long long nSol = g.findUAbyPuzzle(completion);
		//now g has valid UA list
		//printf("%d\t%llu\t%d\t", band + 1, nSol, (int)g.usetsBySize.size());
		clueIterator ci(g);
		ci.iterateBand(5);
		bandCompletions[band] = (unsigned int*) malloc((nCompletions[band] = (int)((ci.minimizedPuzzles.size()) * sizeof(unsigned int))));
		int n = 0;
		//fractal
		char fBand[81], fCanon[81];

		for(puzTextSet::const_iterator p = ci.minimizedPuzzles.begin(); p != ci.minimizedPuzzles.end(); p++) {
			unsigned int m = 0, k = 0;
			int fBox[3] = {0,0,0};
			int fRow[3] = {0,0,0};
			int fCol[3] = {0,0,0};
			for(int i = 0; i < 27; i++) {
				if(p->chars[i] != '0') {
					fBox[boxByCellIndex[i]]++;
					fRow[rowByCellIndex[i]]++;
					fCol[colByCellIndex[i] % 3]++;
					m |= Digit2Bitmap[i + 1];
					k++;
				}
			}
			//fractal
			if(k != 5) continue;
			if(fBox[0] != 5 && fBox[1] != 5 && fBox[2] != 5) continue;
			if(fRow[0] * fRow[1] * fRow[2] != 2*2*1) continue;
			if(fCol[0] * fCol[1] * fCol[2] != 2*2*1) continue;
			memset(fBand, 0, 81);
			for(int i = 0; i < 27; i++) {
				if(p->chars[i] != '0')
					fBand[i] = 1;
			}
			subcanon(fBand, fCanon);
			if(memcmp(fFractalCanon, fCanon, 81)) continue;

			bandCompletions[band][n++] = m | (k << 27);
		}
		//fractal
		nCompletions[band] = n;

		//printf("%d %d\n", band + 1, n);
	}
	//return 0;

	//Prepare the positional part of the transformations
	transformer tr108[108];
	for(int box = 9, i = 0; box < 18; box+= 3) {
		for(int c123 = 0; c123 < 6; c123++) {
			for(int r123 = 0; r123 < 6; r123++) {
				tr108[i].box = box;
				for(int c = 0; c < 3; c++)
					tr108[i].col[c] = tc.perm[c123][c];
				for(int c = 3; c < 9; c++)
					tr108[i].col[c] = c;
				for(int r = 0; r < 3; r++)
					tr108[i].row[r] = tc.perm[r123][r];
				for(int r = 3; r < 9; r++)
					tr108[i].row[r] = r;
				//for(int m = 0; m < 10; m++) {
				//	tr108[i].map[m] = m;
				//}
				tr108[i].map[0] = 0;
				i++;
			}
		}
	}
	opt.uaOpt->digit4Search = true;
	opt.uaOpt->randomSearch = true;
	opt.uaOpt->nAttempts = 200;
	opt.uaOpt->nCells = 52;
	opt.uaOpt->mcnNoAutoUA = true;
	opt.uaOpt->maxuasize = 18;
	char gr[9][9], puzzle[9][9];
	for(int i = 27; i < 81; i++)
		((char*)gr)[i] = 0;
	puzzleSet goods;
	//band permutation mask. To band permutations where all complrtions have empty crossing box 1.
	//000000111000000111000000111
	//000111000000111000000111000
	//111000000111000000111000000
	int bpmasks[3] = {0x1C0E07,0xE07038,0x70381C0}; //bits in box 1, 2, 3 set. Box 1 is rightmost in the bitmap.
	unsigned long long gridCount = 0;
	//Iterate bands
	//for(int band = 0; band < 416; band++) {
	for(int band = 130; band < 131; band++) {
		int nBandCompletions = nCompletions[band];
		if(0 == nBandCompletions)
			continue; // skip empty bands
		//Iterate stacks
		for(int stack = band; stack < 416; stack++) {
		//for(int stack = 260; stack < 261; stack++) {
			int nStackCompletions = nCompletions[stack];
			if(0 == nStackCompletions)
				continue; // skip empty stacks
			puzzleSet ps;
			//Iterate band permutations
			for(int bandPerm = 0; bandPerm < 3; bandPerm++) {
				//fractal
				//skip if there is no any band completion with box1 populated
				int bpmask = bpmasks[bandPerm];
				bool found = false;
				for(int bc = 0; bc < nBandCompletions; bc++) {
					if(bandCompletions[band][bc] & bpmask) {
						found = true;
						break;
					}
				}
				if(! found)
					continue;

				for(int i = 0; i < 27; i++)
					((char*)gr)[i] = bands[band][((unsigned char*)tc.swap[bandPerm])[i]];
				//Iterate stack permutations
				for(int perm = 0; perm < 108; perm++) {
					//Relabel to match values in the crossing box 1
					transformer &t = tr108[perm];
					for(int r = 0; r < 3; r++) {
						for(int c = 0; c < 3; c++) {
							t.map[bands[stack][tc.swap[t.box][t.row[r]][t.col[c]]]] = gr[r][c];
						}
					}
					//remap the stack to boxes 4,7
					for(int r = 3; r < 9; r++) {
						for(int c = 0; c < 3; c++) {
							gr[r][c] = t.map[bands[stack][tc.swap[t.box][t.row[r]][t.col[c]]]];
						}
					}
					//check whether the 27+18 clue puzzle has any solution
					char sol1[9][9];
					if(0 == solve((char*)gr, 1, (char*)sol1)) {
						z1++; //debug
						goto nextStackPermutation;
					}
					//Iterate band completions
					for(int bc = 0; bc < nBandCompletions; bc++) {
						int nBox1BandClues = 0;
						bm128 crossBM;
						crossBM.clear();
						for(int i = 0; i < 27; i++) {
							//i is target index (mapped)
							//bandCompletions[band][bc] is source (unmapped) bitmap
							//((unsigned char*)tc.swap[bandPerm])[i] is source index (unmapped)
							//if(Digit2Bitmap[1 + ((unsigned char*)tc.swap[bandPerm])[i]] & bandCompletions[band][bc]) {
							if(Digit2Bitmap[1 + tc.swap[bandPerm][0][i]] & bandCompletions[band][bc]) {
								puzzle[0][i] = gr[0][i];
								if(0 == boxByCellIndex[i]) {
									crossBM.setBit(i);
									nBox1BandClues++;
								}
							}
							else {
								((char*)puzzle)[i] = 0;
							}
						}

						//fractal
						if(nBox1BandClues == 0) continue;

						//Iterate stack completions
						int minStackCompletion = 0;
						if(band == stack)
							minStackCompletion = bc; //don't cross completions for the same stack twice (a,b) then (b,a)
						for(int sc = minStackCompletion; sc < nStackCompletions; sc++) {
							//Ignore the non-matching box 1 combinations
							for(int r = 0; r < 3; r++) {
								for(int c = 0; c < 3; c++) {
									if((0 == puzzle[r][c]) != (0 == (Digit2Bitmap[1 + tc.swap[t.box][t.row[r]][t.col[c]]] & bandCompletions[stack][sc]))) {
										z0++; //debug
										goto nextStackCompletion;
									}
								}
							}
							//remap the stack completion to boxes 4,7
							for(int r = 3; r < 9; r++) {
								for(int c = 0; c < 3; c++) {
									puzzle[r][c] = Digit2Bitmap[1 + tc.swap[t.box][t.row[r]][t.col[c]]] & bandCompletions[stack][sc] ? gr[r][c] : 0;
								}
							}

							//check whether there is unhit UA entirely located in boxes 1,2,3,4,7
							char sol2[9][9];
							memcpy(sol2, puzzle, 81);
							for(int r = 3; r < 9; r++) {
								for(int c = 3; c < 9; c++) {
									sol2[r][c] = sol1[r][c];
								}
							}
							if(2 == solve((char*)sol2, 2)) {
								z2++; //debug
								goto nextStackCompletion;
							}

							z3++; //debug
							//if(z3 % 10000 == 0) {
							//	printf("%llu\t%llu\t%llu\t%llu\t", z0, z1, z2, z3);
							//	printf("%d\t%d\t%d\t%d\n", band + 1, bandPerm, stack + 1, perm);
							//}

							{
								//int more = 17 - ((bandCompletions[stack][sc] >> 27) + (bandCompletions[band][bc] >> 27) - nBox1BandClues);
								//ch81 pCanon;
								//subcanon((char*)puzzle, pCanon.chars);
								//puzzleSet::iterator duplicate = ps.find(pCanon);
								//if(duplicate == ps.end()) {
								//	ps.insert(pCanon);
									////prepare the grids
									//const int maxGrids = 10;
									//char sol3[maxGrids][81];
									//int nGrids = (int)solve((char*)gr, maxGrids, (char*)sol3);
									//for(int ng = 0; ng < nGrids; ng++) {
									//	grid g;
									//	memcpy(g.digits, (char*)sol3[ng], 81);
									//	digit2bitmap(g.digits, g.gridBM);
									//	//find UA only if needed
									//	g.findInitialUA();
									//	printf("%d UA\n", g.usetsBySize.size()); //debug
									//	clueIterator ci(g);
									//	for(int i = 0; i < 81; i++) {
									//		if(((char*)gr)[i]) {
									//			if(((char*)puzzle)[i]) {
									//				ci.theClique.fixedClues.setBit(i);
									//			}
									//			else {
									//				ci.theClique.fixedNonClues.setBit(i);
									//			}
									//		}
									//	}
									//	ci.theClique.fixedClues.positionsByBitmap();
									//	ci.theClique.fixedNonClues.positionsByBitmap();
									//	//do the job
									//	ci.iterateBandStack(17);
									//}
								//}
								//compose a subgrid with band1 and stack1 populated except the givens in box1
								ch81 pCanon;
								char pIsDone[9][9];
								memcpy(pIsDone, gr, 81);
								for(int r = 0; r < 3; r++) {
									for(int c = 0; c < 3; c++) {
										if(puzzle[r][c])
											pIsDone[r][c] = 0;
									}
								}
								subcanon((char*)pIsDone, pCanon.chars);
								puzzleSet::iterator duplicate = ps.find(pCanon);
								if(duplicate != ps.end()) {
									printf("-");
									goto nextStackCompletion;
								}
								printf("+");
								ps.insert(pCanon);
								const unsigned long long maxGrids = 6000;
								char g[maxGrids][9][9];
								unsigned long long nGrids = solve((char*)gr, maxGrids, (char*)g);
								gridCount += nGrids;
								//printf("\tBand=%d\tStack=%d\tBPerm=%d\tSPerm=%d\tBCompletion=%d\tSCompletion=%d\tGrids=%llu\n", band + 1, stack + 1, bandPerm, perm, bc, sc, nGrids);
								//compose the common part of the puzzle candidates
								//char pc[9][9];
								//for(int r = 0; r < 3; r++) {
								//	for(int c = 0; c < 3; c++) {
								//		pc[r][c] = puzzle[r][c];
								//	}
								//}

								//goto nextStackCompletion; //debug

								bm128 fPuzBM[81];
								for(int i = 0; i < 81; i++) {
									fPuzBM[i] = fPermBM[i] | crossBM;
								}

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif //_OPENMP
								for(int n = 0; n < (int)nGrids; n++) {
									grid gc;
									memcpy(gc.digits, g[n], 81);
									//digit2bitmap(gc.digits, gc.gridBM);
									gc.findShortUA();
									char validPuzzles[81][81];
									//int nValid = gc.checkPuzSetValidity(fPuzBM, 81, validPuzzles[0]);
									int nValid = gc.checkPuzSetValidity(crossBM, fPuzBM, 81, validPuzzles[0]);
									for (int v = 0; v < nValid; v++) {
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
										{
											ch81 good;
											char buf[81];
											subcanon(validPuzzles[v], good.chars);
											puzzleSet::iterator d = goods.find(good);
											if(d == goods.end()) {
												goods.insert(good);
												good.toString(buf);
												printf("\n%81.81s\t%d\t%d\n", buf, band + 1, stack + 1);
											}
										}
									}
//									for(int mask = 0; mask < 81; mask++) {
//										for(int r = 0; r < 6; r++) {
//											for(int c = 0; c < 6; c++) {
//												pc[r + 3][c + 3] = g[n][r + 3][c + 3] & fPerm[mask][r][c];
//											}
//										}
//										unsigned long long nPcSol = solve(gr.gridBM, (char*)pc, 2);
//										//unsigned long long nPcSol = solve((char*)pc, 2);
//										if(nPcSol == 1) {
//#ifdef _OPENMP
//#pragma omp critical
//#endif //_OPENMP
//											{
//												ch81 good;
//												char buf[81];
//												subcanon((char*)pc, good.chars);
//												puzzleSet::iterator d = goods.find(good);
//												if(d == goods.end()) {
//													goods.insert(good);
//													good.toString(buf);
//													printf("\n%81.81s\t%d\t%d\n", buf, band + 1, stack + 1);
//												}
//											}
//										}
//									}
								}

								//place the rest of the clues in boxes 5,6,8,9
								
								////debug
								//printf("%d\t%d\t%d\n", bandCompletions[stack][sc] >> 27, bandCompletions[band][bc] >> 27, nBox1BandClues);
								//ch81::toString(puzzle, buf);
								//printf("%81.81s\t%d\n", buf, more);

								//fflush(NULL);
							}
nextStackCompletion:
							;
						}
					}
					////debug
					//printf("B=%d\tBp=%d\tS=%d\tSp=%d\tn=%llu\tnED=%d\n", band + 1, bandPerm, stack + 1, perm, z3, ps.size());
					//z3 = 0;
nextStackPermutation:
					;
				}
//nextBandPermutation:
//				;
			}
			////debug
			//printf("B=%d\tS=%d\tspInv=%llu\tn=%llu\tnInv=%llu\tnED=%d\n", band + 1, stack + 1, z1, z3, z2, ps.size());
			//z3 = z2 = z1 = 0;
//nextStack:
//				;
		}
//nextBand:
//		;
	}
	printf("\nTotal Grids Processed=%llu\n", gridCount);
	printf("Valid Puzzles=%d, solver called %llu times\n", (int)goods.size(), z5);
	for(puzzleSet::const_iterator g = goods.begin(); g != goods.end(); g++) {
		char buf[81];
		g->toString(buf);
		printf("%81.81s\n", buf);
	}

	for(int band = 0; band < 416; band++) {
		free(bandCompletions[band]);
	}

	return 0;
}
#endif

int scanFixedBandStack() { //leave one band and one stack from the original puzzle, scan other 4 boxes for valid completions with same number of clues
	char buf[2000];
	//opt.scanOpt->storepseudos = true;
	opt.scanOpt->scanpuzzleset = true;
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		int puzSize = puz.fromString(buf);
		grid g;
		if(0 == solve(puz.chars, 1, g.digits)) { //check for any valid completion
			fprintf(stderr, "No completion for %81.81s\n", buf);
			continue;
		}
		g.findUA12(); //find all 4..12 digit UA
		//now g has valid UA list
		digit2bitmap(g.digits, g.gridBM);
		for(int band = 0; band < 3; band++) { //any cell in this band is fixed given/non-given
			for(int stack = 0; stack < 3; stack++) { //any cell in this stack is fixed given/non-given
				clueIterator ci(g);
				//prepare fixed clues and fixed non-clues
				for(int i = 0; i < 81; i++) {
					if(bandByCellIndex[i] == band || stackByCellIndex[i] == stack) {
						//fixed part
						if(puz.chars[i]) {
							ci.theClique.fixedClues.setBit(i);
						}
						else {
							ci.theClique.fixedNonClues.setBit(i);
						}
					}
				}
				ci.theClique.fixedClues.positionsByBitmap();
				ci.theClique.fixedNonClues.positionsByBitmap();
				ci.iterateFixed(puzSize);

				for(puzTextSet::const_iterator p = ci.minimizedPuzzles.begin(); p != ci.minimizedPuzzles.end(); p++) {
					printf("%81.81s\n", p->chars);
				}
			}
		}
	}
	return 0;
}

typedef set<int> intset;

int allBandCompletions() {
	char completion[81];
	for(int i = 0; i < 27; i++) {
		completion[i] = 0;
	}
	opt.scanOpt->storepseudos = true;
	int nCompletions[416];

	for(int band = 0; band < 416; band++) {
		intset res[6];
		grid g;
		memcpy(g.digits, bands[band], 81);
		digit2bitmap(g.digits, g.gridBM);
		//prepare a pseudo-puzzle for UA search
		for(int i = 27; i < 81; i++) {
			completion[i] = bands[band][i];
		}
		unsigned long long nSol = g.findUAbyPuzzle(completion);
		//now g has valid UA list
		//printf("%d\t%llu\t%d\t", band + 1, nSol, (int)g.usetsBySize.size());
		for(usetListBySize::const_iterator x = g.usetsBySize.begin(); x != g.usetsBySize.end(); x++) {
			char u[88];
			x->toMask81(u);
			printf("%81.81s\t%d\n", u, band + 1);
		}
//		for(int numClues = 2; numClues < 8; numClues++) {
//			clueIterator ci(g);
//			ci.iterateBand(numClues);
//			if(numClues - 2) {
//				//generate {+1} from the previous pass
//				for(intset::const_iterator old = res[numClues - 3].begin(); old != res[numClues - 3].end(); old++) {
//					int mask = *old;
//					for(int i = 0; i < 27; i++) {
//						if(Digit2Bitmap[i + 1] & mask) continue;
//						res[numClues - 2].insert(mask | Digit2Bitmap[i + 1]);
//					}
//				}
//			}
//
//			for(puzTextSet::const_iterator p = ci.minimizedPuzzles.begin(); p != ci.minimizedPuzzles.end(); p++) {
//				int mask = 0;
//				int numActualClues = 0;
//				for(int i = 0; i < 27; i++) {
//					if(p->chars[i] != '0') {
//						mask |= Digit2Bitmap[i + 1];
//						numActualClues++;
//					}
//				}
//				if(numActualClues == numClues) {
//					res[numClues - 2].insert(mask);
//				}
////				char outbuf[32];
////				for(int i = 0; i < 27; i++) {
////					outbuf[i] = p->chars[i] != '0' ? '1' : '.';
////				}
////				printf("%d\t%d\t%27.27s\n", band + 1, numClues, outbuf);
//				//printf("%d\t%d\t%d\n", band + 1, numClues, mask);
//			}
//			//printf("%d\t%d\t%d\n", band + 1, numClues, (int)ci.minimizedPuzzles.size());
//			//printf("%d\t%d\t%d\n", band + 1, numClues, (int)res[band][numClues - 2].size());
//
//			//write results
//			printf("#%d\t%d\t%d\n", band + 1, numClues, (int)res[numClues - 2].size()); //header
//			for(intset::const_iterator old = res[numClues - 2].begin(); old != res[numClues - 2].end(); old++) {
//				int mask = *old;
//				printf("%d\n", mask);
//			}
//		}
//		//printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\n", band + 1, (int)res[0].size(), (int)res[1].size(), (int)res[2].size(), (int)res[3].size(), (int)res[4].size(), (int)res[5].size());
	}
	return 0;
}

#if 0
int allBandGrids() {
	//Prepare the positional part of the transformations
	transformer tr108[108];
	for(int box = 9, i = 0; box < 18; box+= 3) {
		for(int c123 = 0; c123 < 6; c123++) {
			for(int r123 = 0; r123 < 6; r123++) {
				tr108[i].box = box;
				for(int c = 0; c < 3; c++)
					tr108[i].col[c] = tc.perm[c123][c];
				for(int c = 3; c < 9; c++)
					tr108[i].col[c] = c;
				for(int r = 0; r < 3; r++)
					tr108[i].row[r] = tc.perm[r123][r];
				for(int r = 3; r < 9; r++)
					tr108[i].row[r] = r;
				//tr108[i].map[0] = 0;
				i++;
			}
		}
	}
	//unsigned int stack23BM[9];
	char gr[9][9];
	//char puzzle[9][9];
	for(int i = 27; i < 81; i++)
		((char*)gr)[i] = 0;
	unsigned long long gridCount = 0;
	unsigned long long maxComplCount = 0;
	unsigned long long minComplCount = INT_MAX;
	//Iterate band1
	for(int band = 0; band < 416; band++) {
	//for(int band = 0; band < 1; band++) { //debug
		//for(int i = 3; i < 9; i++)
		//	stack23BM[i] = 0;
		for(int i = 0; i < 27; i++) {
			((char*)gr)[i] = bands[band][i];
			//stack23BM[i % 9] |= Digit2Bitmap[bands[band][i]];
		}
		//gr[3][0] = 2;
		//gridCount += solve((char*)gr, ULLONG_MAX);
		//Iterate stack 1
		for(int stack = band; stack < 416; stack++) {
			puzzleSet ps;
			//Iterate permutations of stack1
			for(int perm = 0; perm < 108; perm++) {
				//Relabel to match values in the crossing box 1
				transformer &t = tr108[perm];
				for(int r = 0; r < 3; r++) {
					for(int c = 0; c < 3; c++) {
						t.map[bands[stack][tc.swap[t.box][t.row[r]][t.col[c]]]] = gr[r][c];
					}
				}
				////skip stack1 permutation if r4c1 != 2
				//if(t.map[bands[stack][tc.swap[t.box][t.row[3]][t.col[0]]]] != 2)
				//	continue;
				//remap the stack to boxes 4,7
				for(int r = 3; r < 9; r++) {
					for(int c = 0; c < 3; c++) {
						gr[r][c] = t.map[bands[stack][tc.swap[t.box][t.row[r]][t.col[c]]]];
					}
				}

				unsigned long long complCount = solve((char*)gr, 1);
				if(complCount) gridCount++;
				//unsigned long long complCount = solve((char*)gr, INT_MAX);
				//gridCount += complCount;
				//if(maxComplCount < complCount)
				//	maxComplCount = complCount;
				//if(minComplCount > complCount)
				//	minComplCount = complCount;

//				//Iterate band2
//				for(int band2 = band; band2 < 416; band2++) {
//					//Iterate left box of band2
//					transformer tb2;
//					for(int band2Box = 0; band2Box < 3; band2Box++) {
//						tb2.box = band2Box;
//						//Iterate row permutation of band2
//						for(int band2rp = 0; band2rp < 6; band2rp++) {
//							for(int r = 0; r < 3; r++)
//								tb2.row[r + 3] = tc.perm[band2rp][r];
//							//Iterate first 3 columns' permutation of band2
//							for(int band2cp = 0; band2cp < 6; band2cp++) {
//								for(int c = 0; c < 3; c++)
//									tb2.col[c] = tc.perm[band2cp][c];
//								//relabel band2 to match crossing box 4
//								for(int r = 3; r < 6; r++) {
//									for(int c = 0; c < 3; c++) {
//										tb2.map[bands[band2][tc.swap[tb2.box][tb2.row[r]][tb2.col[c]]]] = gr[r + 3][c];
//									}
//								}
//								//order cols 4,5,6,7,8,9 of band 2
//								for(int band2stack23perm = 0; band2stack23perm < 2; band2stack23perm++) {
//									for(int band2stack2cp = 0; band2stack2cp < 6; band2stack2cp++) {
//										for(int c = 3; c < 6; c++) {
//											tb2.col[c] = 3 + band2stack23perm * 3 + tc.perm[band2stack2cp][c % 3];
//											for(int r = 3; r < 6; r++) {
//												gr[r][c] = tb2.map[bands[band2][tc.swap[tb2.box][tb2.row[r]][tb2.col[c]]]];
//												if(Digit2Bitmap[gr[r][c]] & stack23BM[3 + band2stack23perm * 3 + tc.perm[band2stack2cp][c % 3]])
//													goto nextBand2stack2cp;
//											}
//										}
//										for(int band2stack3cp = 0; band2stack3cp < 6; band2stack3cp++) {
//											for(int c = 6; c < 9; c++) {
//												tb2.col[c] = 3 + (1 - band2stack23perm) * 3 + tc.perm[band2stack3cp][c % 3];
//												for(int r = 3; r < 6; r++) {
//													gr[r][c] = tb2.map[bands[band2][tc.swap[tb2.box][tb2.row[r]][tb2.col[c]]]];
//													if(Digit2Bitmap[gr[r][c]] & stack23BM[3 + (1 - band2stack23perm) * 3 + tc.perm[band2stack3cp][c % 3]])
//														goto nextBand2stack3cp;
//												}
//											}
//											gridCount++;
//nextBand2stack3cp:
//											;
//										}
//nextBand2stack2cp:
//										;
//									}
//								}
//							}
//						}
//					}
//				}
//nextStackPermutation:
//				;
			}
////nextStack:
////				;
		}
//nextBand:
//		;
		printf("."); //debug
		//printf("%d (%llu,%llu) %llu\n", band, minComplCount, maxComplCount, gridCount);
	}
	printf("\nTotal Grids Processed=%llu\n", gridCount);
	return 0;
}
#endif
//int allBand2Completions() {
//	char completion[81];
//	for(int i = 27; i < 81; i++) {
//		completion[i] = 0;
//	}
//	opt.scanOpt->storepseudos = true;
//	//for(int band = 0; band < 416; band++) {
//	for(int band = 0; band < 1; band++) {
//		grid g;
//		memcpy(g.digits, bands[band], 81);
//		digit2bitmap(g.digits, g.gridBM);
//		//prepare a pseudo-puzzle for UA search
//		for(int i = 0; i < 27; i++) {
//			completion[i] = bands[band][i];
//		}
//		unsigned long long nSol = g.findUAbyPuzzle(completion);
//		//now g has valid UA list
//		printf("%d\t%llu\t%d\t", band + 1, nSol, (int)g.usetsBySize.size());
//		clueIterator ci(g);
//		ci.iterateBand2(1);
//	}
//	return 0;
//}

int scanGridFromFile() {
	const char* fname = opt.scanOpt->gridFileName;
	const int nClues = opt.scanOpt->nClues;
	grid g;
	g.fname = fname;
	if(g.readFromFile())
		return -1;
	g.readUAFromFile();
	clueIterator ci(g);
	switch(opt.scanOpt->fixMode) {
		case scanOptions::fix1box:
			ci.iterateBoxes(nClues);
			break;
		case scanOptions::fixband:
			//ci.iterateBands(nClues);
			ci.iterateBands(nClues);
			break;
		case scanOptions::fix2digits:
			ci.iterate2digits(nClues);
			break;
		case scanOptions::fix2boxes:
			ci.iterate2Boxes(nClues);
			break;
		case scanOptions::fix1digit:
			ci.iterateDigits(nClues);
			break;
		case scanOptions::fix3digits:
			ci.iterate3digits(nClues);
			break;
		case scanOptions::fix4digits:
			ci.iterate4digits(nClues);
			break;
		case scanOptions::fixauto:
			ci.iterateWhole(nClues);
			break;
		case scanOptions::exhaustive:
		default:
			ci.iterateWhole(nClues);
			break;
	}
	return 0;
}
int scanUnavForPuzzles() {
	char buf[3000];
	int nClues = opt.scanOpt->nClues;
	if(nClues <= 1)
		return 1; //stupid parameter
	if(nClues > 20)
		return 1; //stupid parameter
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		int puzSize = puz.fromString(buf);
		if(puzSize <= nClues - 1)
			continue; //no room for numclues
		int	nSol = (int)solve(puz.chars, 30, buf);
		if(nSol >= 30)
			continue; //valency of 30 == something bad, silently ignore
		uset uua;
		uua.fromPuzzle(puz.chars);
		for(int s = 0; s < nSol; s++) { //find the unsplitable UA if weakly minimal
			grid g;
			memcpy(g.digits, &buf[s * 81], 81);
			digit2bitmap(g.digits, g.gridBM);
			g.findUAbyPuzzle(puz.chars);
			const uset &ua = *(g.usetsBySize.rbegin());
			if(g.usetsBySize.size() != 1 || ua.nbits != 81 - puzSize)
				continue; //non-minimal ua or not complementary
			if(opt.uaOpt->subcanon) {
				//only dump minimal UA
				ch81 p;
				g.ua2InvariantPuzzle(ua, buf);
				subcanon(buf, p.chars);
				p.toString(buf);
				printf("%81.81s\t%d\n", buf, ua.nbits);
			}
			else {
				int perm = Digit2Bitmap[nClues] - 1; //start with rightmost nClues - 1 bits set
				while(0 == (perm & Digit2Bitmap[puzSize + 1])) {
					memset(buf, 0, 81); //start from empty puzzle
					for(int i = 0; i < puzSize; i++) {
						if(0 == (perm & Digit2Bitmap[i + 1]))
							continue;
						buf[uua.positions[i]] = g.digits[uua.positions[i]];
					}
					//advance to next permutation
					int t = (perm | (perm - 1)) + 1;
					perm = t | ((((t & -t) / (perm & -perm)) >> 1) - 1);
					for(int uaClue = 0; uaClue < ua.nbits; uaClue++) {
						buf[ua.positions[uaClue]] = g.digits[ua.positions[uaClue]]; //set the external given
						//check the puzzle
						if(1 == solve(g.gridBM, buf, 2)) {
							//we got a valid puzzle
							ch81 p;
							p.toString(buf, p.chars);
							printf("%81.81s\n", p.chars);
							fflush(NULL);
						}
						buf[ua.positions[uaClue]] = 0; //clear this given
					} //uaClue
				}
				//break; //process only the first UA
			}
		}
	}
	return 0;
}

int scanPuzzleset(const bool invert) {
	const char* fname = opt.scanOpt->gridFileName;
	char buf[2000];
	if(NULL == fname)
		return 1; //no input file specified
	ifstream file(fname);
	if(! file)
		return -1;
	while(file.getline(buf, sizeof(buf))) {
		ch81 puz;
		//int puzSize = puz.fromString(buf + 82); //for rookeries <tab> exemplar
		int puzSize = puz.fromString(buf);
		grid g;
		if(0 == solve(puz.chars, 1, g.digits)) { //check for any valid completion
			fprintf(stderr, "No completion for %81.81s\n", buf);
			continue;
		}
		digit2bitmap(g.digits, g.gridBM);
		//prepare a pseudo-puzzle for UA search
		ch81 completion;
		if(invert) {
			for(int i = 0; i < 81; i++) {
				completion.chars[i] = puz.chars[i] ? 0 : g.digits[i];
			}
			puzSize = 81 - puzSize;
		}
		else {
			completion = puz; //structure copy
		}
		unsigned long long nSol;
		if(puzSize <= 36) {
			//g.findUA4cells();
			//g.findUA6cells();
			//g.findUAbyPuzzleBox(completion.chars);
			//opt.uaOpt->nAttempts = 3000;
			//opt.uaOpt->nCells = 54;
			opt.scanOpt->progressSeconds = 100000;
			g.findUArandom(completion.chars);
			nSol = 0;
		}
		else {
			nSol = g.findUAbyPuzzle(completion.chars);
		}
		//now g has valid UA list
		clueIterator ci(g);
		ci.iterateFixedCells(completion.chars);
		//puzzles found are in ci.minimizedPuzzles
		ch81 pp;
		puz.toString(pp.chars);
		//printf("%81.81s\t%llu\t%d\n", pp.chars, nSol, (int)g.usetsBySize.size());
		//printf("%81.81s\t%llu\n", pp.chars, nSol);
		//printf("%81.81s\t%d\t%d\n", pp.chars, ci.nClues - puzSize, ci.minimizedPuzzles.size());
		printf("%81.81s\t%d\n", pp.chars, ci.nClues - puzSize);
		for(puzTextSet::const_iterator p = ci.minimizedPuzzles.begin(); p != ci.minimizedPuzzles.end(); p++) {
			printf("%81.81s\n", p->chars);
		}
		fflush(NULL);
	}
	return 0;
}
extern int scanGridsFromFile() {
	if(opt.scanOpt->scanpuzzleset) { // --scan { --subgridlist | --isubgridlist } in.txt
		return scanPuzzleset(opt.scanOpt->invertpuzzleset);
	}
	if(opt.scanOpt->scanunav) { // --scan --scanunav --numclues 17
		return scanUnavForPuzzles();
	}
	if(opt.scanOpt->bandcompletions) {
		return allBandCompletions();
	}
	if(opt.scanOpt->scanfixedbandstack) {
		return scanFixedBandStack();
	}
	if(!opt.scanOpt->batchMode) {
		return scanGridFromFile();
	}
	const char* fname = opt.scanOpt->gridFileName;
	const int nClues = opt.scanOpt->nClues;
	char buf[3000];
	FILE *file;
	if(fname) {
		strcpy(buf, fname);
		strcat(buf, ".txt");
		file = fopen(buf, "rt");
	}
	else {
		file = stdin;
	}
	while(fgets(buf, sizeof(buf), file)) {
		//printf("grid: %s\n", buf);
		grid g;
		g.fromString(buf);
		g.findInitialUA();
		g.fname = fname;
		clueIterator ci(g);
		switch(opt.scanOpt->fixMode) {
			case scanOptions::fix1box:
				ci.iterateBoxes(nClues);
				break;
			case scanOptions::fixband:
				ci.iterateBands(nClues);
				break;
			case scanOptions::fix2digits:
				ci.iterate2digits(nClues);
				break;
			case scanOptions::fix2boxes:
				ci.iterate2Boxes(nClues);
				break;
			case scanOptions::fix1digit:
				ci.iterateDigits(nClues);
				break;
			case scanOptions::fix3digits:
				ci.iterate3digits(nClues);
				break;
			case scanOptions::fix4digits:
				ci.iterate4digits(nClues);
				break;
			case scanOptions::fixauto:
				ci.iterateWhole(nClues);
				break;
			case scanOptions::exhaustive:
			default:
				ci.iterateWhole(nClues);
				break;
		}
	}
	if(fname) {
		fclose(file);
	}
	return 0;
}

