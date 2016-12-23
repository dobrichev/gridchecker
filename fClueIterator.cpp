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

//typedef bit_mask<512> bm1_index_type; //closest to McGuire (smallest)
//typedef bit_mask<8192> bm2_index_type; //10240
//typedef bit_mask<16384> bm3_index_type; //32768
//typedef bit_mask<32768> bm4_index_type; //65536
//typedef bit_mask<16384> bm5_index_type; //40960
//typedef bit_mask<8192> bm6_index_type; //32768
//
//typedef bit_mask<256> fbm1_index_type;
//typedef bit_mask<512> fbm2_index_type; //768
//typedef bit_mask<1280> fbm3_index_type; //1536
//typedef bit_mask<1536> fbm4_index_type; //1792
//typedef bit_mask<1536> fbm5_index_type; //2048
//typedef bit_mask<1536> fbm6_index_type; //1792

typedef bit_mask<768> bm1_index_type; //768,1024  medium
typedef bit_mask<10240> bm2_index_type; //10240
typedef bit_mask<32768> bm3_index_type; //32768
typedef bit_mask<40960> bm4_index_type; //65536
typedef bit_mask<20480> bm5_index_type; //40960
typedef bit_mask<20480> bm6_index_type; //32768

typedef bit_mask<256> fbm1_index_type;
typedef bit_mask<768> fbm2_index_type; //768
typedef bit_mask<1536> fbm3_index_type; //1536
typedef bit_mask<1792> fbm4_index_type; //1792
typedef bit_mask<1792> fbm5_index_type; //2048
typedef bit_mask<1536> fbm6_index_type; //1792

//typedef bit_mask<768> bm1_index_type; //768,1024  large
//typedef bit_mask<10240> bm2_index_type; //10240
//typedef bit_mask<32768> bm3_index_type; //32768
//typedef bit_mask<65536> bm4_index_type; //65536
//typedef bit_mask<40960> bm5_index_type; //40960
//typedef bit_mask<32768> bm6_index_type; //32768
//
//typedef bit_mask<256> fbm1_index_type;
//typedef bit_mask<768> fbm2_index_type; //768
//typedef bit_mask<1792> fbm3_index_type; //1536
//typedef bit_mask<1792> fbm4_index_type; //1792
//typedef bit_mask<1792> fbm5_index_type; //2048
//typedef bit_mask<1792> fbm6_index_type; //1792, zero disables this functionality

struct starters {
	int starter2;
	int starter3;
	int starter4;
	int starter5;
	int starter6;
};
starters stFamily[] = {
	{44,30,27,21,19}, //0 original McGuire 16s
	{46,33,28,20,19}, //1 low, test "a"
	{46,34,28,20,19}, //2 low, test "a"
	{47,37,29,22,19}, //3
	{50,40,30,27,20}, //4 yet higher
	{54,42,30,27,21}, //5
	{56,44,30,27,21}, //6 shift right (only b6b is a bit better than for 50,40,30,27,20)
	{58,45,31,27,21}, //7
	{52,42,38,33,30}, //8 for low u4
	{43,29,26,20,18}, //9 original -1
	{44,30,27,21,18}, //10 original but u6 down
	{44,30,27,21,15}, //11 original McGuire 16s with lower U6
	{44,30,27,21,20}, //12 original but u6 up
};

