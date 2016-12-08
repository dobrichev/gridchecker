#define _CRT_SECURE_NO_DEPRECATE

#include <emmintrin.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <set>
#include <iostream>
#include <algorithm>

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

struct reusableUA {
	sizedUsetList clique[21]; //0..15
	int mcn; //6797" for 120K calls in 14x17, 2136" for mcn-1 < 3
	int populate(int stopAt) { //expects previous level 0 is already done
		for(mcn = 1; /*mcn < 3 &&*/ mcn < stopAt && clique[mcn - 1].size(); mcn++) {
			if(populateLevel(mcn)) {
				return 1; //too high mcn
			}
		}
		return 0;
	}
	int populateLevel(int level) { //expects previous levels are already done
		sizedUsetList &singles = clique[0];
		sizedUsetList &t = clique[level];
		t.clear();
		if(level == 1) { //special case
			for(sizedUsetList::const_iterator u0 = singles.begin(); u0 != singles.end(); u0++) {
				for(sizedUsetList::const_iterator u1 = u0; ++u1 != singles.end();) {
					sizedUset tt(*u1);
					if(tt.join(*u0)) {
						t.insert(tt);
					}
				}
			}
		}
		else {
			sizedUsetList &prev = clique[level - 1];
			for(sizedUsetList::const_iterator u0 = singles.begin(); u0 != singles.end(); u0++) {
				for(sizedUsetList::const_iterator u1 = prev.begin(); u1 != prev.end(); u1++) {
					sizedUset tt(*u1);
					if(tt.join(*u0)) {
						t.insert(tt);
					}
				}
			}
		}
		return 0;
	}
};

typedef bit_masks<512> ua1_type;
typedef bit_masks<4096> ua2_type;
typedef bit_masks<12288> ua3_type;
typedef bit_masks<12288> ua4_type;
typedef bit_masks<2048> ua5_type;

struct fastState {
	bm128 deadClues;
	bm128 setClues;
	ua1_type setMask;
	ua2_type setMask2;
	ua3_type setMask3;
	ua4_type setMask4;
	ua5_type setMask5;
	int nPositions;
	int positions[81];
	sizedUsetVector sizedUsetV;
};

class fastClueIterator {
private:
	void fastIterateLevel0(int currentUaIndex);
	void fastIterateLevel1(int currentUaIndex);
	void fastIterateLevel(int currentUaIndex = 0);
	void switch2bm();
	void checkPuzzle(bm128 &dc, int startPos = 0);
	fastClueIterator();
	//void bm128ToIndex(const bm128 *sets, int nsets, BitMask768 &setMask, BitMask768 *hittingMasks);
	//void iterateLevel();
public:
	ua1_type hittingMasks[81];
	ua2_type hittingMasks2[81];
	ua3_type hittingMasks3[81];
	ua4_type hittingMasks4[81];
	ua5_type hittingMasks5[81];
	int nClues;
	grid &g;
	unsigned int nPuzzles;
	unsigned int nChecked;
	int clueNumber; //nClues - 1 .. 0
	fastState state[81];
	char clues[81];
	reusableUA cliques;
	sizedUsetVector *ua1;

	fastClueIterator(grid &g);
	void iterate();
};

