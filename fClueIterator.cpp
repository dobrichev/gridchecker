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
#include "t_128.h"
#include "uset.h"
#include "grid.h"
#include "BitMask768.h"
#include "ch81.h"
#include "options.h"

using namespace std;

//typedef bit_masks<384> ua1_type; //McGuire
//typedef bit_masks<8192> ua2_type;
//typedef bit_masks<16384> ua3_type;
//typedef bit_masks<32768> ua4_type;
//typedef bit_masks<16384> ua5_type;
//
//typedef bit_masks<128> fua1_type;
//typedef bit_masks<384> fua2_type;
//typedef bit_masks<1280> fua3_type;
//typedef bit_masks<1536> fua4_type;
//typedef bit_masks<1536> fua5_type;

typedef bit_masks<768> ua1_type;
typedef bit_masks<10240> ua2_type;
typedef bit_masks<32768> ua3_type;
typedef bit_masks<32768> ua4_type;
typedef bit_masks<32768> ua5_type;
typedef bit_masks<32768> ua6_type;

typedef bit_masks<256> fua1_type;
typedef bit_masks<768> fua2_type;
typedef bit_masks<1536> fua3_type;
typedef bit_masks<1792> fua4_type;
typedef bit_masks<1792> fua5_type;
typedef bit_masks<1792> fua6_type;

//consolidated
//128
//384
//1280
//1536
//1536

struct fastState {
	bm128 deadClues;
	bm128 setClues;
	ua1_type setMask;
	ua2_type setMask2;
	ua3_type setMask3;
	ua4_type setMask4;
	ua5_type setMask5;
	ua6_type setMask6;
	fua1_type fSetMask;
	fua2_type fSetMask2;
	fua3_type fSetMask3;
	fua4_type fSetMask4;
	fua5_type fSetMask5;
	fua5_type fSetMask6;
	int nPositions;
	int positions[81];
};

class fastClueIterator {
private:
	void fastIterateLevel0(int currentUaIndex);
	void fastIterateLevel1(int currentUaIndex);
	void fastIterateLevel2(int currentUaIndex);
	void fastIterateLevel3(int currentUaIndex);
	void fastIterateLevel4(int currentUaIndex);
	void fastIterateLevel5(int currentUaIndex);
	void fastIterateLevel9to6(int currentUaIndex);
	void fastIterateLevel10(int currentUaIndex);
	void fastIterateLevel11(int currentUaIndex);
	void fastIterateLevel12(int currentUaIndex);
	void fastIterateLevel13(int currentUaIndex);
	void fastIterateLevel(int currentUaIndex = 0);
	void switch2bm();
	void checkPuzzle(bm128 &dc, int startPos = 0);
	fastClueIterator();
	//void bm128ToIndex(const bm128 *sets, int nsets, BitMask768 &setMask, BitMask768 *hittingMasks);
	//void iterateLevel();
//	static int const starter2 = 44; // Gary McGure http://www.math.ie/checker.html
//	static int const starter3 = 30;
//	static int const starter4 = 27;
//	static int const starter5 = 21;
	static int const starter2 = 44;
	static int const starter3 = 30;
	static int const starter4 = 27;
	static int const starter5 = 21;
	static int const starter6 = 18;
public:
	ua1_type hittingMasks[81];
	ua2_type hittingMasks2[81];
	ua3_type hittingMasks3[81];
	ua4_type hittingMasks4[81];
	ua5_type hittingMasks5[81];
	ua5_type hittingMasks6[81];
	fua1_type fHittingMasks[81];  //7+ (10)
	fua2_type fHittingMasks2[81]; //6+ (11)
	fua3_type fHittingMasks3[81]; //6+ (11)
	fua4_type fHittingMasks4[81]; //5+ (12)
	fua5_type fHittingMasks5[81]; //4+ (13)
	fua5_type fHittingMasks6[81]; //4+ (13)?
	sizedUset ua[ua1_type::maxSize];
	sizedUset ua2[ua2_type::maxSize];
	sizedUset ua3[ua3_type::maxSize];
	sizedUset ua4[ua4_type::maxSize];
	sizedUset ua5[ua5_type::maxSize];
	sizedUset ua6[ua6_type::maxSize];
	sizedUset fUa[fua1_type::maxSize];
	sizedUset fUa2[fua2_type::maxSize];
	sizedUset fUa3[fua3_type::maxSize];
	sizedUset fUa4[fua4_type::maxSize];
	sizedUset fUa5[fua5_type::maxSize];
	sizedUset fUa6[fua6_type::maxSize];
	int uaActualSize;
	int ua2ActualSize;
	int ua3ActualSize;
	int ua4ActualSize;
	int ua5ActualSize;
	int ua6ActualSize;
	int fuaActualSize;
	int fua2ActualSize;
	int fua3ActualSize;
	int fua4ActualSize;
	int fua5ActualSize;
	int fua6ActualSize;

