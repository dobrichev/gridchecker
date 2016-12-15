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

unsigned long long d0, d1, d2, d3, d4, d5, s0, s1, s2, s3, s4, s5; //debug

//typedef bit_masks<384> ua1_type; //McGuire
//typedef bit_masks<8192> ua2_type;
//typedef bit_masks<16384> ua3_type;
//typedef bit_masks<32768> ua4_type;
//typedef bit_masks<16384> ua5_type;
//typedef bit_masks<0> ua6_type;
//
//typedef bit_masks<128> fua1_type;
//typedef bit_masks<384> fua2_type;
//typedef bit_masks<1280> fua3_type;
//typedef bit_masks<1536> fua4_type;
//typedef bit_masks<1536> fua5_type;
//typedef bit_masks<0> fua6_type;

//typedef bit_masks<512> ua1_type; //closest to McGuire
//typedef bit_masks<8192> ua2_type;
//typedef bit_masks<16384> ua3_type;
//typedef bit_masks<32768> ua4_type;
//typedef bit_masks<16384> ua5_type;
//typedef bit_masks<8192> ua6_type;
//
//typedef bit_masks<256> fua1_type;
//typedef bit_masks<512> fua2_type;
//typedef bit_masks<1280> fua3_type;
//typedef bit_masks<1536> fua4_type;
//typedef bit_masks<1536> fua5_type;
//typedef bit_masks<1536> fua6_type;

typedef bit_masks<1024> ua1_type; //768
typedef bit_masks<16384> ua2_type; //10240
typedef bit_masks<32768> ua3_type; //32768
typedef bit_masks<65536> ua4_type; //32768
typedef bit_masks<32768> ua5_type; //65536
typedef bit_masks<16384> ua6_type; //40960

typedef bit_masks<256> fua1_type;
typedef bit_masks<768> fua2_type; //768
typedef bit_masks<2048> fua3_type; //1536
typedef bit_masks<3072> fua4_type; //1792
typedef bit_masks<3072> fua5_type;
typedef bit_masks<3072> fua6_type; //8192, zero disables this functionality

//consolidated
//128
//384
//1280
//1536
//1536

struct starters {
	int starter2;
	int starter3;
	int starter4;
	int starter5;
	int starter6;
};
starters stFamily[] = {
	{44,30,27,21,20}, //original McGuire 16s
	{46,33,28,20,19}, //low, test "a"
	{47,38,33,33,30}, //high, test "c"
	{50,40,36,30,30}, //yet higher
	{52,42,38,33,30}, //yet higher
};

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
	fua6_type fSetMask6;
	int positions[81];
};
struct forecastState {
	bm128 deadClues;
	bm128 setClues;
	ua1_type setMask;
	int positions[81];
};

class fastClueIterator {
private:
	void forecastIterateLevel(int currentUaIndex = 0);
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
public:
	ua1_type hittingMasks[81];
	ua2_type hittingMasks2[81];
	ua3_type hittingMasks3[81];
	ua4_type hittingMasks4[81];
	ua5_type hittingMasks5[81];
	ua6_type hittingMasks6[81];
	fua1_type fHittingMasks[81];  //7+ (10)
	fua2_type fHittingMasks2[81]; //6+ (11)
	fua3_type fHittingMasks3[81]; //6+ (11)
	fua4_type fHittingMasks4[81]; //5+ (12)
	fua5_type fHittingMasks5[81]; //4+ (13)
	fua6_type fHittingMasks6[81]; //4+ (13)?
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
	fastState state[25];
	forecastState fState[4];
	char clues[81];
	starters starter;

	fastClueIterator(grid &g);
	void iterate();
};
void fastClueIterator::forecastIterateLevel(int currentUaIndex) {
	if(currentUaIndex == INT_MAX) {
		return;
	}
	forecastState &oldState = fState[clueNumber];
	clueNumber--;
	forecastState &newState = fState[clueNumber];
	newState.deadClues = oldState.deadClues;
	//prepare state for the next clue
	uset u((ua)[currentUaIndex].bitmap128);
	u &= maskLSB[81];
	u.clearBits(oldState.deadClues);
	u.positionsByBitmap();
	for(int i = 0; i < u.nbits; i++) {
		int cluePosition = u.positions[i];
		clues[cluePosition] = g.digits[cluePosition];
		printf("clue[%d] using ua[%3d] cell[%2.d]=%2.d\n", clueNumber, currentUaIndex, cluePosition, clues[cluePosition]);
		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
		int nextUaIndex = newState.setMask.firstUnhit();
		if(clueNumber == 0) {
			//log the setting
			;
		}
		else {
			forecastIterateLevel(nextUaIndex); //call recursively
		}
		newState.deadClues.setBit(cluePosition);
		clues[cluePosition] = 0;
	}
	clueNumber++;
}