class fastClueIterator {
private:
	//void forecastIterateLevel(int currentUaIndex = 0);
	inline void fastIterateLevel0(const dead_clues_type deadClues1, const fbm1_index_type fua1_alive1) __attribute__((noinline));
	inline void fastIterateLevel1(const dead_clues_type deadClues2,
			const fbm1_index_type fua1_alive2, const fbm2_index_type &fua2_alive2) __attribute__((noinline));
	inline void fastIterateLevel2(const dead_clues_type deadClues3,
			const fbm1_index_type fua1_alive3, const fbm2_index_type &fua2_alive3, const fbm3_index_type &fua3_alive3) __attribute__((noinline));
	void fastIterateLevel3(const dead_clues_type deadClues4,
			const fbm1_index_type fua1_alive4, const fbm2_index_type &fua2_alive4, const fbm3_index_type &fua3_alive4,
			const fbm4_index_type &fua4_alive4) __attribute__((noinline));
	void fastIterateLevel4(const dead_clues_type &deadClues5,
			const fbm1_index_type fua1_alive5, const fbm2_index_type &fua2_alive5, const fbm3_index_type &fua3_alive5,
			const fbm4_index_type &fua4_alive5, const fbm5_index_type &fua5_alive5) __attribute__((noinline));
	void fastIterateLevel5(const dead_clues_type &deadClues6,
			const fbm1_index_type &fua1_alive6, const fbm2_index_type &fua2_alive6, const fbm3_index_type &fua3_alive6,
			const fbm4_index_type &fua4_alive6, const fbm5_index_type &fua5_alive6, const fbm6_index_type &fua6_alive6) __attribute__((noinline));
	void fastIterateLevel9to6(const dead_clues_type &deadClues_old,
			const fbm1_index_type &fua1_alive_old, const fbm2_index_type &fua2_alive_old, const fbm3_index_type &fua3_alive_old,
			const fbm4_index_type &fua4_alive_old, const fbm5_index_type &fua5_alive_old, const fbm6_index_type &fua6_alive_old) __attribute__((noinline));
	void fastIterateLevel10(const dead_clues_type &deadClues11,
			const bm1_index_type &ua1_alive11, const fbm2_index_type &fua2_alive11, const fbm3_index_type &fua3_alive11,
			const fbm4_index_type &fua4_alive11, const fbm5_index_type &fua5_alive11, const fbm6_index_type &fua6_alive11) __attribute__((noinline));
	void fastIterateLevel11(const dead_clues_type &deadClues12,
			const bm1_index_type &ua1_alive12, const bm2_index_type &ua2_alive12, const bm3_index_type &ua3_alive12,
			const fbm4_index_type &fua4_alive12, const fbm5_index_type &fua5_alive12, const fbm6_index_type &fua6_alive12) __attribute__((noinline));
	void fastIterateLevel12(const dead_clues_type &deadClues13,
			const bm1_index_type &ua1_alive13, const bm2_index_type &ua2_alive13, const bm3_index_type &ua3_alive13,
			const bm4_index_type &ua4_alive13, const fbm5_index_type &fua5_alive13, const fbm6_index_type &fua6_alive13) __attribute__((noinline));
	void fastIterateLevel13(const dead_clues_type &deadClues14,
			const bm1_index_type &ua1_alive14, const bm2_index_type &ua2_alive14, const bm3_index_type &ua3_alive14,
			const bm4_index_type &ua4_alive14, const bm5_index_type &ua5_alive14, const bm6_index_type &ua6_alive14) __attribute__((noinline));
	void fastIterateLevel(const dead_clues_type &deadClues_old,
			const bm1_index_type &ua1_alive_old, const bm2_index_type &ua2_alive_old, const bm3_index_type &ua3_alive_old,
			const bm4_index_type &ua4_alive_old, const bm5_index_type &ua5_alive_old, const bm6_index_type &ua6_alive_old) __attribute__((noinline));
	void buildComposites();
	void checkPuzzle();
	void checkPuzzle(int clueNumber, const dead_clues_type & dc, int startPos = 0);
	fastClueIterator();
public:
	bm1_index_type ua1_indexes[81];
	bm2_index_type ua2_indexes[81];
	bm3_index_type ua3_indexes[81];
	bm4_index_type ua4_indexes[81];
	bm5_index_type ua5_indexes[81];
	bm6_index_type ua6_indexes[81];
	fbm1_index_type fua1_indexes[81];
	fbm2_index_type fua2_indexes[81];
	fbm3_index_type fua3_indexes[81];
	fbm4_index_type fua4_indexes[81];
	fbm5_index_type fua5_indexes[81];
	fbm6_index_type fua6_indexes[81];
	sizedUset ua[bm1_index_type::maxSize];
	sizedUset ua2[bm2_index_type::maxSize];
	sizedUset ua3[bm3_index_type::maxSize];
	sizedUset ua4[bm4_index_type::maxSize];
	sizedUset ua5[bm5_index_type::maxSize];
	sizedUset ua6[bm6_index_type::maxSize];
	sizedUset fUa[fbm1_index_type::maxSize];
	sizedUset fUa2[fbm2_index_type::maxSize];
	sizedUset fUa3[fbm3_index_type::maxSize];
	sizedUset fUa4[fbm4_index_type::maxSize];
	sizedUset fUa5[fbm5_index_type::maxSize];
	sizedUset fUa6[fbm6_index_type::maxSize];
	uset usets[bm1_index_type::maxSize]; //same as ua but with sizes
	uset fUsets[fbm1_index_type::maxSize]; //same as fUa but with sizes
	bm1_index_type ua1_alive_initial;
	bm2_index_type ua2_alive_initial;
	bm3_index_type ua3_alive_initial;
	bm4_index_type ua4_alive_initial;
	bm5_index_type ua5_alive_initial;
	bm6_index_type ua6_alive_initial;

	dead_clues_type deadClues_initial;

	int fuaActualSize;
	int fua2ActualSize;
	int fua3ActualSize;
	int fua4ActualSize;
	int fua5ActualSize;
	int fua6ActualSize;
	int uaActualSize;
	int ua2ActualSize;
	int ua3ActualSize;
	int ua4ActualSize;
	int ua5ActualSize;
	int ua6ActualSize;

	int clueNumber; //nClues - 1 .. 0
	int nClues;
	unsigned int nPuzzles;
	unsigned int nChecked;
	grid &g;
	//forecastState fState[11];
	std::set< int > topUA;
	starters starter;
	char clues[81];

	fastClueIterator(grid &g);
	void iterate();
};
//void fastClueIterator::forecastIterateLevel(int currentUaIndex) {
//	forecastState &oldState = fState[clueNumber];
//	clueNumber--;
//	forecastState &newState = fState[clueNumber];
//	newState.deadClues = oldState.deadClues;
//	//prepare state for the next clue
//	uset u((ua)[currentUaIndex].bitmap128);
//	u &= maskLSB[81];
//	u.clearBits(oldState.deadClues);
//	u.positionsByBitmap();
//	for(int i = 0; i < u.nbits; i++) {
//		int cluePosition = u.positions[i];
//		//printf("clue[%d] using ua[%3d] cell[%2.d]\n", clueNumber, currentUaIndex, cluePosition);
//		newState.setMask.hitOnly(oldState.setMask, hittingMasks[cluePosition]);
//		int nextUaIndex = newState.setMask.firstUnhit();
//		topUA.insert(nextUaIndex);
//		if(clueNumber) {
//			forecastIterateLevel(nextUaIndex); //call recursively
//		}
//		newState.deadClues.setBit(cluePosition);
//	}
//	clueNumber++;
//}