	int nClues;
	grid &g;
	unsigned int nPuzzles;
	unsigned int nChecked;
	int clueNumber; //nClues - 1 .. 0
	fastState state[81];
	char clues[81];

	fastClueIterator(grid &g);
	void iterate();
};

void fastClueIterator::fastIterateLevel0(int currentUaIndex) {
	fastState &oldState = state[1];
	fastState &newState = state[0];
	newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(newState.deadClues);
		goto done;
	}
	else {
		uset u((fUa)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
//		if(u.isZero()) {
//			//the UA is entirely within the dead clues
//			//printf("the UA is entirely within the dead clues\n");
//			goto done;
//		}
		u.positionsByBitmap();
		for(int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(oldState.fSetMask.isHittingAll(fHittingMasks[cluePosition])) {
				//all UA are hit, check the puzzle for uniqueness
				//solve
				clueNumber = 0;
				clues[cluePosition] = g.digits[cluePosition];
				checkPuzzle(newState.deadClues);
				clues[cluePosition] = 0;
			}
			//newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 1;
}
void fastClueIterator::fastIterateLevel1(int currentUaIndex) {
	fastState &oldState = state[2];
	fastState &newState = state[1];
	newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(newState.deadClues);
		goto done;
	}
	else {
		uset u((fUa)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		clueNumber = 1;
		u.positionsByBitmap();
		for(int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(state[2].fSetMask2.isHittingAll(fHittingMasks2[cluePosition])) {
				//all ua2 are hit, check ua1
				newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
				int nextUaIndex = newState.fSetMask.firstUnhit();
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
void fastClueIterator::fastIterateLevel2(int currentUaIndex) {
	fastState &oldState = state[3];
	fastState &newState = state[2];
	newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(newState.deadClues);
		goto done;
	}
	else {
		uset u((fUa)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		clueNumber = 2;
		u.positionsByBitmap();
		for(int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(state[3].fSetMask3.isHittingAll(fHittingMasks3[cluePosition])) {
				//all ua3 are hit, now hit ua2 and ua
				state[2].fSetMask2.hitOnly(state[3].fSetMask2, fHittingMasks2[cluePosition]);
				newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
				int nextUaIndex = newState.fSetMask.firstUnhit();
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel1(nextUaIndex);
				clues[cluePosition] = 0;
			}
			newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 3;
}
void fastClueIterator::fastIterateLevel3(int currentUaIndex) {
	fastState &oldState = state[4];
	fastState &newState = state[3];
	newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(newState.deadClues);
		goto done;
	}
	else {
		uset u((fUa)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		clueNumber = 3;
		u.positionsByBitmap();
		for(int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(state[4].fSetMask4.isHittingAll(fHittingMasks4[cluePosition])) {
				//all ua4 are hit, now hit ua3, ua2 and ua
				state[3].fSetMask3.hitOnly(state[4].fSetMask3, fHittingMasks3[cluePosition]);
				state[3].fSetMask2.hitOnly(state[4].fSetMask2, fHittingMasks2[cluePosition]);
				newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
				int nextUaIndex = newState.fSetMask.firstUnhit();
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel2(nextUaIndex);
				clues[cluePosition] = 0;
			}
			newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 4;
}
void fastClueIterator::fastIterateLevel4(int currentUaIndex) {
	fastState &oldState = state[5];
	fastState &newState = state[4];
	newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(newState.deadClues);
		goto done;
	}
	else {
		uset u((fUa)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		clueNumber = 4;
		u.positionsByBitmap();
		for(int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(state[5].fSetMask5.isHittingAll(fHittingMasks5[cluePosition])) {
				//all ua5 are hit, now hit ua4, ua3, ua2 and ua
				state[4].fSetMask4.hitOnly(state[5].fSetMask4, fHittingMasks4[cluePosition]);
				state[4].fSetMask3.hitOnly(state[5].fSetMask3, fHittingMasks3[cluePosition]);
				state[4].fSetMask2.hitOnly(state[5].fSetMask2, fHittingMasks2[cluePosition]);
				newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
				int nextUaIndex = newState.fSetMask.firstUnhit();
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel3(nextUaIndex);
				clues[cluePosition] = 0;
			}
			newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 5;
}
void fastClueIterator::fastIterateLevel5(int currentUaIndex) {
	fastState &oldState = state[6];
	fastState &newState = state[5];
	newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(newState.deadClues);
		goto done;
	}
	else {
		uset u((fUa)[currentUaIndex].bitmap128);
		u &= maskLSB[81];
		u.clearBits(oldState.deadClues);
		clueNumber = 5;
		u.positionsByBitmap();
		for(int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(oldState.fSetMask6.isHittingAll(fHittingMasks6[cluePosition])) {
				//all ua6 are hit, now hit ua5, ua4, ua3, ua2 and ua
				newState.fSetMask5.hitOnly(oldState.fSetMask5, fHittingMasks5[cluePosition]);
				newState.fSetMask4.hitOnly(oldState.fSetMask4, fHittingMasks4[cluePosition]);
				newState.fSetMask3.hitOnly(oldState.fSetMask3, fHittingMasks3[cluePosition]);
				newState.fSetMask2.hitOnly(oldState.fSetMask2, fHittingMasks2[cluePosition]);
				newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
				int nextUaIndex = newState.fSetMask.firstUnhit();
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel4(nextUaIndex);
				clues[cluePosition] = 0;
			}
			newState.deadClues.setBit(cluePosition);
		}
	}
done:
	clueNumber = 6;
}
void fastClueIterator::fastIterateLevel9to6(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	fastState &oldState = state[clueNumber];
	clueNumber--;
	fastState &newState = state[clueNumber];
	newState.deadClues = oldState.deadClues;
	uset u((fUa)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		newState.fSetMask5.hitOnly(oldState.fSetMask5, fHittingMasks5[cluePosition]);
		newState.fSetMask4.hitOnly(oldState.fSetMask4, fHittingMasks4[cluePosition]);
		newState.fSetMask3.hitOnly(oldState.fSetMask3, fHittingMasks3[cluePosition]);
		newState.fSetMask2.hitOnly(oldState.fSetMask2, fHittingMasks2[cluePosition]);
		newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
		int nextUaIndex = newState.fSetMask.firstUnhit();
		clues[cluePosition] = g.digits[cluePosition];
		if(clueNumber == 6) {
			fastIterateLevel5(nextUaIndex);
		}
		else {
			fastIterateLevel9to6(nextUaIndex); //call recursively
		}
		clues[cluePosition] = 0;
		newState.deadClues.setBit(cluePosition);
	}
	clueNumber++;
}

void fastClueIterator::fastIterateLevel10(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	fastState &oldState = state[11];
	clueNumber = 10;
	fastState &newState = state[10];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((ua)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		clues[cluePosition] = g.digits[cluePosition];
		//update composite UA requiring 1, 2, 3, 4, 5 and 6 clues
		newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		newState.fSetMask5.hitOnly(oldState.fSetMask5, fHittingMasks5[cluePosition]);
		newState.fSetMask4.hitOnly(oldState.fSetMask4, fHittingMasks4[cluePosition]);
		newState.fSetMask3.hitOnly(oldState.fSetMask3, fHittingMasks3[cluePosition]);
		newState.fSetMask2.hitOnly(oldState.fSetMask2, fHittingMasks2[cluePosition]);
		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
		//consolidate ua to fUa
		fuaActualSize = newState.setMask.copyAlive(ua, fUa, fua1_type::maxSize, newState.deadClues);
		//printf("fuaActualSize=%d", fuaActualSize);
		std::sort(fUa, fUa + fuaActualSize, sizedUset::isSmaller);
		//fuaActualSize = (std::unique(fUa, fUa + fuaActualSize) - &fUa[0]);
		//printf("\t%d\n", fuaActualSize);
		fua1_type::bm128ToIndex(fUa, fuaActualSize, newState.fSetMask, fHittingMasks);
		fastIterateLevel9to6(0);
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
	clueNumber = 11;
}
void fastClueIterator::fastIterateLevel11(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	fastState &oldState = state[12];
	clueNumber = 11;
	fastState &newState = state[11];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((ua)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		clues[cluePosition] = g.digits[cluePosition];
		//update composite UA requiring 1, 2, 3, 4, 5 and 6 clues
		newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		newState.fSetMask5.hitOnly(oldState.fSetMask5, fHittingMasks5[cluePosition]);
		newState.fSetMask4.hitOnly(oldState.fSetMask4, fHittingMasks4[cluePosition]);
		newState.setMask3.hitOnly(oldState.setMask3, hittingMasks3[cluePosition]);
		//consolidate ua3 to fUa3
		fua3ActualSize = newState.setMask3.copyAlive(ua3, fUa3, fua3_type::maxSize, newState.deadClues);
		//std::sort(fUa3, fUa3 + fua3ActualSize, sizedUset::isSmaller);
		fua3_type::bm128ToIndex(fUa3, fua3ActualSize, newState.fSetMask3, fHittingMasks3);
		newState.setMask2.hitOnly(oldState.setMask2, hittingMasks2[cluePosition]);
		//consolidate ua2 to fUa2
		fua2ActualSize = newState.setMask2.copyAlive(ua2, fUa2, fua2_type::maxSize, newState.deadClues);
		//std::sort(fUa2, fUa2 + fua2ActualSize, sizedUset::isSmaller);
		fua2_type::bm128ToIndex(fUa2, fua2ActualSize, newState.fSetMask2, fHittingMasks2);
		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
		int nextUaIndex = newState.setMask.firstUnhit();
		fastIterateLevel10(nextUaIndex);
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
	clueNumber = 12;
}
void fastClueIterator::fastIterateLevel12(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	fastState &oldState = state[13];
	clueNumber = 12;
	fastState &newState = state[12];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((ua)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		clues[cluePosition] = g.digits[cluePosition];
		//update composite UA requiring 1, 2, 3, 4, 5 and 6 clues
		newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		newState.fSetMask5.hitOnly(oldState.fSetMask5, fHittingMasks5[cluePosition]);
		newState.setMask4.hitOnly(oldState.setMask4, hittingMasks4[cluePosition]);
		//consolidate ua4 to fUa4
		fua4ActualSize = newState.setMask4.copyAlive(ua4, fUa4, fua4_type::maxSize, newState.deadClues);
		//std::sort(fUa4, fUa4 + fua4ActualSize, sizedUset::isSmaller);
		fua4_type::bm128ToIndex(fUa4, fua4ActualSize, newState.fSetMask4, fHittingMasks4);
		newState.setMask3.hitOnly(oldState.setMask3, hittingMasks3[cluePosition]);
		newState.setMask2.hitOnly(oldState.setMask2, hittingMasks2[cluePosition]);
		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
		int nextUaIndex = newState.setMask.firstUnhit();
		fastIterateLevel11(nextUaIndex); //call recursively
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
	clueNumber= 13;
}
void fastClueIterator::fastIterateLevel13(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	fastState &oldState = state[14];
	clueNumber = 13;
	fastState &newState = state[13];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((ua)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		clues[cluePosition] = g.digits[cluePosition];
		//update composite UA requiring 1, 2, 3, 4, 5 and 6 clues
		newState.setMask6.hitOnly(oldState.setMask6, hittingMasks6[cluePosition]);
		//consolidate ua6 to fUa6
		fua6ActualSize = newState.setMask6.copyAlive(ua6, fUa6, fua6_type::maxSize, newState.deadClues);
		//std::sort(fUa6, fUa6 + fua6ActualSize, sizedUset::isSmaller);
		fua6_type::bm128ToIndex(fUa6, fua6ActualSize, newState.fSetMask6, fHittingMasks6);
		newState.setMask5.hitOnly(oldState.setMask5, hittingMasks5[cluePosition]);
		//consolidate ua5 to fUa5
		fua5ActualSize = newState.setMask5.copyAlive(ua5, fUa5, fua5_type::maxSize, newState.deadClues);
		//std::sort(fUa5, fUa5 + fua5ActualSize, sizedUset::isSmaller);
		fua5_type::bm128ToIndex(fUa5, fua5ActualSize, newState.fSetMask5, fHittingMasks5);
		newState.setMask4.hitOnly(oldState.setMask4, hittingMasks4[cluePosition]);
		newState.setMask3.hitOnly(oldState.setMask3, hittingMasks3[cluePosition]);
		newState.setMask2.hitOnly(oldState.setMask2, hittingMasks2[cluePosition]);
		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
		int nextUaIndex = newState.setMask.firstUnhit();
		fastIterateLevel12(nextUaIndex);
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
	clueNumber = 14;
}
void fastClueIterator::fastIterateLevel(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	fastState &oldState = state[clueNumber];
	clueNumber--;
	fastState &newState = state[clueNumber];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((ua)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		clues[cluePosition] = g.digits[cluePosition];
		//update composite UA requiring 1, 2, 3, 4, 5 and 6 clues
		newState.setMask6.hitOnly(oldState.setMask6, hittingMasks6[cluePosition]);
		newState.setMask5.hitOnly(oldState.setMask5, hittingMasks5[cluePosition]);
		newState.setMask4.hitOnly(oldState.setMask4, hittingMasks4[cluePosition]);
		newState.setMask3.hitOnly(oldState.setMask3, hittingMasks3[cluePosition]);
		newState.setMask2.hitOnly(oldState.setMask2, hittingMasks2[cluePosition]);
		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
		int nextUaIndex = newState.setMask.firstUnhit();
		if(clueNumber == 14) {
			fastIterateLevel13(nextUaIndex);
		}
		else {
			fastIterateLevel(nextUaIndex); //call recursively
		}
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
	clueNumber++;
}

//b6b.txt 58.3"
//b6a.txt 69.2
//rnd1.txt 22.7
void fastClueIterator::switch2bm() {
	//compose ordinary UA
	ua1_type::bm128ToIndex(ua, uaActualSize, state[clueNumber].setMask, hittingMasks);

	//compose UA2
	ua2ActualSize = 0;
	for(int s1 = starter2; s1 < uaActualSize - 1; s1++) {
		for(int s2 = s1 + 1; s2 < uaActualSize; s2++) {
			sizedUset tt(ua[s1]);
			if(tt.join(ua[s2])) {
				ua2[ua2ActualSize] = tt;
				ua2ActualSize++;
				if(ua2ActualSize >= ua2_type::maxSize)
					goto ua2composed;
			}
		}
	}
	ua2composed:;
	std::sort(ua2, ua2 + ua2ActualSize, sizedUset::isSmaller);
	ua2_type::bm128ToIndex(ua2, ua2ActualSize, state[clueNumber].setMask2, hittingMasks2);
	//ua2_type::debug_check_hitting_masks(ua2ActualSize, ua2, hittingMasks2);

	//compose UA3
	ua3ActualSize = 0;
	for(int s1 = starter3; s1 < uaActualSize; s1++) {
		for(int s2 = 0; s2 < ua2ActualSize; s2++) {
			sizedUset tt(ua[s1]);
			if(tt.join(ua2[s2])) {
				ua3[ua3ActualSize] = tt;
				ua3ActualSize++;
				if(ua3ActualSize >= ua3_type::maxSize)
					goto ua3composed;
			}
		}
	}
	ua3composed:;
	std::sort(ua3, ua3 + ua3ActualSize, sizedUset::isSmaller);
	ua3_type::bm128ToIndex(ua3, ua3ActualSize, state[clueNumber].setMask3, hittingMasks3);

	//compose UA4
	ua4ActualSize = 0;
	for(int s1 = starter4; s1 < uaActualSize; s1++) {
		for(int s2 = 0; s2 < ua3ActualSize; s2++) {
			sizedUset tt(ua[s1]);
			if(tt.join(ua3[s2])) {
				ua4[ua4ActualSize] = tt;
				ua4ActualSize++;
				if(ua4ActualSize >= ua4_type::maxSize)
					goto ua4composed;
			}
		}
	}
	ua4composed:;
	std::sort(ua4, ua4 + ua4ActualSize, sizedUset::isSmaller);
	ua4_type::bm128ToIndex(ua4, ua4ActualSize, state[clueNumber].setMask4, hittingMasks4);

	//compose UA5
	ua5ActualSize = 0;

//	//inject an artificial UA5 for band 3
//	sizedUset tt(maskLSB[27]);
//	tt.setSize();
//	ua5[0] = tt;
//	ua5ActualSize = 1;

	for(int s1 = starter5; s1 < uaActualSize; s1++) {
		for(int s2 = 0; s2 < ua4ActualSize; s2++) {
			sizedUset tt(ua[s1]);
			if(tt.join(ua4[s2])) {
				ua5[ua5ActualSize] = tt;
				ua5ActualSize++;
				if(ua5ActualSize >= ua5_type::maxSize)
					goto ua5composed;
			}
		}
	}
	ua5composed:;
	std::sort(ua5, ua5 + ua5ActualSize, sizedUset::isSmaller);
	ua5_type::bm128ToIndex(ua5, ua5ActualSize, state[clueNumber].setMask5, hittingMasks5);

	//compose UA6
	ua6ActualSize = 0;

//	//inject an artificial UA6 for band 3
//	sizedUset tt(maskLSB[27]);
//	tt.setSize();
//	ua6[0] = tt;
//	ua6ActualSize = 1;

	for(int s1 = starter6; s1 < uaActualSize; s1++) {
		for(int s2 = 0; s2 < ua5ActualSize; s2++) {
			sizedUset tt(ua[s1]);
			if(tt.join(ua5[s2])) {
				ua6[ua6ActualSize] = tt;
				ua6ActualSize++;
				if(ua6ActualSize >= ua6_type::maxSize)
					goto ua6composed;
			}
		}
	}
	ua6composed:;
	std::sort(ua6, ua6 + ua6ActualSize, sizedUset::isSmaller);
	ua6_type::bm128ToIndex(ua6, ua6ActualSize, state[clueNumber].setMask6, hittingMasks6);
}

fastClueIterator::fastClueIterator(grid &g) : g(g), clueNumber(0) {
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
		//debug
		if(nChecked % 300000 == 0) {
			printf("checked %d, found %d\n", nChecked, nPuzzles);
		}
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
}

unsigned long long d0, d1, d2, d3, d4; //debug


void fastClueIterator::iterate() {
	//find all UA sets of size 4..12
	g.findUA12();
	usetListBySize &us = g.usetsBySize;
	//printf("\t%d\n", (int)us.size());
	//debug: add all 4-digit UA
	g.findUA4digits();
	//g.findUA4boxes(); //slows down the whole process, even w/o taking into account the UA creation
	//printf("\t%d\n", (int)us.size());

	//init the top of the stack
	clueNumber = nClues; //stack pointer to the "empty" puzzle
	fastState &s = state[nClues];
	for(int i = 0; i < 81; i++) {
		clues[i] = 0;
	}

	s.deadClues.clear();
	s.setClues.clear();
	int curUA = 0;
	uaActualSize = 0;
	for(usetListBySize::const_iterator p = us.begin(); p != us.end() && uaActualSize < ua1_type::maxSize; p++, curUA++) {
		sizedUset su;
		su.bitmap128 = p->bitmap128; //don't calculate the size
		su.setSize(p->nbits);
		ua[uaActualSize++] = su; //store for hitting
	}

//	//always start from the first UA which is one of the shortest
//	uset top;
//	top.bitmap128 = ua[0].bitmap128;
//	top &= maskLSB[81];
//	top.positionsByBitmap();
//	s.nPositions = top.nbits;
//	for(int i = 0; i < s.nPositions; i++) {
//		s.positions[i] = top.positions[i];
//	}

	switch2bm();
	//some info for debugging/optimization
	printf("\t%d clues\n", nClues);
	printf("ua =%d\n", uaActualSize);
	printf("ua2=%d\n", ua2ActualSize);
	printf("ua3=%d\n", ua3ActualSize);
	printf("ua4=%d\n", ua4ActualSize);
	printf("ua5=%d\n", ua5ActualSize);
	printf("ua6=%d\n", ua6ActualSize);

	fastIterateLevel();
	//iterateLevel();
	printf("\tpuz=%d\tch=%d\n", nPuzzles, nChecked);
	//printf("%llu\t%llu\t%llu\t%llu\t%llu\n", d0, d1, d2, d3, d4);
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
		//return 0; //bug in eclipse???
	}
	return 0;
}
