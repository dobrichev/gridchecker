#ifndef MINIMIZER_H_INCLUDED

#define MINIMIZER_H_INCLUDED

#include "uset.h"
#include "grid.h"
#include "solver.h"
#include "ch81.h"

#define USET_PAGES (32)
//#define USET_PAGES (64)
//#define USET_PAGES (128)
#define MAX_USETS (USET_PAGES * 128)

extern int xskipped[20];

struct maskLong {
	bm128 pages[USET_PAGES];
};

struct minimizerState {
	maskLong setMask;
	bm128 clues;
	bm128 deadClues;
	bm128 redundantCandidates;
	int uaIndex;
	//void dump() { //debug
	//	char buf[256];
	//	clues.toMask81(buf);
	//	printf("\nuaIndex=%d\n", uaIndex);
	//	printf("    Clues=%81.81s\n", buf);
	//	deadClues.toMask81(buf);
	//	printf("deadClues=%81.81s\n", buf);
	//}
};
struct minimizer {
	uset usets[MAX_USETS];
	maskLong hittingMasks[81];
	minimizerState state[81];
	grid g;
	int nFixedGivens;
	int numUsets;
	int numUsetPages;
	bm128 fg, fng; //forced givens and non-givens
	ch81 puz, fixedGivens;
	int NOINLINE init(const char * const givens);
	int NOINLINE initFast(const minimizer & parent, int stateIndex);
	void NOINLINE enumerateState(int stateIndex);
	void combineFloating();
	static inline size_t nextPerm(const size_t prev) {
		//http://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
		int lo = prev & ~(prev - 1);	// lowest one bit
		int lz = (prev + lo) & ~prev;	// lowest zero bit above lo
		size_t next = prev | lz;		// add lz to the set
		next &= ~(lz - 1);				// reset bits below lz
		next |= (lz / lo / 2) - 1;		// put back right number of bits at end
		return next;
	}
	static NOINLINE int isIrreducible(char *puz, const bm128 &possiblyRedundant) {
		for(int red = 0; red < 81; red++) {
			if(puz[red] == 0)
				continue; //not a given
			xskipped[11]++;
			if(!possiblyRedundant.isBitSet(red)) {
				xskipped[0]++;
				continue; //not a candidate for redundancy
			}
			int redValue = puz[red];
			puz[red] = 0;
			if(solve(puz, 2) == 1) {
				//given at position red is redundant
				puz[red] = redValue;
				return 0;
			}
			puz[red] = redValue;
		}
		return 1;
	}
};

struct checkAndMark {
	static const int bufSize = 6000;
	size_t masks[bufSize];
	bool unique[bufSize];
	//std::vector<bool> &invalids;
	bool *invalids;
	const uset &fl;
	const ch81 &maxPuz;
	const size_t size;
	int nElem;
	//checkAndMark(std::vector<bool> &inv, const uset &f, const ch81 &mPuz, size_t sz) : invalids(inv), fl(f), nElem(0), maxPuz(mPuz), size(sz) {}
	checkAndMark(bool *inv, const uset &f, const ch81 &mPuz, size_t sz) : invalids(inv), fl(f), maxPuz(mPuz), size(sz), nElem(0) {}
	void add(size_t mask) {
		masks[nElem++] = mask;
		if(nElem == bufSize) {
			finalize();
		}
	}
	void NOINLINE finalize() {
		if(nElem == 0)
			return;
#ifdef _OPENMP
//#pragma omp parallel for schedule(static, 1)
#endif //_OPENMP
		for(int n = 0; n < nElem; n++) {
			//compose a puzzle with initial givens with those from the mask excluded
			ch81 p = maxPuz; //structure copy
			size_t mask = masks[n];
			for(unsigned int theBit = 0; theBit < fl.nbits; theBit++) {
				if(mask & (1 << theBit)) {
					//clear this floating clue
					p.chars[fl.positions[theBit]] = 0;
				}
			}
			unique[n] = (1 == solve(p.chars, 2));
		}
		for(int n = 0; n < nElem; n++) {
			if(unique[n]) {
				continue;
			}
			//Multiple solutions, mark all supersets as invalid
			//http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=bitManipulation
			size_t nmask = ~masks[n];
			//for(size_t i = nmask, ni = ~i; ni < size; i = (i - 1) & nmask, ni = ~i) {
			for(size_t ni = ~nmask; ni < size; ni = ~(((~ni) - 1) & nmask)) {
				if(invalids[ni]) {
					continue;
				}
				invalids[ni] = true;
			}
		}
		nElem = 0;
	}
};
#endif //MINIMIZER_H_INCLUDED
