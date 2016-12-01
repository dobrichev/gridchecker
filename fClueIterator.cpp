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
//	void searchForMoreUA(grid &g) {
//		g.usetsBySize.clear();
//		g.setBM();
//		unsigned char unknowns[81];
//		for(int i = 0; i < clique[mcn - 1].size(); i++) {
//			sizedUset u(clique[mcn - 1][i]);
//			int n = 0;
//			for(int j = 0; j < 81; j++) {
//				if(!u.isBitSet(j)) {
//					unknowns[n++] = j;
//				}
//			}
//			g.findUaBySolving(unknowns, n);
//			if(!g.usetsBySize.empty()) {
//				ch81 txt;
//				uset uu(u);
//				uu.positionsByBitmap();
//				g.ua2puzzle(uu, txt.chars);
//				printf("/n%81.81s\t%d\n", txt.chars, u.getSize());
//				for(usetListBySize::const_iterator p = g.usetsBySize.begin(); p != g.usetsBySize.end(); p++) {
//					uu = *p;
//					g.ua2puzzle(uu, txt.chars);
//					printf("%81.81s\tnewUA\t%d\n", txt.chars, u.getSize());
//				}
//				g.usetsBySize.clear();
//			}
//		}
//	}
	int populateLevel(int level) { //expects previous levels are already done
		sizedUsetList &singles = clique[0];
		sizedUsetList &t = clique[level];
		t.clear();
		int s0 = (int)singles.size();
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
//		//check whether some clique is a subset of another clique (of the same level)
//		for(sizedUsetList::iterator p = t.begin(); p != t.end(); p++) {
//			sizedUset u1(*p);
//			u1.setSize(0);
//			sizedUsetList::iterator pp = p;
//			pp++;
//			for(; pp != t.end();) {
//				if(p->isSubsetOf(*pp)) {
//					sizedUsetList::iterator e = pp;
//					pp++;
//					t.erase(e);
//				}
//				else {
//					pp++;
//				}
//			}
//		}

//		for(sizedUsetList::const_iterator p = t.begin(); p != t.end(); p++) {
//			target.push_back(*p);
//		}
		return 0;
	}
};

struct hittingClue {
	int offset;
	int mask;
	void setClue(int pos) {
		offset = (int)(pos / (8 * sizeof(int)));
		mask = ((unsigned int)1) << (pos % (8 * sizeof(int)));
	}
	int hits(const sizedUset &u) const {
		int const frame = (reinterpret_cast< int const * >(&u))[offset];
		return frame & mask;
	}
	//void reduce(sizedUset &u) const {
	//	int &frame = (reinterpret_cast< int * >(&u))[offset];
	//	if(frame & mask) {
	//		frame ^= mask;
	//		u.decreaseSize();
	//	}
	//}
};

struct fastState {
	bm128 deadClues;
	int nPositions;
	int positions[81];
	sizedUsetVector sizedUsetV;
};

class fastClueIterator {
private:
	fastClueIterator();
public:
	BitMask768 hittingMasks[81];
	int nClues;
	grid &g;
	unsigned int nPuzzles;
	unsigned int nChecked;
	int clueNumber; //nClues - 1 .. 0
	fastState state[81];
	char clues[81];
	reusableUA cliques;

	fastClueIterator(grid &g);
	void checkPuzzle(bm128 &dc, int startPos = 0);
	void iterate();
	void iterateLevel();
	void findBestUA();
};

fastClueIterator::fastClueIterator(grid &g) : g(g) {
	nPuzzles = 0;
	nChecked = 0;
	nClues = opt.scanOpt->nClues;
	//numUaSizeLimit = 500;
	//numUaTotalLimit = 1000;
	//for(int i = 0; i < 81; i++) {
	//	state[i].maxPositionLimitsByUA = 81;
	//}
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
}