//void fastClueIterator::fastIterateLevel0(int currentUaIndex) {
//	fastState &oldState = state[1];
//	if(currentUaIndex == INT_MAX) {
//		//iterate over all non-dead clues
//		checkPuzzle(oldState.deadClues);
//		goto done;
//	}
//	else {
//		register __m256i setMask = oldState.fSetMask.getTopWord();
//		uset &u = fUsets[currentUaIndex];
//		for(int i = 0; i < u.nbits; i++) {
//			int cluePosition = u.positions[i];
//			if(oldState.deadClues.isBitSet(cluePosition))
//				continue;
//			s0++;
//			//if(oldState.fSetMask.isHittingAll(fHittingMasks[cluePosition])) {
//			if(fua1_type::isHitting(fHittingMasks[cluePosition].getTopWord(), setMask)) {
//				d0++;
//				//all UA are hit, check the puzzle for uniqueness
//				//solve
//				clueNumber = 0;
//				clues[cluePosition] = g.digits[cluePosition];
//				checkPuzzle(oldState.deadClues);
//				clues[cluePosition] = 0;
//			}
//		}
//	}
//done:
//	clueNumber = 1;
//}
//void fastClueIterator::fastIterateLevel1(int currentUaIndex, bm128& deadClues2) {
//	fastState &oldState = state[2];
//	//register bm128 deadClues1 = oldState.deadClues; //dead clues after clue 2 placement
//	register bm128 deadClues1 = deadClues2; //dead clues after clue 2 placement
//	if(currentUaIndex == INT_MAX) {
//		//iterate over all non-dead clues
//		checkPuzzle(2, deadClues1);
//		goto done;
//	}
//	else {
//		clueNumber = 1;
//		register __m256i setMask2 = oldState.fSetMask.getTopWord(); //unhit ua1 after clue 2 placement
//		uset &u = fUsets[currentUaIndex]; //topmost unhit ua1 after clue 2 placement
//		for(int i = 0; i < u.nbits; i++) { //iterate clue 1
//			int cluePosition = u.positions[i];
//			if(deadClues1.isBitSet(cluePosition))
//				continue;
//			s1++;
//			if(state[2].fSetMask2.isHittingAll(fHittingMasks2[cluePosition])) {
//				d1++;
//				//all ua2 are hit, check ua1
//				//newState.fSetMask.hitOnly(oldState.fSetMask, fHittingMasks[cluePosition]);
//				register __m256i setMask1 = fua1_type::hitWord(fHittingMasks[cluePosition].getTopWord(), setMask2);
//				//int nextUaIndex = newState.fSetMask.firstUnhit();
//				int nextUaIndex = fua1_type::firstUnhitWord(setMask1);
//				clues[cluePosition] = g.digits[cluePosition];
//
//				//fastIterateLevel0(nextUaIndex);
//				if(nextUaIndex == INT_MAX) {
//					//iterate over all non-dead clues
//					checkPuzzle(1, deadClues1);
//					//goto done;
//				}
//				else {
//					//register __m256i setMask1 = oldState.fSetMask.getTopWord();
//					uset &u = fUsets[nextUaIndex];
//					for(int i = 0; i < u.nbits; i++) { //iterate clue 0
//						int clue0Position = u.positions[i];
//						if(deadClues1.isBitSet(clue0Position))
//							continue;
//						s0++;
//						//if(oldState.fSetMask.isHittingAll(fHittingMasks[cluePosition])) {
//						if(fua1_type::isHitting(fHittingMasks[clue0Position].getTopWord(), setMask1)) {
//							d0++;
//							//all UA are hit, check the puzzle for uniqueness
//							//solve
//							clueNumber = 0;
//							clues[clue0Position] = g.digits[clue0Position];
//							checkPuzzle(0);
//							clues[clue0Position] = 0;
//						}
//					}
//				}
//				clueNumber = 1;
//				clues[cluePosition] = 0;
//			}
//			deadClues1.setBit(cluePosition);
//		}
//	}
//done:
//	clueNumber = 2;
//}
void fastClueIterator::fastIterateLevel0(__restrict const dead_clues_type deadClues1, __restrict const fbm1_index_type fua1_alive1) {
	int uaIndex0 = fua1_alive1.getMinIndex();
	if(uaIndex0 != INT_MAX) {
		clueNumber = 0;
		const uset &u = fUsets[uaIndex0];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues1.isBitSet(cluePosition)) //todo: we know the latest position isn't dead
				continue;
			s0++;
			if(fua1_alive1.isSubsetOf(fua1_indexes[cluePosition])) {
				d0++;
				clues[cluePosition] = g.digits[cluePosition];
				checkPuzzle();
				clues[cluePosition] = 0;
			}
		}
		clueNumber = 1;
	}
	else {
		//iterate over all non-dead clues
		checkPuzzle(1, deadClues1);
	}
}
void fastClueIterator::fastIterateLevel1(__restrict const dead_clues_type deadClues2,
		__restrict const fbm1_index_type fua1_alive2, __restrict const fbm2_index_type & fua2_alive2) {
	int uaIndex1 = fua1_alive2.getMinIndex();
	if(uaIndex1 != INT_MAX) {
		clueNumber = 1;
		dead_clues_type deadClues1(deadClues2);
		const uset &u = fUsets[uaIndex1];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues2.isBitSet(cluePosition))
				continue;
			s1++;
			if(fua2_alive2.isSubsetOf(fua2_indexes[cluePosition])) {
				d1++;
				fbm1_index_type fua1_alive1(fua1_alive2, fua1_indexes[cluePosition]); //hit
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel0(deadClues1, fua1_alive1);
//				{
//					int uaIndex0 = fua1_alive1.getMinIndex();
//					if(uaIndex0 != INT_MAX) {
//						clueNumber = 0;
//						const uset &u = fUsets[uaIndex0];
//						for(unsigned int i = 0; i < u.nbits; i++) {
//							int cluePosition = u.positions[i];
//							if(deadClues1.isBitSet(cluePosition))
//								continue;
//							s0++;
//							if(fua1_alive1.isSubsetOf(fua1_indexes[cluePosition])) {
//								d0++;
//								clues[cluePosition] = g.digits[cluePosition];
//								checkPuzzle();
//								clues[cluePosition] = 0;
//							}
//						}
//						clueNumber = 1;
//					}
//					else {
//						//iterate over all non-dead clues
//						checkPuzzle(1, deadClues1);
//					}
//				}
				clues[cluePosition] = 0;
			}
			deadClues1.setBit(cluePosition);
		}
		clueNumber = 2;
	}
	else {
		//iterate over all non-dead clues
		checkPuzzle(2, deadClues2);
	}
}
void fastClueIterator::fastIterateLevel2(__restrict const dead_clues_type deadClues3,
		__restrict const fbm1_index_type fua1_alive3, __restrict const fbm2_index_type & fua2_alive3, __restrict const fbm3_index_type & fua3_alive3) {
	int uaIndex2 = fua1_alive3.getMinIndex();
	if(uaIndex2 != INT_MAX) {
		clueNumber = 2;
		dead_clues_type deadClues2(deadClues3);
		const uset &u = fUsets[uaIndex2];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues3.isBitSet(cluePosition))
				continue;
			s2++;
			if(fua3_alive3.isSubsetOf(fua3_indexes[cluePosition])) {
				d2++;
				fbm2_index_type fua2_alive2(fua2_alive3, fua2_indexes[cluePosition]); //hit
				fbm1_index_type fua1_alive2(fua1_alive3, fua1_indexes[cluePosition]); //hit
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel1(deadClues2, fua1_alive2, fua2_alive2);
				clues[cluePosition] = 0;
			}
			deadClues2.setBit(cluePosition);
		}
		clueNumber = 3;
	}
	else {
		//iterate over all non-dead clues
		checkPuzzle(3, deadClues3);
	}
}
void fastClueIterator::fastIterateLevel3(__restrict const dead_clues_type deadClues4,
		__restrict const fbm1_index_type fua1_alive4, __restrict const fbm2_index_type & fua2_alive4, __restrict const fbm3_index_type & fua3_alive4,
		__restrict const fbm4_index_type & fua4_alive4) {
	int uaIndex3 = fua1_alive4.getMinIndex();
	if(uaIndex3 != INT_MAX) {
		clueNumber = 3;
		dead_clues_type deadClues3(deadClues4);
		const uset &u = fUsets[uaIndex3];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues4.isBitSet(cluePosition))
				continue;
			s3++;
			if(fua4_alive4.isSubsetOf(fua4_indexes[cluePosition])) {
				d3++;
				fbm3_index_type fua3_alive3(fua3_alive4, fua3_indexes[cluePosition]); //hit
				fbm2_index_type fua2_alive3(fua2_alive4, fua2_indexes[cluePosition]); //hit
				fbm1_index_type fua1_alive3(fua1_alive4, fua1_indexes[cluePosition]); //hit
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel2(deadClues3, fua1_alive3, fua2_alive3, fua3_alive3);
				clues[cluePosition] = 0;
			}
			deadClues3.setBit(cluePosition);
		}
		clueNumber = 4;
	}
	else {
		//iterate over all non-dead clues
		checkPuzzle(4, deadClues4);
	}
}
void fastClueIterator::fastIterateLevel4(__restrict const dead_clues_type & deadClues5,
		__restrict const fbm1_index_type fua1_alive5, __restrict const fbm2_index_type & fua2_alive5, __restrict const fbm3_index_type & fua3_alive5,
		__restrict const fbm4_index_type & fua4_alive5, __restrict const fbm5_index_type & fua5_alive5) {
	int uaIndex5 = fua1_alive5.getMinIndex();
	if(uaIndex5 != INT_MAX) {
		clueNumber = 4;
		dead_clues_type deadClues4(deadClues5);
		const uset &u = fUsets[uaIndex5];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues5.isBitSet(cluePosition))
				continue;
			s4++;
			if(fua5_alive5.isSubsetOf(fua5_indexes[cluePosition])) {
				d4++;
				fbm4_index_type fua4_alive4(fua4_alive5, fua4_indexes[cluePosition]); //hit
				fbm3_index_type fua3_alive4(fua3_alive5, fua3_indexes[cluePosition]); //hit
				fbm2_index_type fua2_alive4(fua2_alive5, fua2_indexes[cluePosition]); //hit
				fbm1_index_type fua1_alive4(fua1_alive5, fua1_indexes[cluePosition]); //hit
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel3(deadClues4, fua1_alive4, fua2_alive4, fua3_alive4, fua4_alive4);
				clues[cluePosition] = 0;
			}
			deadClues4.setBit(cluePosition);
		}
		clueNumber = 5;
	}
	else {
		//iterate over all non-dead clues
		checkPuzzle(5, deadClues5);
	}
}
void fastClueIterator::fastIterateLevel5(__restrict const dead_clues_type & deadClues6,
		__restrict const fbm1_index_type & fua1_alive6, __restrict const fbm2_index_type & fua2_alive6, __restrict const fbm3_index_type & fua3_alive6,
		__restrict const fbm4_index_type & fua4_alive6, __restrict const fbm5_index_type & fua5_alive6, __restrict const fbm6_index_type & fua6_alive6) {
	int uaIndex6 = fua1_alive6.getMinIndex();
	if(uaIndex6 != INT_MAX) {
		clueNumber = 5;
		dead_clues_type deadClues5(deadClues6);
		const uset &u = fUsets[uaIndex6];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues6.isBitSet(cluePosition))
				continue;
			s5++;
			if(fua6_alive6.isSubsetOf(fua6_indexes[cluePosition])) {
				d5++;
				fbm5_index_type fua5_alive5(fua5_alive6, fua5_indexes[cluePosition]); //hit
				fbm4_index_type fua4_alive5(fua4_alive6, fua4_indexes[cluePosition]); //hit
				fbm3_index_type fua3_alive5(fua3_alive6, fua3_indexes[cluePosition]); //hit
				fbm2_index_type fua2_alive5(fua2_alive6, fua2_indexes[cluePosition]); //hit
				fbm1_index_type fua1_alive5(fua1_alive6, fua1_indexes[cluePosition]); //hit
				clues[cluePosition] = g.digits[cluePosition];
				fastIterateLevel4(deadClues5,
						fua1_alive5, fua2_alive5, fua3_alive5,
						fua4_alive5, fua5_alive5);
				clues[cluePosition] = 0;
			}
			deadClues5.setBit(cluePosition);
		}
		clueNumber = 6;
	}
	else {
		//iterate over all non-dead clues
		checkPuzzle(6, deadClues6);
	}
}
void fastClueIterator::fastIterateLevel9to6(__restrict const dead_clues_type &deadClues_old,
		__restrict const fbm1_index_type &fua1_alive_old, __restrict const fbm2_index_type &fua2_alive_old, __restrict const fbm3_index_type &fua3_alive_old,
		__restrict const fbm4_index_type &fua4_alive_old, __restrict const fbm5_index_type &fua5_alive_old, __restrict const fbm6_index_type &fua6_alive_old) {
	int uaIndex_old = fua1_alive_old.getMinIndex();
	if(uaIndex_old != INT_MAX) {
		clueNumber--;
		dead_clues_type deadClues_new(deadClues_old);
		const uset &u = fUsets[uaIndex_old];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues_old.isBitSet(cluePosition))
				continue;
			fbm6_index_type fua6_alive_new(fua6_alive_old, fua6_indexes[cluePosition]); //hit
			fbm5_index_type fua5_alive_new(fua5_alive_old, fua5_indexes[cluePosition]); //hit
			fbm4_index_type fua4_alive_new(fua4_alive_old, fua4_indexes[cluePosition]); //hit
			fbm3_index_type fua3_alive_new(fua3_alive_old, fua3_indexes[cluePosition]); //hit
			fbm2_index_type fua2_alive_new(fua2_alive_old, fua2_indexes[cluePosition]); //hit
			fbm1_index_type fua1_alive_new(fua1_alive_old, fua1_indexes[cluePosition]); //hit
			clues[cluePosition] = g.digits[cluePosition];
			if(clueNumber == 6) {
				fastIterateLevel5(deadClues_new,
						fua1_alive_new, fua2_alive_new, fua3_alive_new,
						fua4_alive_new, fua5_alive_new, fua6_alive_new);
			}
			else {
				fastIterateLevel9to6(deadClues_new,
						fua1_alive_new, fua2_alive_new, fua3_alive_new,
						fua4_alive_new, fua5_alive_new, fua6_alive_new);
			}
			clues[cluePosition] = 0;
			deadClues_new.setBit(cluePosition);
		}
		clueNumber++;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
		return;
	}
}
void fastClueIterator::fastIterateLevel10(__restrict const dead_clues_type &deadClues11,
		__restrict const bm1_index_type &ua1_alive11, __restrict const fbm2_index_type &fua2_alive11, __restrict const fbm3_index_type &fua3_alive11,
		__restrict const fbm4_index_type &fua4_alive11, __restrict const fbm5_index_type &fua5_alive11, __restrict const fbm6_index_type &fua6_alive11) {
	int uaIndex11 = ua1_alive11.getMinIndex();
	if(uaIndex11 != INT_MAX) {
		clueNumber = 10;
		dead_clues_type deadClues10(deadClues11);
		const uset &u = usets[uaIndex11];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues11.isBitSet(cluePosition))
				continue;
			fbm6_index_type fua6_alive10(fua6_alive11, fua6_indexes[cluePosition]); //hit
			fbm5_index_type fua5_alive10(fua5_alive11, fua5_indexes[cluePosition]); //hit
			fbm4_index_type fua4_alive10(fua4_alive11, fua4_indexes[cluePosition]); //hit
			fbm3_index_type fua3_alive10(fua3_alive11, fua3_indexes[cluePosition]); //hit
			fbm2_index_type fua2_alive10(fua2_alive11, fua2_indexes[cluePosition]); //hit
			bm1_index_type ua1_alive10(ua1_alive11, ua1_indexes[cluePosition]); //hit
			fuaActualSize = ua1_alive10.copyAlive(ua, fUa, fbm1_index_type::maxSize, deadClues10); //extract
			//printf("fuaActualSize=%d", fuaActualSize);
			std::sort(fUa, fUa + fuaActualSize, sizedUset::isSmaller);
			//fuaActualSize = (std::unique(fUa, fUa + fuaActualSize) - &fUa[0]);
			//printf("\t%d\n", fuaActualSize);
			for(int n = 0; n < fuaActualSize; n++) {
				fUsets[n] = fUa[n];
				fUsets[n] &= maskLSB[81];
				fUsets[n].positionsByBitmap();
			}
			fbm1_index_type fua1_alive10(fUa, fuaActualSize, fua1_indexes); //rebuild
			clues[cluePosition] = g.digits[cluePosition];
			fastIterateLevel9to6(deadClues10,
					fua1_alive10, fua2_alive10, fua3_alive10,
					fua4_alive10, fua5_alive10, fua6_alive10);
			clues[cluePosition] = 0;
			deadClues10.setBit(cluePosition);
		}
		clueNumber = 11;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
		return;
	}
}
void fastClueIterator::fastIterateLevel11(const dead_clues_type &deadClues12,
		const bm1_index_type &ua1_alive12, const bm2_index_type &ua2_alive12, const bm3_index_type &ua3_alive12,
		const fbm4_index_type &fua4_alive12, const fbm5_index_type &fua5_alive12, const fbm6_index_type &fua6_alive12) {
	int uaIndex12 = ua1_alive12.getMinIndex();
	if(uaIndex12 != INT_MAX) {
		clueNumber = 11;
		dead_clues_type deadClues11(deadClues12);
		const uset &u = usets[uaIndex12];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues12.isBitSet(cluePosition))
				continue;
			fbm6_index_type fua6_alive11(fua6_alive12, fua6_indexes[cluePosition]); //hit
			fbm5_index_type fua5_alive11(fua5_alive12, fua5_indexes[cluePosition]); //hit
			fbm4_index_type fua4_alive11(fua4_alive12, fua4_indexes[cluePosition]); //hit
			bm3_index_type ua3_alive11(ua3_alive12, ua3_indexes[cluePosition]); //hit
			fua3ActualSize = ua3_alive11.copyAlive(ua3, fUa3, fbm3_index_type::maxSize, deadClues11); //extract
			fbm3_index_type fua3_alive11(fUa3, fua3ActualSize, fua3_indexes); //rebuild
			bm2_index_type ua2_alive11(ua2_alive12, ua2_indexes[cluePosition]); //hit
			fua2ActualSize = ua2_alive11.copyAlive(ua2, fUa2, fbm2_index_type::maxSize, deadClues11); //extract
			fbm2_index_type fua2_alive11(fUa2, fua2ActualSize, fua2_indexes); //rebuild
			bm1_index_type ua1_alive11(ua1_alive12, ua1_indexes[cluePosition]); //hit
			clues[cluePosition] = g.digits[cluePosition];
			fastIterateLevel10(deadClues11,
					ua1_alive11, fua2_alive11, fua3_alive11,
					fua4_alive11, fua5_alive11, fua6_alive11);
			clues[cluePosition] = 0;
			deadClues11.setBit(cluePosition);
		}
		clueNumber = 12;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
		return;
	}
}
void fastClueIterator::fastIterateLevel12(const dead_clues_type &deadClues13,
		const bm1_index_type &ua1_alive13, const bm2_index_type &ua2_alive13, const bm3_index_type &ua3_alive13,
		const bm4_index_type &ua4_alive13, const fbm5_index_type &fua5_alive13, const fbm6_index_type &fua6_alive13) {
	int uaIndex13 = ua1_alive13.getMinIndex();
	if(uaIndex13 != INT_MAX) {
		clueNumber = 12;
		dead_clues_type deadClues12(deadClues13);
		const uset &u = usets[uaIndex13];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues13.isBitSet(cluePosition))
				continue;
			fbm6_index_type fua6_alive12(fua6_alive13, fua6_indexes[cluePosition]); //hit
			fbm5_index_type fua5_alive12(fua5_alive13, fua5_indexes[cluePosition]); //hit
			bm4_index_type ua4_alive12(ua4_alive13, ua4_indexes[cluePosition]); //hit
			fua4ActualSize = ua4_alive12.copyAlive(ua4, fUa4, fbm4_index_type::maxSize, deadClues12); //extract
			fbm4_index_type fua4_alive12(fUa4, fua4ActualSize, fua4_indexes); //rebuild
			bm3_index_type ua3_alive12(ua3_alive13, ua3_indexes[cluePosition]); //hit
			bm2_index_type ua2_alive12(ua2_alive13, ua2_indexes[cluePosition]); //hit
			bm1_index_type ua1_alive12(ua1_alive13, ua1_indexes[cluePosition]); //hit
			clues[cluePosition] = g.digits[cluePosition];
			fastIterateLevel11(deadClues12,
					ua1_alive12, ua2_alive12, ua3_alive12,
					fua4_alive12, fua5_alive12, fua6_alive12);
			clues[cluePosition] = 0;
			deadClues12.setBit(cluePosition);
		}
		clueNumber = 13;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
		return;
	}
}
void fastClueIterator::fastIterateLevel13(const dead_clues_type &deadClues14,
		const bm1_index_type &ua1_alive14, const bm2_index_type &ua2_alive14, const bm3_index_type &ua3_alive14,
		const bm4_index_type &ua4_alive14, const bm5_index_type &ua5_alive14, const bm6_index_type &ua6_alive14) {
	int uaIndex14 = ua1_alive14.getMinIndex();
	if(uaIndex14 != INT_MAX) {
		clueNumber = 13;
		dead_clues_type deadClues13(deadClues14);
		const uset &u = usets[uaIndex14];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues14.isBitSet(cluePosition))
				continue;
			bm6_index_type ua6_alive13(ua6_alive14, ua6_indexes[cluePosition]); //hit
			fua6ActualSize = ua6_alive13.copyAlive(ua6, fUa6, fbm6_index_type::maxSize, deadClues13); //extract
			fbm6_index_type fua6_alive13(fUa6, fua6ActualSize, fua6_indexes); //rebuild
			bm5_index_type ua5_alive13(ua5_alive14, ua5_indexes[cluePosition]); //hit
			fua5ActualSize = ua5_alive13.copyAlive(ua5, fUa5, fbm5_index_type::maxSize, deadClues13); //extract
			fbm5_index_type fua5_alive13(fUa5, fua5ActualSize, fua5_indexes); //rebuild
			bm4_index_type ua4_alive13(ua4_alive14, ua4_indexes[cluePosition]); //hit
			bm3_index_type ua3_alive13(ua3_alive14, ua3_indexes[cluePosition]); //hit
			bm2_index_type ua2_alive13(ua2_alive14, ua2_indexes[cluePosition]); //hit
			bm1_index_type ua1_alive13(ua1_alive14, ua1_indexes[cluePosition]); //hit
			clues[cluePosition] = g.digits[cluePosition];
			fastIterateLevel12(deadClues13,
					ua1_alive13, ua2_alive13, ua3_alive13,
					ua4_alive13, fua5_alive13, fua6_alive13);
			clues[cluePosition] = 0;
			deadClues13.setBit(cluePosition);
		}
		clueNumber = 14;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
		return;
	}
}
void fastClueIterator::fastIterateLevel(const dead_clues_type &deadClues_old,
		const bm1_index_type &ua1_alive_old, const bm2_index_type &ua2_alive_old, const bm3_index_type &ua3_alive_old,
		const bm4_index_type &ua4_alive_old, const bm5_index_type &ua5_alive_old, const bm6_index_type &ua6_alive_old) {
	int uaIndex_old = ua1_alive_old.getMinIndex();
	if(uaIndex_old != INT_MAX) {
		clueNumber--;
		dead_clues_type deadClues_new(deadClues_old);
		const uset &u = usets[uaIndex_old];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition = u.positions[i];
			if(deadClues_old.isBitSet(cluePosition))
				continue;
			bm6_index_type ua6_alive_new(ua6_alive_old, ua6_indexes[cluePosition]); //hit
			bm5_index_type ua5_alive_new(ua5_alive_old, ua5_indexes[cluePosition]); //hit
			bm4_index_type ua4_alive_new(ua4_alive_old, ua4_indexes[cluePosition]); //hit
			bm3_index_type ua3_alive_new(ua3_alive_old, ua3_indexes[cluePosition]); //hit
			bm2_index_type ua2_alive_new(ua2_alive_old, ua2_indexes[cluePosition]); //hit
			bm1_index_type ua1_alive_new(ua1_alive_old, ua1_indexes[cluePosition]); //hit
			clues[cluePosition] = g.digits[cluePosition];
			if(clueNumber == 14) {
			fastIterateLevel13(deadClues_new,
					ua1_alive_new, ua2_alive_new, ua3_alive_new,
					ua4_alive_new, ua5_alive_new, ua6_alive_new);
			}
			else {
				fastIterateLevel(deadClues_new,
						ua1_alive_new, ua2_alive_new, ua3_alive_new,
						ua4_alive_new, ua5_alive_new, ua6_alive_new);
			}
			clues[cluePosition] = 0;
			deadClues_new.setBit(cluePosition);
		}
		clueNumber++;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
		return;
	}
}

