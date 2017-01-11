#ifndef USET_H_INCLUDED

#define USET_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <memory.h>
#include <fstream>
#include <set>
#include <vector>
//#include "tables.h"
#include "mm_allocator.h"
#include "t_128.h"
//#include "solver.h"

using namespace std;

struct ratingByDisjoints {
	int numDJ[24];
	double ratePlus; //less significant, useful for symmetric grids
	ratingByDisjoints();
	bool operator< (const ratingByDisjoints &r) const;
	void setRate(const int size, const int rate);
	void setRate(const double rate);
};

struct uset : public bm128 {
	unsigned int nbits;
	unsigned char positions[81];
	//int digitMask;
	//int numDisjoints;
	uset(void);
	uset(const bm128 &bm);
	bool operator < (const uset &rhs) const;
	void positionsByBitmap() {
		nbits = getPositions(positions);
	}
	void bitmapByPositions();
	void bitmapByIntegerPos(const int *pos, const int size);
	void toString(char *r) const;
	void toString1(char *r) const;
	void fromString(const char* s);
	void fromPuzzle(const char* s);
	/*inline*/ void operator=(const bm128 &rhs);
	//void toPseudoPuzzle(const char* digits, char* r) const;
};

class lightweightUsetList : public set<bm128, less<bm128>, mm_allocator<bm128> > {
public:
	int find2of3() const;
};

class usetList : public set<uset, less<uset>, mm_allocator<uset> > {
};

//class usetVector : public vector<uset, mm_allocator<uset> > {
//};

struct compareUsetBySize {
	bool operator() (const uset& _Left, const uset& _Right) const;
};

class usetListBySize : public set<uset, compareUsetBySize, mm_allocator<uset> > {
public:
	int distributionBySize[88];
	void insertNoSuperset(const uset &us);
	void removeSupersets();
	//void unused_setNumDisjionts ();
	void getRate(const unsigned int maxSize, const bm128 &mask, ratingByDisjoints &rating) const;
	void setDistributions();
	double getNumDisjoints(const bm128 &mask) const;
	int ReadFromFile(char const *filename);
};

struct sizedUset : public bm128 {
	sizedUset(void);
	sizedUset(const bm128 &bm);
	sizedUset(const bm128 &bm, const int size);
	sizedUset(const sizedUset &bm);
	int getSize() const;
	void setSize();
	void setSize(int newSize);
	//int decreaseSize();
	int join(const sizedUset &rhs);
	//bool operator < (const sizedUset &rhs) const;
	//void operator=(const bm128 &rhs);
	void operator=(const sizedUset &rhs);
	//inline bool isSubsetOf(const bm128 &s) const {return equals(s.bitmap128.m128i_m128i, _mm_or_si128(bitmap128.m128i_m128i, s.bitmap128.m128i_m128i));}
	inline bool isSubsetOf(const bm128 &s) const {
		bm128 u = bitmap128;
		u &= maskLSB[81];
		return u.isSubsetOf(s);
	}
	bool static isSmaller(const sizedUset elem1, const sizedUset elem2)
	{
	   return elem1.getSize() < elem2.getSize();
	   //return elem1 < elem2;
	}
//	void setRating(int rating) { //so that usets are ordered by size, rating, 81 cells
//		bitmap128.m128i_u32[2] = (((uint32_t)rating) << 17) | (bitmap128.m128i_u32[2] & 0x1FFFF);
//	}
};

//struct compareLightweightUsetBySize {
//	bool operator() (const bm128& _Left, const bm128& _Right) const;
//};
//
//class lightweightUsetListBySize : public set<bm128, compareLightweightUsetBySize, mm_allocator<bm128> > {
//};

class sizedUsetList : public set<sizedUset, less<sizedUset>, mm_allocator<sizedUset> > {};

class sizedUsetVector : public vector<sizedUset, mm_allocator<sizedUset> > {};

class bm128Vector : public vector<bm128, mm_allocator<bm128> > {};

class cliqueMember : public uset {
public:
	bm128 accumulatedBM;
	int numDisjoints;
	ratingByDisjoints rating;
	char mappedTo[88];
	cliqueMember();
	cliqueMember(const bm128 &bm);
	void setNumDisjoints(const bm128 &bm, const usetListBySize &ul);
	void rateIt(const usetListBySize &ul, const bm128 &bm);
	//void rateIt(const usetListBySize &ul, const bm128 &bm, ratingByDisjoints &result);
	void rateCells(const usetListBySize &ul);
	//void rateCells1(const usetListBySize &ul);
};

struct cliquePartition {
	bm128 bm; //all cells in the partition
	int memberSizes;
	int numMembers;
	int firstCliqueMember;
};

struct clique {
	cliqueMember ua[20];
	int size;
	uset fixedClues;
	uset fixedNonClues;
	clique();
	void insert(cliqueMember &m);
	void clear();
	void sortMembers(const usetListBySize &ul);
	void toString(char *r) const;
private:
	cliquePartition partitions[20];
	int nPartitions;
	void swap(const int m1, const int m2);
	void sortBySize();
	void createPartitions();
};

struct lightweightClique {
	unsigned int memberIndexes[20];
};

class lightweightCliqueList : public vector<lightweightClique> {};

//the only function in Gary McGuire's unav12.cpp
void FindUnavoidableSets12( int A[9][9], lightweightUsetList &sets, bool transpose = false );

#endif //USET_H_INCLUDED