void fastClueIterator::findBestUA() {
	//best UA is the one with
	// 1) smallest size
	// 2) most disjoint short UA
	// 3) most cells that "join" other short UA
	//i.e. any "dead" clue causes more disjoint short UA
	sizedUsetVector &s = state[nClues].sizedUsetV; //the UA vector to deal with
	int smallestUaSize = s[0].getSize();
	int numUA = (int)s.size();
	int bestDJnum = 0;
	int bestJnum = 0;
	int bestIndex = 0;
	for(int i = 0; i < numUA && s[i].getSize() == smallestUaSize; i++) {
		sizedUset &u = s[i];
		int curJnum = 0;
		int curDJnum = 0;
		for(int n1 = 0; n1 < numUA && s[n1].getSize() <= 9; n1++) {
			sizedUset &u1 = s[n1];
			if(u1.isDisjoint(u)) {
				curJnum++;
				continue;
			}
			for(int n2 = n1 + 1; n2 < numUA && s[n2].getSize() <= 9; n2++) {
				sizedUset &u2 = s[n2];
				if(u2.isDisjoint(u)) {
					curJnum++;
					continue;
				}
				if(!u2.isDisjoint(u1)) {
					sizedUset x(u2.bitmap128);
					x &= u1.bitmap128;
					if(x.isSubsetOf(u)) {
						x.setSize();
						if(x.getSize() == 1) {
							curJnum++;
						}
					}
				}
			}
			if(bestDJnum < curDJnum || (bestDJnum == curDJnum && bestJnum < curJnum)) {
				bestIndex = i;
				bestDJnum = curDJnum;
				bestJnum = curJnum;
			}
		}
	}
	if(bestIndex) {
		ch81 p0;
		s[0].toMask81(p0.chars);
		printf("%81.81s\t%d <<0\n", p0.chars, s[0].getSize());
		s[bestIndex].toMask81(p0.chars);
		printf("%81.81s\t%d <<%d\n", p0.chars, s[0].getSize(), bestIndex);
		//swap the best UA with first UA
		sizedUset top = s[bestIndex];
		s[bestIndex] = s[0];
		s[0] = top;
	}
}

unsigned long long d0, d1, d2, d3, d4; //debug