void fastClueIterator::buildComposites() {
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
				if(ua2ActualSize >= bm2_index_type::maxSize)
					goto ua2composed;
			}
next_s2:;
		}
	}
	ua2composed:;
	ua2_alive_initial = bm2_index_type(ua2, ua2ActualSize, ua2_indexes); //build

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
						if(ua3ActualSize >= bm3_index_type::maxSize)
							goto ua3composed;
					}
next_s3:;
				}
			}
		}
	}

	ua3composed:;
	//ua3_type::bm128ToIndex(ua3, ua3ActualSize, state[clueNumber].setMask3, hittingMasks3);
	ua3_alive_initial = bm3_index_type(ua3, ua3ActualSize, ua3_indexes); //build

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
								//if(tttt.getSize() > 55)
								//	goto next_s4;
								for (int s = 0; s < starter.starter4 - 3; s++) {
									if (ua[s].isSubsetOf(tttt))
										goto next_s4;
								}
								ua4[ua4ActualSize] = tttt;
								ua4ActualSize++;
								if(ua4ActualSize >= bm4_index_type::maxSize)
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
	//ua4_type::bm128ToIndex(ua4, ua4ActualSize, state[clueNumber].setMask4, hittingMasks4);
	ua4_alive_initial = bm4_index_type(ua4, ua4ActualSize, ua4_indexes); //build

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
										//if(ttttt.getSize() > 58)
										//	goto next_s5;
										for (int s = 0; s < starter.starter5 - 4; s++) {
											if (ua[s].isSubsetOf(ttttt))
												goto next_s5;
										}
										ua5[ua5ActualSize] = ttttt;
										ua5ActualSize++;
										if(ua5ActualSize >= bm5_index_type::maxSize)
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
	//ua5_type::bm128ToIndex(ua5, ua5ActualSize, state[clueNumber].setMask5, hittingMasks5);
	ua5_alive_initial = bm5_index_type(ua5, ua5ActualSize, ua5_indexes); //build

	//compose UA6
	ua6ActualSize = 0;

