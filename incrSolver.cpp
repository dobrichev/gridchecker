//Incremental solver

#include "solver.h"

struct pencilmarks {
	//bitmap of cell possibilities
	//int pm[(81*9+8*sizeof(int)-1)/(sizeof(int)*8)];
#if USHRT_MAX < 0xffff
#error unsigned short type is too short
#endif
	unsigned short pm[81];
	int solvedCells;
	int solvedPencilmarks;

	//inline pencilmarks() {
	//	for(int i = 0; i < 81; i++)
	//		pm[i] = 0x1ff;
	//}

	inline void clearBit(const int cell, const int value) {
		pm[cell] &= (~(unsigned short)Digit2Bitmap[value]);
	}
	inline bool isCleared(const int cell, const int value) const {
		return 0 == (pm[cell] & Digit2Bitmap[value]);
	}
};

struct vector81 {
	int v[81];
	int size;
	vector81() {
		for(int i = 0; i < 81; i++)
			v[i] = 0;
		size = 0;
	}
	//vector81(const vector81 &x) {
	//	memcpy(this, &x, sizeof(this));
	//}
	inline void min(const vector81 &x) {
		for(int i = 0; i < size; i++) {
			if(v[i] > x.v[i]) {
				for(int j = i; j < size; j++) {
					v[j] = x.v[j];
				}
				return; //given is smaller and copied
			}
			else if(v[i] < x.v[i]) {
				return; //given is larger, do nothing
			}
		}
	}
	inline void max(const vector81 &x) {
		for(int i = 0; i < size; i++) {
			if(v[i] < x.v[i]) {
				for(int j = i; j < size; j++) {
					v[j] = x.v[j];
				}
				return; //given is larger and copied
			}
			else if(v[i] > x.v[i]) {
				return; //given is smaller, do nothing
			}
		}
	}
};

static const unsigned int Bitmap2Digit[512] =
{
        0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static unsigned int s = 0; //debug
static int knownSolution[81];
void addClue(const int cluesBM, const int nClues, const int *cluePos, const int *clueVal, const pencilmarks *basePM, pencilmarks *pm, char *puz) {
	pencilmarks *thisPM = &pm[cluesBM]; //pencilmarks to populate
	memcpy(thisPM, basePM, sizeof(pencilmarks)); //inherited pencilmarks from previously set clues
	char ppuz[81];
	memcpy(ppuz, puz, 81);
	ppuz[cluePos[0]] = clueVal[0]; //set the leftmost clue
	thisPM->solvedCells++;
	thisPM->solvedPencilmarks += BitCount[thisPM->pm[cluePos[0]]] - 1;
	thisPM->pm[cluePos[0]] = Digit2Bitmap[clueVal[0]];
	for(int c = 0; c < 81; c++) { //cell
		if(ppuz[c])
			continue; //a given
		for(int v = 1; v <= 9; v++) { //value
			if(thisPM->isCleared(c, v))
				continue; //cleared by previous clues
			//compose puzzle with givens in cluesBM set
			ppuz[c] = v;
			s++; //debug
			if(s%100000 == 0) printf(".");//debug
			if(solve(knownSolution, ppuz, 1))
				continue; //has solution => valid pencilmark
			thisPM->clearBit(c, v);
		}
		ppuz[c] = 0;
		if(Bitmap2Digit[thisPM->pm[c]]) { //solved cell
			ppuz[c] = Bitmap2Digit[thisPM->pm[c]];
			thisPM->solvedCells++;
		}
	}
	for(int i = 1; i < nClues; i++) {
		addClue(cluesBM | Digit2Bitmap[nClues], nClues - i, &cluePos[i], &clueVal[i], thisPM, pm, ppuz);
	}
}

void incSolve(const char *puz) {
	int cluePos[81];
	int clueVal[81];
	int nClues = 0;
	char ppuz[81];
	
	for(int i = 0; i < 81; i++) {
		ppuz[i] = 0;
		if(puz[i]) {
			cluePos[nClues] = i;
			clueVal[nClues] = puz[i];
			nClues++;
		}
	}
	if(nClues > 31) return; //error
	char sol[81];
	if(0 == solve(puz, 1, sol))
		return; //no solution
	digit2bitmap(sol, knownSolution);
	pencilmarks *pm = (pencilmarks *)malloc((1 << nClues) * sizeof(pencilmarks));
	//memset(pm, -1, sizeof(pencilmarks)); //set pencilmarks for the empty grid to 1
	for(int i = 0; i < 81; i++)
		pm->pm[i] = 0x1ff;
	pm->solvedCells = 0;
	pm->solvedPencilmarks = 0;

	for(int i = 0; i < nClues; i++) {
		addClue(Digit2Bitmap[i], nClues - i, cluePos, clueVal, pm, &pm[1 << i], ppuz);
	}
	//process results
	//vector81 minByCells, minByPencilmarks, maxByCells, maxByPencilmarks;


	free(pm);
	printf("\n%u solver calls\n", s);
}

//extern void test() {
//	//char p[82] = "000400000006000020800001005001600000000000070070020000340007000080000300000000060"; //mega 68004152 solver calls
//	char p[82] = "504200000000040030100000000060073000000000501000000000030000070000500400800020000";//20x17a 67723674 solver calls, Total time 2362.391 seconds.
//	for(int i = 0; i < 81; i++)
//		p[i] -= '0';
//	incSolve(p);
//}