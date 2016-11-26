#include <vector>
#include <set>
#include <memory.h>

#include "BitMask768.h"

#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

using namespace std;

struct puzText {
	char chars[81];
	bool operator < (const puzText &rhs) const {
		return (memcmp(this, &rhs, sizeof(puzText)) < 0);
	}
};

class puzTextList : public vector<puzText> {};
class puzTextSet : public set<puzText> {};

struct state_t {
	//BitMask128 setMask;
	int cluePosition;
	int maxPositionLimitsByUA;
};

class clueIterator;

class simpleIterator {
	simpleIterator();
	int nPositions;
	int nClues;
public:
	int positions[82];
	simpleIterator(const int numCells, const int numClues) {
		nPositions = numCells;
		nClues = numClues;
		for(int i = 0; i < nClues; i++) {
			positions[i] = i;
		}
		positions[nClues] = nPositions;
	}
	bool increment() {
		int lsb = 0; //always start from the less significant bit
		do {
			positions[lsb]++;
			if(positions[lsb] < positions[lsb + 1]) {
				//thats OK
				for(int i = 0; i < lsb; i++) {
					positions[i] = i;
				}
				return true;
			}
			else {
				//must increment next position
				lsb++;
			}
		} while(lsb < nClues);
		//exhausted
		return false;
	}
	void test() {
		printf("%d bits on %d positions\n", nClues, nPositions);
		do {
			for(int i = 0; i < nClues; i++) {
				printf("%2d ", positions[i]);
			}
			printf("\n");
		} while(increment());
	}
};

class chunk {
public:
	static const int maxNumClues = 2;	//max positions of the leading numClues clues are fixed
	int cluePositions[maxNumClues];	//positions in the iterator's coordinate system
	int numClues;					//positions of the leading numClues clues are fixed
	double portion;					//portion of the total progress
};

class chunkList : public vector<chunk> {
public:
	chunkList() {
		srand((unsigned)time(NULL));
	}
	void randomize(void) {
		int nChunks = (int)size();
		for(int i = 0; i < nChunks - 1; i++) {
			int other = rand() % nChunks;
			chunk ch = at(i);
			at(i) = at(other);
			at(other) = ch;
		}
	}
	void rescalePortions() {
		int nChunks = (int)size();
		if(nChunks == 1)
			return;
		double progressScale = 1.0 / (1.0 - at(0).portion); //first chunk is starting from 0%
		for(int i = 0; i < nChunks - 1; i++) {
			at(i).portion = progressScale * (at(i + 1).portion - at(i).portion);
		}
		at(nChunks - 1).portion = progressScale * (1 - at(nChunks - 1).portion); //last chunk is finishing at 100%
	}
};

class cellMapper {
public:
	int i2g[81]; //iterator to grid mapping
	int g2i[81]; //grid to iterator mapping
	cellMapper() {
		for(int i = 0; i < 81; i++)
			i2g[i] = g2i[i] = i;
	}
	void map(const int gridCoord, const int iteratorCoord) {
		g2i[gridCoord] = iteratorCoord;
		i2g[iteratorCoord] = gridCoord;
	}
};

class chunkProcessor {
	BitMask768 hittingMasks[81];
	const clueIterator *theClueIterator;
	const chunk &theChunk;
	int clueNumber;
	int nClues;
	state_t state[81];
	const int *lsbBM;
	const bm128 *setsBM;
	int nFixed;
	bool stopAtFirst;

	NOINLINE void switch2bm(const bm128 *sets, const int *lsb, const int nsets);
	NOINLINE void iterateClue(const bm128 *sets, const int *lsb, const int nsets);
	NOINLINE void iterateClueBM(const BitMask768 &setMask, const int nFirst);
	NOINLINE bool checkPuzzle();
	NOINLINE void generatePuzzles();
	NOINLINE void minimizeFixed(char *puzzle);

public:
	unsigned int nPuzzles;
	unsigned long long nChecked;
	puzTextList validPuzzles;
	puzTextSet minimizedPuzzles;
	chunkProcessor(const clueIterator *clueIterator, const chunk &chunk);
	void process(void);
};

class clueIterator {
private:
	clueIterator();
public:
	BitMask768 hittingMasks[81];
	int *uaLSB;
	bm128 *allUA;
	int nAllUA;
	int nClues;
	int huntClues;
	grid &g;
	unsigned int nPuzzles;
	unsigned long long nChecked;
	int clueNumber; //nClues - 1 .. 0
	state_t state[81];
	//unsigned long long skipped[81]; //debug
	//unsigned long long maxPositionLimitsByUAweight[81]; //debug
	UATable tresholds[15]; //up to 14th clue
	clock_t start;
	cellMapper theMapper;
	double NoK[82][82];
	usetList usets;
	FILE *puzFile;
	//FILE *pseudoPuzFile;
	usetListBySize usetsBySize;
	clique theClique;
	chunkList theChunks;
	puzTextSet minimizedPuzzles;
	int numUaSizeLimit; //include UA of size 0..n so UA count < numUaSizeLimit (1500)
	int numUaTotalLimit; //add UA of larger size so total UA count to become numUaTotalLimit (2000)

	clueIterator(grid &g);
	void remap();
	void remapUA();
	void removeLargeUA();
	void iterateBoxes(const int numClues);
	void iterateDigits(const int numClues);
	void iterate2Boxes(const int numClues);
	void iterate2digits(const int numClues);
	void iterate3digits(const int numClues);
	void iterate4digits(const int numClues);
	void iterateFixedCells(const char *fixed, const int minClues = 0, const int maxClues = 100);
	void iterateBand(const int numClues);
	void iterateFixed(const int numClues);
	void iterateBands(const int numClues);
	int prepareGrid();
	void findTresholds();
	void iterateWhole(const int numClues);
	void iterate();
	NOINLINE void getChunks(const bm128 *sets, const int *lsb, const int nsets);
	NOINLINE void processChunks();
	void initProgress();
	NOINLINE double progress();
	NOINLINE void showProgress();
	NOINLINE void showProgress(const double pr);
};