//	//inject an artificial UA6 for band 3
//	sizedUset tt(maskLSB[27]);
//	tt.setSize();
//	ua6[0] = tt;
//	ua6ActualSize = 1;

	if(bm6_index_type::maxSize) {
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
													//if(tttttt.getSize() > 58)
													//	goto next_s6;
													for (int s = 0; s < starter.starter6 - 5; s++) {
														if (ua[s].isSubsetOf(tttttt))
															goto next_s6;
													}
//													//check for subsets/supersets
//													for(int ss = 0; ss < ua6ActualSize; ss++) {
//														if(ua6[ss].isSubsetOf(tttttt)) {
//															//the same free clues are covered by a smaller ua6. Ignore the newly found one.
//															// < 1:1000
//															printf("S");
//															goto next_s6;
//														}
//														if(tttttt.isSubsetOf(ua6[ss])) {
//															//replace the superset with the new set
//															// < 1:100000
//															printf("R");
//															ua6[ss] = tttttt;
//															goto next_s6;
//														}
//													}
													ua6[ua6ActualSize] = tttttt;
													ua6ActualSize++;
													if(ua6ActualSize >= bm6_index_type::maxSize)
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
		//ua6_type::bm128ToIndex(ua6, ua6ActualSize, state[clueNumber].setMask6, hittingMasks6);
		ua6_alive_initial = bm6_index_type(ua6, ua6ActualSize, ua6_indexes); //build
	}
}

