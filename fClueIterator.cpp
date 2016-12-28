#define _CRT_SECURE_NO_DEPRECATE

#include <emmintrin.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <set>
#include <queue>
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

typedef std::queue< dead_clues_type > passedBitmaps_t;
struct incomplete_puzzle_t {
	dead_clues_type setClues;
	dead_clues_type deadClues;
	int nPositions;
	incomplete_puzzle_t(const dead_clues_type& s, const dead_clues_type &d,
			int n) :
			setClues(s), deadClues(d), nPositions(n) {}
};
typedef std::queue< incomplete_puzzle_t > passedIncompleteBitmaps_t;

unsigned long long d0, d1, d2, d3, d4, d5, s0, s1, s2, s3, s4, s5; //debug
//compiler options to try:  -fno-unroll-loops -fipa-pta

//#define likely(x)      __builtin_expect(!!(x), 1)
//#define unlikely(x)    __builtin_expect(!!(x), 0)

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

//typedef bit_mask<768> bm1_index_type; //768,1024  medium
//typedef bit_mask<10240> bm2_index_type; //10240
//typedef bit_mask<32768> bm3_index_type; //32768
//typedef bit_mask<40960> bm4_index_type; //40960
//typedef bit_mask<20480> bm5_index_type; //20480
//typedef bit_mask<10240> bm6_index_type; //20480
//
//typedef bit_mask<256> fbm1_index_type;
//typedef bit_mask<1024> fbm2_index_type; //768
//typedef bit_mask<1536> fbm3_index_type; //1536
//typedef bit_mask<1792> fbm4_index_type; //1792
//typedef bit_mask<1792> fbm5_index_type; //2048
//typedef bit_mask<768> fbm6_index_type; //1792

typedef bit_mask<768> bm1_index_type; //768,1024  large
typedef bit_mask<10240> bm2_index_type; //10240
typedef bit_mask<32768> bm3_index_type; //32768
typedef bit_mask<65536> bm4_index_type; //65536
typedef bit_mask<40960> bm5_index_type; //40960
typedef bit_mask<20480> bm6_index_type; //32768

typedef bit_mask<256> fbm1_index_type;
typedef bit_mask<1024> fbm2_index_type; //768
typedef bit_mask<1792> fbm3_index_type; //1536
typedef bit_mask<4096> fbm4_index_type; //1792
typedef bit_mask<1792> fbm5_index_type; //2048
typedef bit_mask<1792> fbm6_index_type; //1792

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
	{48,36,33,25,19}, //3 u4<10 && u6>=30
	{50,40,30,27,20}, //4 yet higher
	{54,42,30,27,21}, //5
	{56,44,30,27,21}, //6 shift right (only b6b is a bit better than for 50,40,30,27,20)
	{58,45,31,27,21}, //7
	{52,42,38,33,30}, //8 for low u4
	{43,29,26,20,18}, //9 original -1
	{44,30,27,21,18}, //10 original but u6 down
	{44,30,27,21,15}, //11 original McGuire 16s with lower U6
	{44,30,27,21,20}, //12 original but u6 up
	{44,30,27,21,21}, //13 original but u6 +2
	{45,30,27,21,19}, //14 original but u2 +1
	{46,30,27,21,19}, //15 original but u2 +2
	{50,30,27,21,19}, //16 original but u2 +6
};