void fastClueIterator::fastIterateLevel0(int currentUaIndex) {
	fastState &oldState = state[1];
	//fastState &newState = state[0];
	//newState.deadClues = oldState.deadClues;
	if(currentUaIndex == INT_MAX) {
		//iterate over all non-dead clues
		checkPuzzle(oldState.deadClues);
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
			s0++;
			if(oldState.fSetMask.isHittingAll(fHittingMasks[cluePosition])) {
				d0++;
				//all UA are hit, check the puzzle for uniqueness
				//solve
				clueNumber = 0;
				clues[cluePosition] = g.digits[cluePosition];
				checkPuzzle(oldState.deadClues);
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
			s1++;
			if(state[2].fSetMask2.isHittingAll(fHittingMasks2[cluePosition])) {
				d1++;
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
			s2++;
			if(state[3].fSetMask3.isHittingAll(fHittingMasks3[cluePosition])) {
				d2++;
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
			s3++;
			if(state[4].fSetMask4.isHittingAll(fHittingMasks4[cluePosition])) {
				d3++;
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
			s4++;
			if(state[5].fSetMask5.isHittingAll(fHittingMasks5[cluePosition])) {
				d4++;
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
			s5++;
			if((fua6_type::maxSize == 0) || oldState.fSetMask6.isHittingAll(fHittingMasks6[cluePosition])) {
				d5++;
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
		if(fua6_type::maxSize) {
			newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		}
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
		if(fua6_type::maxSize) {
			newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		}
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
		if(fua6_type::maxSize) {
			newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		}
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
		if(fua6_type::maxSize) {
			newState.fSetMask6.hitOnly(oldState.fSetMask6, fHittingMasks6[cluePosition]);
		}
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
		if(fua6_type::maxSize) {
			newState.setMask6.hitOnly(oldState.setMask6, hittingMasks6[cluePosition]);
			//consolidate ua6 to fUa6
			fua6ActualSize = newState.setMask6.copyAlive(ua6, fUa6, fua6_type::maxSize, newState.deadClues);
			//std::sort(fUa6, fUa6 + fua6ActualSize, sizedUset::isSmaller);
			fua6_type::bm128ToIndex(fUa6, fua6ActualSize, newState.fSetMask6, fHittingMasks6);
		}
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
		if(fua6_type::maxSize) {
			newState.setMask6.hitOnly(oldState.setMask6, hittingMasks6[cluePosition]);
		}
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
	//ua1_type::bm128ToIndex(ua, uaActualSize, state[clueNumber].setMask, hittingMasks);

	//compose UA2
	ua2ActualSize = 0;
	for(int s1 = starter.starter2; s1 < uaActualSize; s1++) {
		for (int s2 = starter.starter2 - 1; s2 < s1; s2++) {
			sizedUset tt(ua[s1]);
			if(tt.join(ua[s2])) {
				for (int s = 0; s < starter.starter2 - 1; s++) {
					if (ua[s].isSubsetOf(tt))
						goto next_s2;
				}
				ua2[ua2ActualSize] = tt;
				ua2ActualSize++;
				if(ua2ActualSize >= ua2_type::maxSize)
					goto ua2composed;
			}
next_s2:;
		}
	}
	ua2composed:;
	//std::sort(ua2, ua2 + ua2ActualSize, sizedUset::isSmaller);
	ua2_type::bm128ToIndex(ua2, ua2ActualSize, state[clueNumber].setMask2, hittingMasks2);
	//ua2_type::debug_check_hitting_masks(ua2ActualSize, ua2, hittingMasks2);

	//compose UA3
	ua3ActualSize = 0;
	for ( int s1 = starter.starter3;  s1 < uaActualSize;  s1++ ) {
		for (int s2 = starter.starter3 - 1; s2 < s1; s2++) {
			sizedUset tt = ua[s1];
			if(tt.join(ua[s2])) {
				for (int s3 = starter.starter3 - 2; s3 < s2; s3++) {
					sizedUset ttt = tt;
					if (ttt.join(ua[s3])) {
						for (int s = 0; s < starter.starter3 - 2; s++) {
							if (ua[s].isSubsetOf(ttt))
								goto next_s3;
						}
						ua3[ua3ActualSize] = ttt;
						ua3ActualSize++;
						if(ua3ActualSize >= ua3_type::maxSize)
							goto ua3composed;
					}
next_s3:;
				}
			}
		}
	}

	ua3composed:;
	ua3_type::bm128ToIndex(ua3, ua3ActualSize, state[clueNumber].setMask3, hittingMasks3);

	//compose UA4
	ua4ActualSize = 0;
	for ( int s1 = starter.starter4;  s1 < uaActualSize;  s1++ ) {
		for (int s2 = starter.starter4 - 1; s2 < s1; s2++) {
			sizedUset tt = ua[s1];
			if(tt.join(ua[s2])) {
				for (int s3 = starter.starter4 - 2; s3 < s2; s3++) {
					sizedUset ttt = tt;
					if (ttt.join(ua[s3])) {
						for (int s4 = starter.starter4 - 3; s4 < s3; s4++) {
							sizedUset tttt = ttt;
							if (tttt.join(ua[s4])) {
								for (int s = 0; s < starter.starter4 - 3; s++) {
									if (ua[s].isSubsetOf(tttt))
										goto next_s4;
								}
								ua4[ua4ActualSize] = tttt;
								ua4ActualSize++;
								if(ua4ActualSize >= ua4_type::maxSize)
									goto ua4composed;
							}
next_s4:;
						}
					}
				}
			}
		}
	}

ua4composed:;
	ua4_type::bm128ToIndex(ua4, ua4ActualSize, state[clueNumber].setMask4, hittingMasks4);

	//compose UA5
	//	//inject an artificial UA5 for band 3
	//	sizedUset tt(maskLSB[27]);
	//	tt.setSize();
	//	ua5[0] = tt;
	//	ua5ActualSize = 1;

	ua5ActualSize = 0;
	for ( int s1 = starter.starter5;  s1 < uaActualSize;  s1++ ) {
		for (int s2 = starter.starter5 - 1; s2 < s1; s2++) {
			sizedUset tt = ua[s1];
			if(tt.join(ua[s2])) {
				for (int s3 = starter.starter5 - 2; s3 < s2; s3++) {
					sizedUset ttt = tt;
					if (ttt.join(ua[s3])) {
						for (int s4 = starter.starter5 - 3; s4 < s3; s4++) {
							sizedUset tttt = ttt;
							if (tttt.join(ua[s4])) {
								for (int s5 = starter.starter5 - 4; s5 < s4; s5++) {
									sizedUset ttttt = tttt;
									if (ttttt.join(ua[s5])) {
										for (int s = 0; s < starter.starter5 - 4; s++) {
											if (ua[s].isSubsetOf(ttttt))
												goto next_s5;
										}
										ua5[ua5ActualSize] = ttttt;
										ua5ActualSize++;
										if(ua5ActualSize >= ua5_type::maxSize)
											goto ua5composed;
									}
next_s5:;
								}
							}
						}
					}
				}
			}
		}
	}

	ua5composed:;
	ua5_type::bm128ToIndex(ua5, ua5ActualSize, state[clueNumber].setMask5, hittingMasks5);

	//compose UA6
	ua6ActualSize = 0;

//	//inject an artificial UA6 for band 3
//	sizedUset tt(maskLSB[27]);
//	tt.setSize();
//	ua6[0] = tt;
//	ua6ActualSize = 1;

	if(fua6_type::maxSize) {
		for ( int s1 = starter.starter6;  s1 < uaActualSize;  s1++ ) {
			for (int s2 = starter.starter6 - 1; s2 < s1; s2++) {
				sizedUset tt = ua[s1];
				if(tt.join(ua[s2])) {
					for (int s3 = starter.starter6 - 2; s3 < s2; s3++) {
						sizedUset ttt = tt;
						if (ttt.join(ua[s3])) {
							for (int s4 = starter.starter6 - 3; s4 < s3; s4++) {
								sizedUset tttt = ttt;
								if (tttt.join(ua[s4])) {
									for (int s5 = starter.starter6 - 4; s5 < s4; s5++) {
										sizedUset ttttt = tttt;
										if (ttttt.join(ua[s5])) {
											for (int s6 = starter.starter6 - 5; s6 < s5; s6++) {
												sizedUset tttttt = ttttt;
												if (tttttt.join(ua[s6])) {
													for (int s = 0; s < starter.starter6 - 5; s++) {
														if (ua[s].isSubsetOf(tttttt))
															goto next_s6;
													}
													ua6[ua6ActualSize] = tttttt;
													ua6ActualSize++;
													if(ua6ActualSize >= ua6_type::maxSize)
														goto ua6composed;
												}
next_s6:;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		ua6composed:;
		ua6_type::bm128ToIndex(ua6, ua6ActualSize, state[clueNumber].setMask6, hittingMasks6);
	}
}

fastClueIterator::fastClueIterator(grid &g) : g(g), clueNumber(0) {
	nPuzzles = 0;
	nChecked = 0;
	nClues = opt.scanOpt->nClues;
	//nClues = 16;
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
//		//debug
//		if(nChecked % 300000 == 0) {
//			printf("checked %d, found %d\n", nChecked, nPuzzles);
//		}
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

void fastClueIterator::iterate() {
	//g.findUA4boxes(); //slows down the whole process, even w/o taking into account the UA creation
	//find all UA sets of size 4..12
	g.findUA12();
	usetListBySize &us = g.usetsBySize;
	//printf("\t%d\n", (int)us.size());
	//debug: add all 4-digit UA
	g.findUA4digits();
	//printf("\t%d\n", (int)us.size());

	//debug
	g.usetsBySize.setDistributions();
	printf("\n");
	for(int i = 4; i < 20; i++) {
		printf("%d=%d\t", i, g.usetsBySize.distributionBySize[i]);
	}
	printf("\n");

	//init the top of the stack
	int curUA = 0;
	uaActualSize = 0;
	for(usetListBySize::const_iterator p = us.begin(); p != us.end() && uaActualSize < ua1_type::maxSize; p++, curUA++) {
		sizedUset su;
		su.bitmap128 = p->bitmap128; //don't calculate the size
		su.setSize(p->nbits);
		ua[uaActualSize++] = su; //store for hitting
	}
	//compose ordinary UA
	ua1_type::bm128ToIndex(ua, uaActualSize, state[nClues].setMask, hittingMasks);

	clueNumber = 3;
	fState[clueNumber].deadClues.clear();
	fState[clueNumber].setClues.clear();
	for(int i = 0; i < 81; i++) {
		clues[i] = 0;
	}
	forecastIterateLevel();
	return;

	clueNumber = nClues; //stack pointer to the "empty" puzzle
	state[clueNumber].deadClues.clear();
	state[clueNumber].setClues.clear();
	for(int i = 0; i < 81; i++) {
		clues[i] = 0;
	}


	d0=d1=d2=d3=d4=d5=s0=s1=s2=s3=s4=s5=0;

//	//always start from the first UA which is one of the shortest
//	uset top;
//	top.bitmap128 = ua[0].bitmap128;
//	top &= maskLSB[81];
//	top.positionsByBitmap();
//	s.nPositions = top.nbits;
//	for(int i = 0; i < s.nPositions; i++) {
//		s.positions[i] = top.positions[i];
//	}
	starter = stFamily[1];

	switch2bm();
	//some info for debugging/optimization
	//printf("\t%d clues\n", nClues);
	printf("ua =%d\n", uaActualSize);
	printf("ua2=%d\n", ua2ActualSize);
	printf("ua3=%d\n", ua3ActualSize);
	printf("ua4=%d\n", ua4ActualSize);
	printf("ua5=%d\n", ua5ActualSize);
	printf("ua6=%d\n", ua6ActualSize);

	fastIterateLevel();
	//iterateLevel();
	printf("\tpuz=%d\tch=%d", nPuzzles, nChecked);
	s0=100*d0/s0;//d0/=1000000;
	s1=100*d1/s1;d1/=1000000;
	s2=100*d2/s2;d2/=1000000;
	s3=100*d3/s3;d3/=1000000;
	s4=100*d4/s4;d4/=1000000;
	s5=100*d5/s5;d5/=1000000;
	printf("\n%llu(%llu%%)\t%lluM(%llu%%)\t%lluM(%llu%%)\t%lluM(%llu%%)\t%lluM(%llu%%)\t%lluM(%llu%%)\n",d0,s0,d1,s1,d2,s2,d3,s3,d4,s4,d5,s5);
}

extern int fastScan() {
	clock_t start, finish;
	char buf[3000];
	while(fgets(buf, sizeof(buf), stdin)) {
		start = clock();
		printf("%81.81s", buf);
		grid g;
		g.fromString(buf);
		fastClueIterator ci(g);
		ci.iterate();
		finish = clock();
		fprintf(stdout, "\ttime %2.3f seconds.\n", (double)(finish - start) / CLOCKS_PER_SEC);
		fflush(NULL);
		//return 0; //bug in eclipse???
	}
	return 0;
}
//extern int test() {
//	return fastScan();
//}