fastClueIterator::fastClueIterator(grid &g) :
		fuaActualSize(0), fua2ActualSize(0), fua3ActualSize(0), fua4ActualSize(
				0), fua5ActualSize(0), fua6ActualSize(0), uaActualSize(0), ua2ActualSize(
				0), ua3ActualSize(0), ua4ActualSize(0), ua5ActualSize(0), ua6ActualSize(
				0), clueNumber(0), nClues(opt.scanOpt->nClues), nPuzzles(0), nChecked(
				0), g(g) {
	//nClues = 16;
}

void fastClueIterator::checkPuzzle(int clueNumber, const dead_clues_type & dc, int startPos) {
	if(clueNumber == 0) {
		checkPuzzle();
	}
	else {
		for(int i = startPos; i < 81; i++) {
			if(clues[i]) {	//given
				continue;
			}
			if(dc.isBitSet(i)) { //dead clue
				continue;
			}
			clues[i] = g.digits[i];
			checkPuzzle(clueNumber - 1, dc, i + 1);
			clues[i] = 0;
		}
	}
}
void fastClueIterator::checkPuzzle() {
	if(solve(clues, 2) == 1) {
		ch81 puz;
		puz.toString(clues, puz.chars);
		printf("%81.81s\n", puz.chars);
		nPuzzles++;
	}
	nChecked++;
//	//debug
//	if(nChecked % 300000 == 0) {
//		printf("checked %d, found %d\n", nChecked, nPuzzles);
//	}
}