class fastClueIterator {
private:
	//void forecastIterateLevel(int currentUaIndex = 0);
	inline void fastIterateLevel0(__restrict const dead_clues_type deadClues1, __restrict const dead_clues_type &setClues1,
			__restrict const fbm1_index_type fua1_alive1);
	inline void fastIterateLevel1(__restrict const dead_clues_type deadClues2, __restrict const dead_clues_type &setClues2,
			__restrict const fbm1_index_type fua1_alive2, __restrict const fbm2_index_type &fua2_alive2);
	void fastIterateLevel2(__restrict const dead_clues_type deadClues3, __restrict const dead_clues_type &setClues3,
			__restrict const fbm1_index_type fua1_alive3, __restrict const fbm2_index_type &fua2_alive3, __restrict const fbm3_index_type &fua3_alive3);
	void fastIterateLevel3(__restrict const dead_clues_type deadClues4, __restrict const dead_clues_type &setClues4,
			__restrict const fbm1_index_type fua1_alive4, __restrict const fbm2_index_type &fua2_alive4, __restrict const fbm3_index_type &fua3_alive4,
			__restrict const fbm4_index_type &fua4_alive4);
	void fastIterateLevel4(__restrict const dead_clues_type &deadClues5, __restrict const dead_clues_type &setClues5,
			__restrict const fbm1_index_type fua1_alive5, __restrict const fbm2_index_type &fua2_alive5, __restrict const fbm3_index_type &fua3_alive5,
			__restrict const fbm4_index_type &fua4_alive5, __restrict const fbm5_index_type &fua5_alive5);
	void fastIterateLevel5(__restrict const dead_clues_type &deadClues6, __restrict const dead_clues_type &setClues6,
			__restrict const fbm1_index_type &fua1_alive6, __restrict const fbm2_index_type &fua2_alive6, __restrict const fbm3_index_type &fua3_alive6,
			__restrict const fbm4_index_type &fua4_alive6, __restrict const fbm5_index_type &fua5_alive6, __restrict const fbm6_index_type &fua6_alive6) __attribute__((noinline));
	void fastIterateLevel9to6(const dead_clues_type &deadClues_old, const dead_clues_type &setClues_old,
			const fbm1_index_type &fua1_alive_old, const fbm2_index_type &fua2_alive_old, const fbm3_index_type &fua3_alive_old,
			const fbm4_index_type &fua4_alive_old, const fbm5_index_type &fua5_alive_old, const fbm6_index_type &fua6_alive_old) __attribute__((noinline));
	void fastIterateLevel10(const dead_clues_type &deadClues11, const dead_clues_type &setClues11,
			const bm1_index_type &ua1_alive11, const bm2_index_type &ua2_alive11, const fbm3_index_type &fua3_alive11,
			const fbm4_index_type &fua4_alive11, const fbm5_index_type &fua5_alive11, const fbm6_index_type &fua6_alive11) __attribute__((noinline));
	void fastIterateLevel11(const dead_clues_type &deadClues12, const dead_clues_type &setClues12,
			const bm1_index_type &ua1_alive12, const bm2_index_type &ua2_alive12, const bm3_index_type &ua3_alive12,
			const fbm4_index_type &fua4_alive12, const fbm5_index_type &fua5_alive12, const fbm6_index_type &fua6_alive12) __attribute__((noinline));
	void fastIterateLevel12(const dead_clues_type &deadClues13, const dead_clues_type &setClues13,
			const bm1_index_type &ua1_alive13, const bm2_index_type &ua2_alive13, const bm3_index_type &ua3_alive13,
			const bm4_index_type &ua4_alive13, const fbm5_index_type &fua5_alive13, const fbm6_index_type &fua6_alive13) __attribute__((noinline));
	void fastIterateLevel13(const dead_clues_type &deadClues14, const dead_clues_type &setClues14,
			const bm1_index_type &ua1_alive14, const bm2_index_type &ua2_alive14, const bm3_index_type &ua3_alive14,
			const bm4_index_type &ua4_alive14, const bm5_index_type &ua5_alive14, const bm6_index_type &ua6_alive14) __attribute__((noinline));
	void fastIterateLevel(const dead_clues_type &deadClues_old, const dead_clues_type &setClues_old,
			const bm1_index_type &ua1_alive_old, const bm2_index_type &ua2_alive_old, const bm3_index_type &ua3_alive_old,
			const bm4_index_type &ua4_alive_old, const bm5_index_type &ua5_alive_old, const bm6_index_type &ua6_alive_old) __attribute__((noinline));
	void buildComposites();
	//void checkPuzzle(const dead_clues_type &setClues);
	void checkPuzzle(int clueNumber, const dead_clues_type &setClues, const dead_clues_type & dc);
	void expandPuzzle(int clueNumber, const dead_clues_type &setClues, const dead_clues_type & dc, int startPos = 0);
	void solvePuzzle(const dead_clues_type &setClues);
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
	sizedUset fUa[fbm1_index_type::maxSize + 100];
	sizedUset fUa2[fbm2_index_type::maxSize];
	sizedUset fUa3[fbm3_index_type::maxSize];
	sizedUset fUa4[fbm4_index_type::maxSize];
	sizedUset fUa5[fbm5_index_type::maxSize];
	sizedUset fUa6[fbm6_index_type::maxSize];
	uset usets[bm1_index_type::maxSize]; //same as ua but with sizes
	uset fUsets[fbm1_index_type::maxSize + 100]; //same as fUa but with sizes
	bm1_index_type ua1_alive_initial;
	bm2_index_type ua2_alive_initial;
	bm3_index_type ua3_alive_initial;
	bm4_index_type ua4_alive_initial;
	bm5_index_type ua5_alive_initial;
	bm6_index_type ua6_alive_initial;