void fastClueIterator::iterateLevel() {
	fastState &oldState = state[clueNumber];
	clueNumber--;
	fastState &newState = state[clueNumber];
	newState.deadClues = oldState.deadClues;
	sizedUsetVector &newUA = newState.sizedUsetV;
	//hittingClue deadClue; //last dead clue
	int numOldUA = (int)oldState.sizedUsetV.size();
	for(int posIndex = 0; posIndex < oldState.nPositions; posIndex++) {
		int cluePosition = oldState.positions[posIndex];
		newUA.clear();
		clues[cluePosition] = g.digits[cluePosition];
		hittingClue hit;
		hit.setClue(cluePosition);
		int minNewUAsize = 100;
		int minNewUAindex = 0;
		for(int i = 0; i < numOldUA; i++) {
			sizedUset u(oldState.sizedUsetV[i]);
			if(hit.hits(u)) {
				continue; //skip UA hit by the current clue
			}
			if(clueNumber == 0) {
				//there is at least one unhit UA after all clues set
				d0++;
				goto backtrack;
			}
			if(posIndex == 0 || u.isDisjoint(newState.deadClues)) {
				//no dead clues
				newUA.push_back(u);
				int size = u.getSize();
				if(minNewUAsize > size) {
					minNewUAsize = size;
					minNewUAindex = (int)newUA.size() - 1;
				}
			}
			else {
				//remove the dead clue from previous iteration
				u.clearBits(newState.deadClues);
				u.setSize();
				int size = u.getSize();
				if(size) { //non-empty UA
					newUA.push_back(u);
					if(minNewUAsize > size) {
						minNewUAsize = size;
						minNewUAindex = (int)newUA.size() - 1;
					}
				}
				else {
					//zero chance to hit this empty UA later
					//this happens verey rare but is cheap to idenify
					printf("%d", clueNumber);
					goto backtrack;
				}
			}
		}
		//if(newUA.empty()) {
		if(minNewUAsize == 100) {
			//all UA hit
			checkPuzzle(state[clueNumber].deadClues);
			goto backtrack;
		}
		else {
			if(minNewUAindex) {
				//place the shortest UA at top
				sizedUset minUA(newUA[minNewUAindex]);
				newUA[minNewUAindex] = newUA[0];
				newUA[0] = minUA;
			}
			//if(clueNumber < cliques.mcn && clueNumber > 5) { //there is some chance unhit "static" clique to exist
			//	bm128 done;
			//	done.clear();
			//	for(int i = 0; i < 81; i++) {
			//		if(clues[i]) {
			//			done.setBit(i);
			//		}
			//	}
			//	sizedUsetVector &cl = cliques.clique[clueNumber];
			//	int clSize = cl.size();
			//	for(int i = 0; i < clSize; i++) {
			//		if(cl[i].isDisjoint(done)) {
			//			//if(clueNumber > 5) {
			//			//	printf("%d", clueNumber);
			//			//}
			//			goto backtrack;
			//		}
			//	}
			//}

			int numUA = (int)newUA.size();
			//perform (partial) sort
			if(clueNumber > 5) {
				//int n = numUA;
				////if(clueNumber < 12 && numUA >= 20) {
				//if(clueNumber < 12 && numUA >= 80) {
				//	n = 80;
				//}
				////if(n <= minNewUAindex) { //ensure the sort is sufficiently depth so that the minimal UA become first 
				////	n = minNewUAindex + 1;
				////}
				//if(minNewUAindex < n) {
				//	//the shortest UA lies within the sorted scope and therefore will be placed on top
				//	minNewUAindex = 0;
				//}
				//std::sort(newUA.begin(), newUA.begin() + n);
				std::sort(newUA.begin(), newUA.end());
				//remove duplicates
				numUA = (int)(std::unique(newUA.begin(), newUA.end()) - newUA.begin());
				newUA.resize(numUA);
			}

			//if(numOldUA > 128 && numUA <= 128) {
			//	//ready to switch to indexed access??
			//	//printf("%d\n", clueNumber);
			//	nPuzzles++;
			//	goto backtrack;
			//}
			if(clueNumber == 1) {
				//skip if UA of level 2 exists
				for(int i1 = 0; i1 < numUA - 1; i1++) {
					bm128 u1(newUA[i1].bitmap128);
					u1 &= maskLSB[81];
					for(int i2 = i1 + 1; i2 < numUA; i2++) {
						if(u1.isDisjoint(newUA[i2])) {
							//printf("dj2\n");
							d1++;
							goto backtrack;
						}
					}
				}
			}
			if(clueNumber == 2) {
				//skip if UA of level 3 exists
				for(int i1 = 0; i1 < numUA - 2; i1++) {
					bm128 u1(newUA[i1].bitmap128);
					u1 &= maskLSB[81];
					for(int i2 = i1 + 1; i2 < numUA - 1; i2++) {
						if(u1.isDisjoint(newUA[i2])) {
							bm128 u2(newUA[i2].bitmap128);
							u2 &= maskLSB[81];
							u2 |= u1;
							for(int i3 = i2 + 1; i3 < numUA; i3++) {
								if(u2.isDisjoint(newUA[i3])) {
									//printf("dj3\n");
									d2++;
									goto backtrack;
								}
							}
						}
					}
				}
			}
			if(clueNumber == 3) { //for sf grid this is called 628 times with 128+ UA, for most others - 0 times
				//skip if UA of level 4 exists
				for(int i1 = 0; i1 < numUA - 3; i1++) {
					bm128 u1(newUA[i1].bitmap128);
					u1 &= maskLSB[81];
					for(int i2 = i1 + 1; i2 < numUA - 2; i2++) {
						if(u1.isDisjoint(newUA[i2])) {
							bm128 u2(newUA[i2].bitmap128);
							u2 &= maskLSB[81];
							u2 |= u1;
							for(int i3 = i2 + 1; i3 < numUA - 1; i3++) {
								if(u2.isDisjoint(newUA[i3])) {
									bm128 u3(newUA[i3].bitmap128);
									u3 &= maskLSB[81];
									u3 |= u2;
									for(int i4 = i3 + 1; i4 < numUA; i4++) {
										if(u3.isDisjoint(newUA[i4])) {
											//printf("4");
											d3++;
											goto backtrack;
										}
									}
								}
							}
						}
					}
				}
				//printf(".");
			}
			if(0 && clueNumber == 4) { //14x17 = 844 seconds processing and 707 seconds skipping this step.
				//skip if UA of level 5 exists
				for(int i1 = 0; i1 < numUA - 4; i1++) {
					if(newUA[i1].getSize() > 6) //6
					//if(newUA[i1].getSize() > minNewUAsize + 1)
						continue;
					bm128 u1(newUA[i1].bitmap128);
					u1 &= maskLSB[81];
					for(int i2 = i1 + 1; i2 < numUA - 3; i2++) {
						if(newUA[i2].getSize() > 6)
							continue;
						if(u1.isDisjoint(newUA[i2])) {
							bm128 u2(newUA[i2].bitmap128);
							u2 &= maskLSB[81];
							u2 |= u1;
							for(int i3 = i2 + 1; i3 < numUA - 2; i3++) {
								if(newUA[i3].getSize() > 9)
									continue;
								if(u2.isDisjoint(newUA[i3])) {
									bm128 u3(newUA[i3].bitmap128);
									u3 &= maskLSB[81];
									u3 |= u2;
									for(int i4 = i3 + 1; i4 < numUA - 1; i4++) {
										if(u3.isDisjoint(newUA[i4])) {
											bm128 u4(newUA[i4].bitmap128);
											u4 &= maskLSB[81];
											u4 |= u3;
											for(int i5 = i4 + 1; i5 < numUA; i5++) {
												if(u4.isDisjoint(newUA[i5])) {
													//printf("5");
													d4++;
													goto backtrack;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				//printf(".");
			}
			if(0 && clueNumber == 5) {
				//skip if UA of level 6 exists
				for(int i1 = 0; i1 < numUA - 5; i1++) {
					if(newUA[i1].getSize() > 4)
						continue;
					bm128 u1(newUA[i1].bitmap128);
					u1 &= maskLSB[81];
					for(int i2 = i1 + 1; i2 < numUA - 4; i2++) {
						if(newUA[i2].getSize() > 5)
							continue;
						if(u1.isDisjoint(newUA[i2])) {
							bm128 u2(newUA[i2].bitmap128);
							u2 &= maskLSB[81];
							u2 |= u1;
							for(int i3 = i2 + 1; i3 < numUA - 3; i3++) {
								if(newUA[i3].getSize() > 6) //8
									continue;
								if(u2.isDisjoint(newUA[i3])) {
									bm128 u3(newUA[i3].bitmap128);
									u3 &= maskLSB[81];
									u3 |= u2;
									for(int i4 = i3 + 1; i4 < numUA - 2; i4++) {
										if(newUA[i4].getSize() > 9)
											continue;
										if(u3.isDisjoint(newUA[i4])) {
											bm128 u4(newUA[i4].bitmap128);
											u4 &= maskLSB[81];
											u4 |= u3;
											for(int i5 = i4 + 1; i5 < numUA - 1; i5++) {
												if(u4.isDisjoint(newUA[i5])) {
													bm128 u5(newUA[i5].bitmap128);
													u5 &= maskLSB[81];
													u5 |= u4;
													for(int i6 = i5 + 1; i6 < numUA; i6++) {
														if(u5.isDisjoint(newUA[i6])) {
															goto backtrack;
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
				}
				//printf(".");
			}

			if(clueNumber == 12) {
				printf("."); //show progress
			}

			//prepare state for the next clue
			//uset u(newState.sizedUsets.begin()->bitmap128);
			//uset u(newUA[minNewUAindex].bitmap128);
			uset u(newUA[0].bitmap128);
			u &= maskLSB[81];
			u.positionsByBitmap();
			newState.nPositions = u.nbits;
			for(int i = 0; i < u.nbits; i++) {
				newState.positions[i] = u.positions[i];
			}

			if(numUA > 128) {
				//iterate recursively
				iterateLevel();
			}
			//switch to clue indexes (119394 times for 14x17 grid with 311 initial UA, 56120 for 20x17/289, 1892149 sf/371)
			std::sort(newUA.begin(), newUA.end());
			//remove duplicates
			numUA = (int)(std::unique(newUA.begin(), newUA.end()) - newUA.begin());

			cliques.clique[0].clear();
			for(int i = 0; i < numUA; i++) {
				//todo: skip the minNewUAindex UA
				cliques.clique[0].insert(newUA[i]);
			}
			if(cliques.populate(clueNumber)) { //this crashes for MostCanonical grid on 32-bit platform
				//mcn > clues available
				printf("x"); //debug
				goto backtrack; //241 for 14x17
			}
			//for(int i = 0; i < cliques.mcn; i++) {
			//	printf("cl(%d)\t%d\n", i, cliques.clique[i].size());
			//}

			nPuzzles++; //119153 = 119394 - 241 for 14x17
		}
backtrack:;
		clues[cluePosition] = 0;
		newState.deadClues.setBit(cluePosition);
	}
	clueNumber++;
}

void fastClueIterator::iterate() {
	//find all UA sets of size 4..12
	g.findUA12();
	usetListBySize &us = g.usetsBySize;
	printf("\t%d\n", (int)us.size());
	//debug: add all 4-digit UA
	//g.findUA4digits();
	//printf("\t%d\n", us.size());

	//perform some initial filtering of the UA based on some criteria (size)
	//todo

	//init the top of the stack
	clueNumber = nClues;
	fastState &s = state[nClues];
	const int numUaToSkip = 0; //Gary McGuire skips the shortest 40 UA
	int curUA = 0;
	for(usetListBySize::const_iterator p = us.begin(); p != us.end(); p++, curUA++) {
		if(curUA < numUaToSkip) continue;
		sizedUset su;
		su.bitmap128 = p->bitmap128; //don't calculate the size
		su.setSize(p->nbits);
		s.sizedUsetV.push_back(su);
		//if(su.getSize() >= 6)
			cliques.clique[0].insert(su);
	}
	for(int i = 0; i < 81; i++) {
		clues[i] = 0;
	}

	//findBestUA();

	s.deadClues.clear();
	//always start from the first UA which is one of the shortest
	uset top;
	top.bitmap128 = s.sizedUsetV[0].bitmap128;
	top &= maskLSB[81];
	top.positionsByBitmap();
	s.nPositions = top.nbits;
	for(int i = 0; i < s.nPositions; i++) {
		s.positions[i] = top.positions[i];
	}

	cliques.populate(5); //this crashes for MostCanonical grid on 32-bit platform

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
		sum = 0;
		for(int j = 0; j < 81; j++) sum += population[j];
		for(int j = 0; j < 81; j++) population[j] /= (sum / 81 / 10);
		for(int j = 0; j < 81; j++) printf("%2.2d ", population[j]);
		printf("\n");
	}

//	//cliques.searchForMoreUA(g);
//	//how the top cliques are joined to each other?
//	int c[4];
//	for(int i = cliques.mcn - 2; i >= 4; i--) {
//		sizedUsetVector &v0 = cliques.clique[i];
//		//compare level to itself
//		c[0] = c[1] = c[2] = c[3] = 0;
//		for(int j1 = 0; j1 < v0.size() /*- 1*/; j1++) {
//			//printf("level %d\tua=%d\tsize=%d\n", i + 1, j1, v0[j1].getSize());
//			//uset u(v0[j1]);
//			//u.positionsByBitmap();
//			//ch81 z;
//			//u.toMask81(z.chars);
//			//printf("%81.81s\n", z.chars);
//			for(int j2 = j1 + 1; j2 < v0.size(); j2++) {
//				sizedUset common(v0[j2]);
//				//common.setSize(0);
//				common &= maskLSB[81];
//				common &= v0[j1];
//				int x = common.popcount_128();
//				if(x <= 3) {
//					c[x]++;
//				}
//			}
//		}
//		printf("joint at level %d+%d\t0=%d\t1=%d\t2=%d\t3=%d\n", i + 1, i + 1, c[0], c[1], c[2], c[3]);
//		//compare level to lower ones
//		for(int i1 = i - 1; i1 >= 4; i1--) {
//			sizedUsetVector &v1 = cliques.clique[i1];
//			c[0] = c[1] = c[2] = c[3] = 0;
//			for(int j1 = 0; j1 < v0.size(); j1++) {
//				for(int j2 = 0; j2 < v1.size(); j2++) {
//					sizedUset common(v1[j2]);
//					common.setSize(0);
//					common &= v0[j1];
//					int x = common.popcount_128();
//					if(x <= 3) {
//						c[x]++;
//					}
//				}
//			}
//			printf("joint at level %d+%d\t0=%d\t1=%d\t2=%d\t3=%d\n", i + 1, i1 + 1, c[0], c[1], c[2], c[3]);
//		}
//	}
//
//	//iterateLevel();
//	printf("puz=%d\tch=%d\n", nPuzzles, nChecked);
//	printf("%llu\t%llu\t%llu\t%llu\t%llu\n", d0, d1, d2, d3, d4); //14x17 grid = 5,889,056 33,028,056 114,316,605 124,680,124 43,565,492 =>2061.494 seconds.
}


extern int fastScan() {
	const char* fname = opt.scanOpt->gridFileName;
	char buf[3000];
	while(fgets(buf, sizeof(buf), stdin)) {
		printf("%81.81s", buf);
		grid g;
		g.fromString(buf);
		//g.findInitialUA();
		g.fname = fname;
		fastClueIterator ci(g);
		ci.iterate();
	}
	return 0;
}
