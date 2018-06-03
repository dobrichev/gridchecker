//uses skfrdll.lib, see details at http://code.google.com/p/skfr-sudoku-fast-rating/

//#define DLLEXPORT __declspec(dllexport)
//extern "C" DLLEXPORT int __stdcall ratePuzzle(char *ze, int * er, int * ep, int * ed, int * aig);
//extern "C" DLLEXPORT void __stdcall setMinMax(int mined,int maxed, int minep, int maxep, int miner, int maxer, unsigned int filt);
//extern "C" DLLEXPORT void __stdcall setParam(int o1, int delta, int os, int oq, int ot, int oexclude, int edcycles);
//extern "C" DLLEXPORT int __stdcall setTestMode(int ot, char * logFileName);
//extern "C" DLLEXPORT void __stdcall ratePuzzles(int nPuzzles, char *ze, int *er, int *ep, int *ed, int *aig, int *ir);

#if 1
#include "rate.h"

//skfr static lib exports
//struct puzzleToRate {
//	int er;
//	int ep;
//	int ed;
//	char p[81];
//};
//
//void rateManyPuzzles(int nPuzzles, puzzleToRate *p);

fskfr::fskfr() {
	count = 0;
}

void fskfr::skfrMultiER(const char *p, uint32_t *rate) {
	for(int i = 0; i < 81; i++) {
		puzzlesToRate[count].p[i] = p[i] + '0';
	}
	res[count] = rate;
	count++;
	if(count >= bufSize)
		skfrCommit();
}

void fskfr::skfrCommit() {
	//skfr::rateManyPuzzles(count, puzzlesToRate);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif //_OPENMP
	for(int i = 0; i < count; i++) {
		skfr::rateOnePuzzle(puzzlesToRate[i]);
		*res[i] = ((*res[i]) & 0xFF) | (puzzlesToRate[i].ed << 24) | (puzzlesToRate[i].ep << 16) | (puzzlesToRate[i].er << 8); //don't touch the less significant 8 bits!
	}
	count = 0;
}
#endif