	dead_clues_type deadClues_initial;
	dead_clues_type setClues_initial;

	dead_clues_type *passedFinalUA_ptr;
	dead_clues_type passedFinalUA[40000];
	//incomplete_puzzle_t *passedFinalUAIncomplete_ptr;
	//incomplete_puzzle_t passedFinalUAIncomplete[10000];

	//passedBitmaps_t passedBM;
	passedIncompleteBitmaps_t passedIncompleteBM;

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
void fastClueIterator::fastIterateLevel0(const dead_clues_type deadClues1, const dead_clues_type &setClues1, const fbm1_index_type fua1_alive1) {
	int uaIndex0 = fua1_alive1.getMinIndex();
	if(uaIndex0 != INT_MAX) {
		const uset &u = fUsets[uaIndex0];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition0 = u.positions[i];
			const bm128 posMask0(bitSet[cluePosition0]);
			if(posMask0.isSubsetOf(deadClues1))
				continue;
			//s0++;
			if(fua1_alive1.isSubsetOf(fua1_indexes[cluePosition0])) {
				//d0++;
				dead_clues_type setClues0(setClues1);
				setClues0 |= posMask0;
				*passedFinalUA_ptr = setClues0;
				//if(passedFinalUA_ptr < passedFinalUA + sizeof(passedFinalUA) / sizeof(passedFinalUA[0]) - 1) passedFinalUA_ptr++;
				passedFinalUA_ptr++;
			}
		}
	}
	else {
		//iterate over all non-dead clues
		//printf("Expanding 1 clue\n"); //debug
		checkPuzzle(1, setClues1, deadClues1);
	}
}
void fastClueIterator::fastIterateLevel1(const dead_clues_type deadClues2, const dead_clues_type &setClues2,
		const fbm1_index_type fua1_alive2, const fbm2_index_type & fua2_alive2) {
	int uaIndex1 = fua1_alive2.getMinIndex();
	if(uaIndex1 != INT_MAX) {
		dead_clues_type deadClues1(deadClues2);
		const uset &u = fUsets[uaIndex1];
//		for(unsigned int i = 0; i < u.nbits; i++) {
//			int cluePosition1 = u.positions[i];
//			const bm128 posMask1(bitSet[cluePosition1]);
//			if(posMask1.isSubsetOf(deadClues2))
//				continue;
//			//s1++;
//			if(fua2_alive2.isSubsetOf(fua2_indexes[cluePosition1])) {
//				//d1++;
//				fbm1_index_type fua1_alive1(fua1_alive2, fua1_indexes[cluePosition1]); //hit
//				dead_clues_type setClues1(setClues2);
//				setClues1 |= posMask1;
//				fastIterateLevel0(deadClues1, setClues1, fua1_alive1);
//			}
//			deadClues1 |= posMask1;
//		}
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition1 = u.positions[i];
			const bm128 posMask1(bitSet[cluePosition1]);
			if(posMask1.isSubsetOf(deadClues2))
				continue;
			//s1++;
			if(fua2_alive2.isSubsetOf(fua2_indexes[cluePosition1])) {
				//d1++;
				fbm1_index_type fua1_alive1(fua1_alive2, fua1_indexes[cluePosition1]); //hit
				dead_clues_type setClues1(setClues2);
				setClues1 |= posMask1;
				fastIterateLevel0(deadClues1, setClues1, fua1_alive1);
			}
			deadClues1 |= posMask1;
		}
	}
	else {
		//iterate over all non-dead clues
		//printf("Expanding 2 clues\n"); //happens but < 1:100
		checkPuzzle(2, setClues2, deadClues2);
	}
}
void fastClueIterator::fastIterateLevel2(const dead_clues_type deadClues3, const dead_clues_type &setClues3,
		const fbm1_index_type fua1_alive3, const fbm2_index_type & fua2_alive3, const fbm3_index_type & fua3_alive3) {
	int uaIndex2 = fua1_alive3.getMinIndex();
	if(uaIndex2 != INT_MAX) {
		dead_clues_type deadClues2(deadClues3);
		const uset &u = fUsets[uaIndex2];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition2 = u.positions[i];
			const bm128 posMask2(bitSet[cluePosition2]);
			if(posMask2.isSubsetOf(deadClues3))
				continue;
			//s2++;
			if(fua3_alive3.isSubsetOf(fua3_indexes[cluePosition2])) {
				//d2++;
				fbm2_index_type fua2_alive2(fua2_alive3, fua2_indexes[cluePosition2]); //hit
				fbm1_index_type fua1_alive2(fua1_alive3, fua1_indexes[cluePosition2]); //hit
				dead_clues_type setClues2(setClues3);
				setClues2 |= posMask2;
				fastIterateLevel1(deadClues2, setClues2, fua1_alive2, fua2_alive2);
			}
			deadClues2 |= posMask2;
		}
	}
	else {
		//iterate over all non-dead clues
		printf("Expanding 3 clues\n"); //debug
		checkPuzzle(3, setClues3, deadClues3);
	}
}
void fastClueIterator::fastIterateLevel3(const dead_clues_type deadClues4, const dead_clues_type &setClues4,
		const fbm1_index_type fua1_alive4, const fbm2_index_type & fua2_alive4, const fbm3_index_type & fua3_alive4,
		const fbm4_index_type & fua4_alive4) {
	int uaIndex3 = fua1_alive4.getMinIndex();
	if(uaIndex3 != INT_MAX) {
		dead_clues_type deadClues3(deadClues4);
		const uset &u = fUsets[uaIndex3];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition3 = u.positions[i];
			const bm128 posMask3(bitSet[cluePosition3]);
			if(posMask3.isSubsetOf(deadClues4))
				continue;
			s3++;
			if(fua4_alive4.isSubsetOf(fua4_indexes[cluePosition3])) {
				d3++;
				fbm3_index_type fua3_alive3(fua3_alive4, fua3_indexes[cluePosition3]); //hit
				fbm2_index_type fua2_alive3(fua2_alive4, fua2_indexes[cluePosition3]); //hit
				fbm1_index_type fua1_alive3(fua1_alive4, fua1_indexes[cluePosition3]); //hit
				dead_clues_type setClues3(setClues4);
				setClues3 |= posMask3;
				fastIterateLevel2(deadClues3, setClues3, fua1_alive3, fua2_alive3, fua3_alive3);
			}
			deadClues3 |= posMask3;
}
	}
	else {
		//iterate over all non-dead clues
		printf("Expanding 4 clues\n"); //debug
		checkPuzzle(4, setClues4, deadClues4);
	}
}
void fastClueIterator::fastIterateLevel4(const dead_clues_type & deadClues5, const dead_clues_type &setClues5,
		const fbm1_index_type fua1_alive5, const fbm2_index_type & fua2_alive5, const fbm3_index_type & fua3_alive5,
		const fbm4_index_type & fua4_alive5, const fbm5_index_type & fua5_alive5) {
	int uaIndex5 = fua1_alive5.getMinIndex();
	if(uaIndex5 != INT_MAX) {
		dead_clues_type deadClues4(deadClues5);
		const uset &u = fUsets[uaIndex5];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition4 = u.positions[i];
			const bm128 posMask4(bitSet[cluePosition4]);
			if(posMask4.isSubsetOf(deadClues5))
				continue;
			s4++;
			if(fua5_alive5.isSubsetOf(fua5_indexes[cluePosition4])) {
				d4++;
				fbm4_index_type fua4_alive4(fua4_alive5, fua4_indexes[cluePosition4]); //hit
				fbm3_index_type fua3_alive4(fua3_alive5, fua3_indexes[cluePosition4]); //hit
				fbm2_index_type fua2_alive4(fua2_alive5, fua2_indexes[cluePosition4]); //hit
				fbm1_index_type fua1_alive4(fua1_alive5, fua1_indexes[cluePosition4]); //hit
				dead_clues_type setClues4(setClues5);
				setClues4 |= posMask4;
				fastIterateLevel3(deadClues4, setClues4, fua1_alive4, fua2_alive4, fua3_alive4, fua4_alive4);
			}
			deadClues4 |= posMask4;
		}
	}
	else {
		printf("UA exhausted after placing clue number 5\n");
		//iterate over all non-dead clues
		//printf("Expanding 5 clues\n"); //debug
		//checkPuzzle(5, setClues5, deadClues5);
	}
}
void fastClueIterator::fastIterateLevel5(const dead_clues_type & deadClues6, const dead_clues_type &setClues6,
		const fbm1_index_type & fua1_alive6, const fbm2_index_type & fua2_alive6, const fbm3_index_type & fua3_alive6,
		const fbm4_index_type & fua4_alive6, const fbm5_index_type & fua5_alive6, const fbm6_index_type & fua6_alive6) {
	//passedFinalUA_ptr = passedFinalUA;
	//passedFinalUAIncomplete_ptr = passedFinalUAIncomplete;
	int uaIndex6 = fua1_alive6.getMinIndex();
	if(uaIndex6 != INT_MAX) {
		dead_clues_type deadClues5(deadClues6);
		const uset &u = fUsets[uaIndex6];
		for(unsigned int i = 0; i < u.nbits; i++) {
			int cluePosition5 = u.positions[i];
			const bm128 posMask5(bitSet[cluePosition5]);
			if(posMask5.isSubsetOf(deadClues6))
				continue;
			s5++;
			if(fua6_alive6.isSubsetOf(fua6_indexes[cluePosition5])) {
				d5++;
				fbm5_index_type fua5_alive5(fua5_alive6, fua5_indexes[cluePosition5]); //hit
				fbm4_index_type fua4_alive5(fua4_alive6, fua4_indexes[cluePosition5]); //hit
				fbm3_index_type fua3_alive5(fua3_alive6, fua3_indexes[cluePosition5]); //hit
				fbm2_index_type fua2_alive5(fua2_alive6, fua2_indexes[cluePosition5]); //hit
				fbm1_index_type fua1_alive5(fua1_alive6, fua1_indexes[cluePosition5]); //hit
				dead_clues_type setClues5(setClues6);
				setClues5 |= posMask5;
				fastIterateLevel4(deadClues5, setClues5,
						fua1_alive5, fua2_alive5, fua3_alive5,
						fua4_alive5, fua5_alive5);
			}
			deadClues5 |= posMask5;
		}
		clueNumber = 6;
	}
	else {
		printf("UA exhausted after placing clue number 5\n");
	}
//	for(dead_clues_type * i = passedFinalUA; i < passedFinalUA_ptr; i++) {
//		solvePuzzle(*i);
//	}
//	passedFinalUA_ptr = passedFinalUA;

	//	for(incomplete_puzzle_t * i = passedFinalUAIncomplete; i < passedFinalUAIncomplete_ptr; i++) {
//		passedIncompleteBM.emplace(*i);
//	}
//	passedFinalUAIncomplete_ptr = passedFinalUAIncomplete;
}
void fastClueIterator::fastIterateLevel9to6(const dead_clues_type &deadClues_old, const dead_clues_type &setClues_old,
		const fbm1_index_type &fua1_alive_old, const fbm2_index_type &fua2_alive_old, const fbm3_index_type &fua3_alive_old,
		const fbm4_index_type &fua4_alive_old, const fbm5_index_type &fua5_alive_old, const fbm6_index_type &fua6_alive_old) {
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
			dead_clues_type setClues_new(setClues_old);
			setClues_new.setBit(cluePosition);
			if(clueNumber == 6) {
				fastIterateLevel5(deadClues_new, setClues_new,
						fua1_alive_new, fua2_alive_new, fua3_alive_new,
						fua4_alive_new, fua5_alive_new, fua6_alive_new);
			}
			else {
				fastIterateLevel9to6(deadClues_new, setClues_new,
						fua1_alive_new, fua2_alive_new, fua3_alive_new,
						fua4_alive_new, fua5_alive_new, fua6_alive_new);
			}
			deadClues_new.setBit(cluePosition);
		}
		clueNumber++;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
	}
}
void fastClueIterator::fastIterateLevel10(const dead_clues_type &deadClues11, const dead_clues_type &setClues11,
		const bm1_index_type &ua1_alive11, const bm2_index_type &ua2_alive11, const fbm3_index_type &fua3_alive11,
		const fbm4_index_type &fua4_alive11, const fbm5_index_type &fua5_alive11, const fbm6_index_type &fua6_alive11) {
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
//			fbm2_index_type fua2_alive10(fua2_alive11, fua2_indexes[cluePosition]); //hit
			bm2_index_type ua2_alive10(ua2_alive11, ua2_indexes[cluePosition]); //hit
			fua2ActualSize = ua2_alive10.copyAlive(ua2, fUa2, fbm2_index_type::maxSize, deadClues10); //extract
			fbm2_index_type fua2_alive10(fUa2, fua2ActualSize, fua2_indexes); //rebuild
			//printf("fua2ActualSize=%d\n", fua2ActualSize);
			bm1_index_type ua1_alive10(ua1_alive11, ua1_indexes[cluePosition]); //hit
			fuaActualSize = ua1_alive10.copyAlive(ua, fUa, sizeof(fUa)/sizeof(fUa[0]), deadClues10); //extract
			//printf("fuaActualSize=%d", fuaActualSize);
			//std::sort(fUa, fUa + fuaActualSize, sizedUset::isSmaller);
			//std::partial_sort(fUa, fUa + fbm1_index_type::maxSize, fUa + fuaActualSize, sizedUset::isSmaller);
			std::partial_sort(fUa, fUa + fbm1_index_type::maxSize, fUa + fuaActualSize);
			//fuaActualSize = (std::unique(fUa, fUa + fuaActualSize) - &fUa[0]);
			//printf("\t%d\n", fuaActualSize);
			for(int n = 0; n < fuaActualSize; n++) {
				fUsets[n] = fUa[n];
				fUsets[n] &= maskLSB[81];
				fUsets[n].positionsByBitmap();
			}
			fbm1_index_type fua1_alive10(fUa, fuaActualSize, fua1_indexes); //rebuild
			dead_clues_type setClues10(setClues11);
			setClues10.setBit(cluePosition);
			fastIterateLevel9to6(deadClues10, setClues10,
					fua1_alive10, fua2_alive10, fua3_alive10,
					fua4_alive10, fua5_alive10, fua6_alive10);
			deadClues10.setBit(cluePosition);
		}
		//clueNumber = 11;
	}
	else {
		printf("UA exhausted after placing clue number 10\n");
	}
	//solve the cached puzzles
	for(dead_clues_type * i = passedFinalUA; i < passedFinalUA_ptr; i++) {
		solvePuzzle(*i);
	}
	passedFinalUA_ptr = passedFinalUA;
	//expand and solve the cached partial puzzles
	while(! passedIncompleteBM.empty()) {
		const incomplete_puzzle_t &incompl = passedIncompleteBM.front();
		//expandPuzzle(incompl.nPositions, incompl.setClues, incompl.deadClues); //expand to passedBM
		expandPuzzle(nClues - incompl.setClues.popcount_128(), incompl.setClues, incompl.deadClues); //expand to passedBM
		passedIncompleteBM.pop();
	}
