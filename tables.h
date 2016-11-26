#ifndef TABLES_H_INCLUDED

#define TABLES_H_INCLUDED

//A quick way to experiment how data representation affects the performance
typedef unsigned int cellIndex;
typedef unsigned char cellDigit;
typedef unsigned short bitmap;

extern const cellIndex affectedCells[81][20];
extern const int affectedGroups[81][3];
extern const cellIndex cellsInGroup[27][9];

extern const int BitCount[512];
extern const unsigned int Digit2Bitmap[33];
extern const int lowestBit[256];
extern const unsigned int rowByCellIndex[81];
extern const unsigned int colByCellIndex[81];
extern const unsigned int boxByCellIndex[81];
extern const unsigned int bandByCellIndex[81];
extern const unsigned int stackByCellIndex[81];

extern const char bands[416][81];
extern const char templ2[181][81];

struct transformationConstants
{
	unsigned char	swap[18][9][9];
	unsigned char	perm[6][3];
	unsigned char	part[9][5];
	unsigned char	boxOffset[9];
};

extern const transformationConstants tc;

extern const int choice2of9[36][2];
extern const int choice3of9[84][3];
extern const int choice4of9[126][4];
extern const int choice2223of9[1260][4];
extern const int choice333of9[280][3];

#endif //TABLES_H_INCLUDED