void fastClueIterator::fastIterateLevel0(int currentUaIndex) {
	fastState &oldState = state[1];
	fastState &newState = state[0];
	newState.deadClues = oldState.deadClues;
	clueNumber = 0;
	if(currentUaIndex == INT_MAX) {
		//todo: iterate over all non-dead clues
		goto done;
	}
	else {
		uset u((*ua1)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		if(u.isZero()) {
			//the UA is entirely within the dead clues
			//printf("the UA is entirely within the dead clues\n");
			goto done;
		}
		u.positionsByBitmap();
		newState.nPositions = u.nbits;
		for(int i = 0; i < u.nbits; i++) {
			newState.positions[i] = u.positions[i];
		}
		for(int i = 0; i < newState.nPositions; i++) {
			int cluePosition = newState.positions[i];
			if(state[1].setMask.isHittingAll(hittingMasks[cluePosition])) {
				//all UA are hit, check the puzzle for uniqueness
				//solve
				clues[cluePosition] = g.digits[cluePosition];
	//			nChecked++; //b6b ch=2738236, 136" no solver / 172" solver, 1 found
	//			if(solve(clues, 2) == 1) {
	//				nPuzzles++;
	//			}
				checkPuzzle(newState.deadClues);
				clues[cluePosition] = 0;
			}
	nextClue:;
			newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 1;
}
void fastClueIterator::fastIterateLevel1(int currentUaIndex) {
	fastState &oldState = state[2];
	fastState &newState = state[1];
	newState.deadClues = oldState.deadClues;
	clueNumber = 1;
	if(currentUaIndex == INT_MAX) {
		//todo: iterate over all non-dead clues
		goto done;
	}
	else {
		uset u((*ua1)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		if(u.isZero()) {
			//the UA is entirely within the dead clues
			//printf("the UA is entirely within the dead clues\n");
			goto done;
		}

		u.positionsByBitmap();
		newState.nPositions = u.nbits;
		for(int i = 0; i < u.nbits; i++) {
			newState.positions[i] = u.positions[i];
		}
		for(int i = 0; i < newState.nPositions; i++) {
			int cluePosition = newState.positions[i];
			if(state[2].setMask2.isHittingAll(hittingMasks2[cluePosition])) {
				//all ua2 are hit, check ua1
				//int nextUaIndex = state[1].setMask.hit(state[2].setMask, hittingMasks[cluePosition]);
				state[1].setMask.hitOnly(state[2].setMask, hittingMasks[cluePosition]);
				int nextUaIndex = state[1].setMask.firstUnhit();
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel0(nextUaIndex);
				clues[cluePosition] = 0;
			}
			newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 2;
}

void fastClueIterator::fastIterateLevel(int currentUaIndex) {
	fastState &oldState = state[clueNumber];
	clueNumber--;
	fastState &newState = state[clueNumber];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((*ua1)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	if(u.isZero()) {
		goto backtrack;
	}
	u.positionsByBitmap();
//	if(u.nbits == 0) {
//		//printf("Empty UA\n");
//		goto backtrack;
//	}
	newState.nPositions = u.nbits;
	for(int i = 0; i < u.nbits; i++) {
		newState.positions[i] = u.positions[i];
	}
	for(int i = 0; i < newState.nPositions; i++) {
		int cluePosition = newState.positions[i];
		//int nextUaIndex = state[clueNumber].setMask.hit(state[clueNumber + 1].setMask, hittingMasks[cluePosition]);
		state[clueNumber].setMask.hitOnly(state[clueNumber + 1].setMask, hittingMasks[cluePosition]);
		int nextUaIndex = state[clueNumber].setMask.firstUnhit();
		clues[cluePosition] = g.digits[cluePosition];
		if(clueNumber) {
			if(nextUaIndex != INT_MAX) {
				if(clueNumber >= 5) {
					//update composite UA requiring 5 clues
					state[clueNumber].setMask5.hitOnly(state[clueNumber + 1].setMask5, hittingMasks5[cluePosition]);
				}
				if(clueNumber == 4) {
					//int unhit;
					if(!state[clueNumber + 1].setMask5.isHittingAll(hittingMasks5[cluePosition])) {
						//un-hit composite UA found. Continue with next.
//						int unhit = state[clueNumber].setMask5.hit(state[clueNumber + 1].setMask5, hittingMasks5[cluePosition]);
//						printf("Un-hit UA5\n");
//						for(int n = 0; n < 81; n++) printf("%d", clues[n]);
//						printf("\t puzzle\n");
//						for(int n = 0; n < 81; n++) printf("%d", hittingMasks5[n].aBits[unhit / 128].isBitSet(unhit % 128) ? 1 : 0);
//						printf("\t UA5\n");
						goto nextClue;
					}
				}
				if(clueNumber >= 4) {
					//update composite UA requiring 4 clues
					state[clueNumber].setMask4.hitOnly(state[clueNumber + 1].setMask4, hittingMasks4[cluePosition]);
				}
				if(clueNumber == 3) {
					if(!state[clueNumber + 1].setMask4.isHittingAll(hittingMasks4[cluePosition])) {
						goto nextClue;
					}
				}
				if(clueNumber >= 3) {
					//update composite UA requiring 3 clues
					state[clueNumber].setMask3.hitOnly(state[clueNumber + 1].setMask3, hittingMasks3[cluePosition]);
				}
				if(clueNumber == 2) {
					if(!state[clueNumber + 1].setMask3.isHittingAll(hittingMasks3[cluePosition])) {
						goto nextClue;
					}
					state[clueNumber].setMask2.hitOnly(state[clueNumber + 1].setMask2, hittingMasks2[cluePosition]);
					fastIterateLevel1(nextUaIndex);
				}
				else {
					//update composite UA requiring 2 clues
					state[clueNumber].setMask2.hitOnly(state[clueNumber + 1].setMask2, hittingMasks2[cluePosition]);
					fastIterateLevel(nextUaIndex); //call recursively
				}
			}
			else {
				//todo: expand all possible puzzles, no more help from UA
				if(clueNumber > 1) printf("all UA hit at clue %d\n", clueNumber);
			}
		} //clueNumber <> 0
nextClue:;
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
backtrack:;
	clueNumber++;
}

//b6b.txt 816"/364/161 ch=2710054 (758/ch=2565209; 135/2565187)
//b6a.txt 421/730/568  ch=723261/502563 (238/ch=297999)
//rnd1.txt 267/250/293/245/252/228   ch=137035/ch=136828/124892 TODO: ua2 cuts wrongly!!!!
void fastClueIterator::switch2bm() {
	sizedUsetVector &newUA = *ua1;

	//compose ordinary UA
	ua1_type::bm128ToIndex(&newUA[0], newUA.size(), state[clueNumber].setMask, hittingMasks);

	bm128 tmp[40000];
	int i;
	//compose UA2
	i = 0;
	for(sizedUsetList::const_iterator c = cliques.clique[1].begin(); i < state[clueNumber].setMask2.maxSize && c != cliques.clique[1].end(); c++, i++) {
		tmp[i] = *c;
	}
	ua2_type::bm128ToIndex(tmp, i, state[clueNumber].setMask2, hittingMasks2);

	//compose UA3
	i = 0;
	for(sizedUsetList::const_iterator c = cliques.clique[2].begin(); i < state[clueNumber].setMask3.maxSize && c != cliques.clique[2].end(); c++, i++) {
		tmp[i] = *c;
	}
	ua3_type::bm128ToIndex(tmp, i, state[clueNumber].setMask3, hittingMasks3);

	//compose UA4
	i = 0;
	for(sizedUsetList::const_iterator c = cliques.clique[3].begin(); i < state[clueNumber].setMask4.maxSize && c != cliques.clique[3].end(); c++, i++) {
		tmp[i] = *c;
	}
	ua4_type::bm128ToIndex(tmp, i, state[clueNumber].setMask4, hittingMasks4);

	//compose UA5
	i = 0;
	for(sizedUsetList::const_iterator c = cliques.clique[4].begin(); i < state[clueNumber].setMask5.maxSize && c != cliques.clique[4].end(); c++, i++) {
		tmp[i] = *c;
	}
	ua5_type::bm128ToIndex(tmp, i, state[clueNumber].setMask5, hittingMasks5);
	//fastIterateLevel();
}

fastClueIterator::fastClueIterator(grid &g) : g(g), clueNumber(0), ua1(NULL) {
	nPuzzles = 0;
	nChecked = 0;
	nClues = opt.scanOpt->nClues;
}

void fastClueIterator::checkPuzzle(bm128 &dc, int startPos) {
	if(clueNumber == 0) {
		if(solve(clues, 2) == 1) {
			ch81 puz;
			puz.toString(clues, puz.chars);
			printf("%81.81s\n", puz.chars);
			nPuzzles++;
		}
		nChecked++;
	}
	else {
		clueNumber--;
		for(int i = startPos; i < 81; i++) {
			if(clues[i]) {	//given
				continue;
			}
			if(dc.isBitSet(i)) { //dead clue
				continue;
			}
			clues[i] = g.digits[i];
			checkPuzzle(dc, i + 1);
			clues[i] = 0;
		}
		clueNumber++;
	}
	//debug
	if(nChecked % 100000 == 0) {
		printf("checked %d, found %d\n", nChecked, nPuzzles);
	}
}

unsigned long long d0, d1, d2, d3, d4; //debug


void fastClueIterator::iterate() {
	//find all UA sets of size 4..12
	g.findUA12();
	usetListBySize &us = g.usetsBySize;
	printf("\t%d\n", (int)us.size());
	//debug: add all 4-digit UA
	//g.findUA4digits();
	//printf("\t%d\n", (int)us.size());

	//init the top of the stack
	clueNumber = nClues; //stack pointer to the "empty" puzzle
	fastState &s = state[nClues];
	for(int i = 0; i < 81; i++) {
		clues[i] = 0;
	}

	s.deadClues.clear();
	s.setClues.clear();
	const int numUaToSkip = 40; //Gary McGuire skips the shortest 40 UA
	int curUA = 0;
	for(usetListBySize::const_iterator p = us.begin(); p != us.end(); p++, curUA++) {
		sizedUset su;
		su.bitmap128 = p->bitmap128; //don't calculate the size
		su.setSize(p->nbits);
		s.sizedUsetV.push_back(su);
		if(curUA < numUaToSkip) continue;
		//if(su.getSize() >= 6)
			cliques.clique[0].insert(su);
	}
	cliques.populate(5); //this crashes for MostCanonical grid on 32-bit platform

	//always start from the first UA which is one of the shortest
	uset top;
	top.bitmap128 = s.sizedUsetV[0].bitmap128;
	top &= maskLSB[81];
	top.positionsByBitmap();
	s.nPositions = top.nbits;
	for(int i = 0; i < s.nPositions; i++) {
		s.positions[i] = top.positions[i];
	}

	//some info for debugging/optimization
	int population[81];
	for(int i = 0; i < cliques.mcn; i++) {
		for(int i = 0; i < 81; i++) population[i] = 0;
		int min = 100, sum = 0, max = 0;
		for(sizedUsetList::const_iterator p = cliques.clique[i].begin(); p != cliques.clique[i].end(); p++) {
			int size = p->getSize();
			sum += size;
			if(min > size) min = size;
			if(max < size) max = size;
			for(int j = 0; j < 81; j++) if(p->isBitSet(j)) population[j]++;
		}
		printf("cl(%d)\tcount=%d\tmin=%d\tavg=%2.2f\tmax=%d\n", i + 1, (int)cliques.clique[i].size(), min, sum/(double)cliques.clique[i].size(), max);
//		sum = 0;
//		for(int j = 0; j < 81; j++) sum += population[j];
//		for(int j = 0; j < 81; j++) population[j] /= (sum / 81 / 10);
//		for(int j = 0; j < 81; j++) printf("%2.2d ", population[j]);
//		printf("\n");
	}
	ua1 = &state[nClues].sizedUsetV;
	switch2bm();
	fastIterateLevel();
	//iterateLevel();
	printf("puz=%d\tch=%d\n", nPuzzles, nChecked);
	printf("%llu\t%llu\t%llu\t%llu\t%llu\n", d0, d1, d2, d3, d4); //14x17 grid = 5,889,056 33,028,056 114,316,605 124,680,124 43,565,492 =>2061.494 seconds.
}


extern int fastScan() {
	//const char* fname = opt.scanOpt->gridFileName;
	char buf[3000];
	while(fgets(buf, sizeof(buf), stdin)) {
		printf("%81.81s", buf);
		grid g;
		g.fromString(buf);
		//g.fname = fname;
		fastClueIterator ci(g);
		ci.iterate();
		return 0; //bug in eclipse???
	}
	return 0;
}