//	//	maxQ=71144 for lowUA1 grid
//	if(maxQ < passedBM.size()) maxQ = passedBM.size();
//	//solve the cached puzzles
//	while(! passedBM.empty()) {
//		//dead_clues_type clues = passedBM.front();
//		solvePuzzle(passedBM.front());
//		passedBM.pop();
//	}
}
void fastClueIterator::fastIterateLevel11(const dead_clues_type &deadClues12, const dead_clues_type &setClues12,
		const bm1_index_type &ua1_alive12, const bm2_index_type &ua2_alive12, const bm3_index_type &ua3_alive12,
		const fbm4_index_type &fua4_alive12, const fbm5_index_type &fua5_alive12, const fbm6_index_type &fua6_alive12) {
	int uaIndex12 = ua1_alive12.getMinIndex();
	if(uaIndex12 != INT_MAX) {
		//clueNumber = 11;
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
//			bm2_index_type ua2_alive11(ua2_alive12, ua2_indexes[cluePosition]); //hit
//			fua2ActualSize = ua2_alive11.copyAlive(ua2, fUa2, fbm2_index_type::maxSize, deadClues11); //extract
//			fbm2_index_type fua2_alive11(fUa2, fua2ActualSize, fua2_indexes); //rebuild
			bm1_index_type ua1_alive11(ua1_alive12, ua1_indexes[cluePosition]); //hit
			dead_clues_type setClues11(setClues12);
			setClues11.setBit(cluePosition);
			fastIterateLevel10(deadClues11, setClues11,
					ua1_alive11, ua2_alive11, fua3_alive11,
					fua4_alive11, fua5_alive11, fua6_alive11);
			deadClues11.setBit(cluePosition);
		}
		//clueNumber = 12;
	}
	else {
		printf("UA exhausted after placing clue number 11\n");
	}
}
void fastClueIterator::fastIterateLevel12(const dead_clues_type &deadClues13, const dead_clues_type &setClues13,
		const bm1_index_type &ua1_alive13, const bm2_index_type &ua2_alive13, const bm3_index_type &ua3_alive13,
		const bm4_index_type &ua4_alive13, const fbm5_index_type &fua5_alive13, const fbm6_index_type &fua6_alive13) {
	int uaIndex13 = ua1_alive13.getMinIndex();
	if(uaIndex13 != INT_MAX) {
		//clueNumber = 12;
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
			dead_clues_type setClues12(setClues13);
			setClues12.setBit(cluePosition);
			fastIterateLevel11(deadClues12, setClues12,
					ua1_alive12, ua2_alive12, ua3_alive12,
					fua4_alive12, fua5_alive12, fua6_alive12);
			deadClues12.setBit(cluePosition);
		}
		//clueNumber = 13;
	}
	else {
		printf("UA exhausted after placing clue number 12\n");
	}
}
void fastClueIterator::fastIterateLevel13(const dead_clues_type &deadClues14, const dead_clues_type &setClues14,
		const bm1_index_type &ua1_alive14, const bm2_index_type &ua2_alive14, const bm3_index_type &ua3_alive14,
		const bm4_index_type &ua4_alive14, const bm5_index_type &ua5_alive14, const bm6_index_type &ua6_alive14) {
	int uaIndex14 = ua1_alive14.getMinIndex();
	if(uaIndex14 != INT_MAX) {
		//clueNumber = 13;
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
			dead_clues_type setClues13(setClues14);
			setClues13.setBit(cluePosition);
			fastIterateLevel12(deadClues13, setClues13,
					ua1_alive13, ua2_alive13, ua3_alive13,
					ua4_alive13, fua5_alive13, fua6_alive13);
			deadClues13.setBit(cluePosition);
		}
		clueNumber = 14;
	}
	else {
		printf("UA exhausted after placing clue number 13\n");
	}
}
void fastClueIterator::fastIterateLevel(const dead_clues_type &deadClues_old, const dead_clues_type &setClues_old,
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
			dead_clues_type setClues_new(setClues_old);
			setClues_new.setBit(cluePosition);
			if(clueNumber == 14) {
			fastIterateLevel13(deadClues_new, setClues_new,
					ua1_alive_new, ua2_alive_new, ua3_alive_new,
					ua4_alive_new, ua5_alive_new, ua6_alive_new);
			}
			else {
				fastIterateLevel(deadClues_new, setClues_new,
						ua1_alive_new, ua2_alive_new, ua3_alive_new,
						ua4_alive_new, ua5_alive_new, ua6_alive_new);
			}
			deadClues_new.setBit(cluePosition);
		}
		clueNumber++;
	}
	else {
		printf("UA exhausted after placing clue number %d\n", clueNumber);
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

//	//inject an artificial UA6 for band 3
//	sizedUset tt(maskLSB[27]);
//	tt.setSize();
//	ua6[0] = tt;
//	ua6ActualSize = 1;

	int st6 = starter.starter6;

	while(st6 >= 10) { //dynamiccaly adjust starter down
		ua6ActualSize = 0;
		for ( int s1 = st6;  s1 < uaActualSize;  s1++ ) {
			for (int s2 = st6 - 1; s2 < s1; s2++) {
				sizedUset tt = ua[s1];
				if(tt.join(ua[s2])) {
					for (int s3 = st6 - 2; s3 < s2; s3++) {
						sizedUset ttt = tt;
						if (ttt.join(ua[s3])) {
							for (int s4 = st6 - 3; s4 < s3; s4++) {
								sizedUset tttt = ttt;
								if (tttt.join(ua[s4])) {
									for (int s5 = st6 - 4; s5 < s4; s5++) {
										sizedUset ttttt = tttt;
										if (ttttt.join(ua[s5])) {
											for (int s6 = st6 - 5; s6 < s5; s6++) {
												sizedUset tttttt = ttttt;
												if (tttttt.join(ua[s6])) {
//													if(tttttt.getSize() > 65)
//														goto next_s6;
													for (int s = 0; s < st6 - 5; s++) {
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
		if(true || 4 * ua6ActualSize > bm6_index_type::maxSize) { //one third of the buffer
			break;
		}
		st6--;
	}
	ua6composed:;
	//ua6_type::bm128ToIndex(ua6, ua6ActualSize, state[clueNumber].setMask6, hittingMasks6);
	ua6_alive_initial = bm6_index_type(ua6, ua6ActualSize, ua6_indexes); //build
}

fastClueIterator::fastClueIterator(grid &g) :
		passedFinalUA_ptr(passedFinalUA),
		fuaActualSize(0), fua2ActualSize(0), fua3ActualSize(0), fua4ActualSize(
				0), fua5ActualSize(0), fua6ActualSize(0), uaActualSize(0), ua2ActualSize(
				0), ua3ActualSize(0), ua4ActualSize(0), ua5ActualSize(0), ua6ActualSize(
				0), clueNumber(0), nClues(opt.scanOpt->nClues), nPuzzles(0), nChecked(
				0), g(g) {
	//nClues = 16;
}

void fastClueIterator::checkPuzzle(int clueNumber, const dead_clues_type &setClues, const dead_clues_type & dc) {
	passedIncompleteBM.emplace(setClues, dc, clueNumber);

//	incomplete_puzzle_t incompl;
//	incompl.setClues = setClues;
//	incompl.deadClues = dc;
//	incompl.nPositions = clueNumber;
//	passedIncompleteBM.emplace(incompl);

	//passedIncompleteBM.push(incompl);
//	passedFinalUAIncomplete_ptr->setClues = setClues;
//	passedFinalUAIncomplete_ptr->deadClues = dc;
//	passedFinalUAIncomplete_ptr->nPositions = clueNumber;
//	passedFinalUAIncomplete_ptr++;
}

void fastClueIterator::expandPuzzle(int clueNumber, const dead_clues_type &setClues, const dead_clues_type & dc, int startPos) {
	if(clueNumber == 0) {
		//checkPuzzle(setClues);
		solvePuzzle(setClues);
	}
	else {
		dead_clues_type skipClues(setClues);
		skipClues |= dc;
		for(int i = startPos; i < 81; i++) {
			bm128 posMask = bitSet[i];
			if(posMask.isSubsetOf(skipClues)) {
				continue;
			}
			dead_clues_type setClues_new(setClues);
			setClues_new |= posMask;
			expandPuzzle(clueNumber - 1, setClues_new, dc, i + 1);
		}
	}
}

//void fastClueIterator::checkPuzzle(const dead_clues_type &setClues) {
//	//passedBM.push(setClues);
//	passedBM.emplace(setClues);
//}
void fastClueIterator::solvePuzzle(const dead_clues_type &setClues) {
	//todo: check against the long list of UA
	char clues[88];
	for(int i = 0; i < 81; i++) {
		if(setClues.isBitSet(i)) {	//given
			clues[i] = g.digits[i];
		}
		else {
			clues[i] = 0;
		}
	}
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
	setClues_initial.clear(); //no partial givens

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
	starter = stFamily[g.usetsBySize.distributionBySize[4] < 10 && g.usetsBySize.distributionBySize[6] >= 30 ? 3 : 0]; //0 works best for random grid, 8 for 17s

	buildComposites();
	//some info for debugging/optimization
	//printf("\t%d clues\n", nClues);
	printf("ua =%d\t", uaActualSize);
	printf("ua2=%d\t", ua2ActualSize);
	printf("ua3=%d\t", ua3ActualSize);
	printf("ua4=%d\t", ua4ActualSize);
	printf("ua5=%d\t", ua5ActualSize);
	printf("ua6=%d\n", ua6ActualSize);

	fastIterateLevel(deadClues_initial, setClues_initial,
		ua1_alive_initial, ua2_alive_initial, ua3_alive_initial,
		ua4_alive_initial, ua5_alive_initial, ua6_alive_initial);
	//printf("\tpuz=%d\tch=%d", nPuzzles, nChecked);
	//s0=100*d0/s0;//d0/=1000000;
	//s1=100*d1/s1;d1/=1000000;
	//s2=100*d2/s2;d2/=1000000;
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