void fastClueIterator::iterate() {
	//find all UA sets of size 4..12
	g.findUA12();
	usetListBySize &us = g.usetsBySize;
	//printf("\t%d\n", (int)us.size());
	//debug: add all 4-digit UA
	g.findUA4digits();
	g.findUA4boxes(); //slows down the whole process, even w/o taking into account the UA creation
	//printf("\t%d\n", (int)us.size());

	//debug
	g.usetsBySize.setDistributions();
	printf("\n");
	for(int i = 4; i < 20; i++) {
		if(i == 5 || i == 7) continue;
		printf("%d=%d\t", i, g.usetsBySize.distributionBySize[i]);
	}
	printf("\n");

	//int max16Index = 0; //debug
	//init the top of the stack
	int curUA = 0;
	uaActualSize = 0;
	for(usetListBySize::const_iterator p = us.begin(); p != us.end() && uaActualSize < bm1_index_type::maxSize; p++, curUA++) {
		usets[uaActualSize] = *p; //structure copy
		sizedUset su;
		su.bitmap128 = p->bitmap128; //don't calculate the size
		su.setSize(p->nbits);
		//if(p->nbits > 14) break;
		//if(p->nbits <= 16) max16Index = uaActualSize; else break;
		ua[uaActualSize++] = su; //store for hitting
	}

	//printf("maxUa16Index=%d\tuaSize[%d]=%d\n", max16Index, uaActualSize - 1, ua[uaActualSize - 1].getSize());
	//return;

	//compose ordinary UA
	//ua1_type::bm128ToIndex(ua, uaActualSize, state[nClues].setMask, hittingMasks);
	ua1_alive_initial = bm1_index_type(ua, uaActualSize, ua1_indexes); //build

//	clueNumber = 9;
//	fState[clueNumber].deadClues.clear();
//	//fState[clueNumber].setClues.clear();
//	fState[clueNumber].setMask = state[nClues].setMask;
//	for(int i = 0; i < 81; i++) {
//		clues[i] = 0;
//	}
//	topUA.clear();
//	forecastIterateLevel();
//	for(std::set<int>::const_iterator i = topUA.begin(); i != topUA.end(); i++) {
//		printf("ua=%d\n", *i);
//	}
//	return;

	clueNumber = nClues; //stack pointer to the "empty" puzzle
	deadClues_initial.clear(); //no forced non-givens
	//state[clueNumber].deadClues.clear();
	//state[clueNumber].setClues.clear();
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

//	int chosenFamily = 1;
//	if(g.usetsBySize.distributionBySize[4] <= 8)
//		chosenFamily = 8;
//	starter = stFamily[chosenFamily];
	starter = stFamily[0]; //0 works best for random grid, 8 for 17s

	buildComposites();
	//some info for debugging/optimization
	//printf("\t%d clues\n", nClues);
	printf("ua =%d\t", uaActualSize);
	printf("ua2=%d\t", ua2ActualSize);
	printf("ua3=%d\t", ua3ActualSize);
	printf("ua4=%d\t", ua4ActualSize);
	printf("ua5=%d\t", ua5ActualSize);
	printf("ua6=%d\n", ua6ActualSize);

	fastIterateLevel(deadClues_initial,
		ua1_alive_initial, ua2_alive_initial, ua3_alive_initial,
		ua4_alive_initial, ua5_alive_initial, ua6_alive_initial);
	//printf("\tpuz=%d\tch=%d", nPuzzles, nChecked);
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
