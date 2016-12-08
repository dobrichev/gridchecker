//Code inspired by Glenn Fowler's sudoku solver/generator
//The original is published under the following license
		/***********************************************************************
		*               This software is part of the ast package               *
		*          Copyright (c) 2005-2009 AT&T Intellectual Property          *
		*                      and is licensed under the                       *
		*                  Common Public License, Version 1.0                  *
		*                    by AT&T Intellectual Property                     *
		*                                                                      *
		*                A copy of the License is available at                 *
		*            http://www.opensource.org/licenses/cpl1.0.txt             *
		*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
		*                                                                      *
		*              Information and Software Systems Research               *
		*                            AT&T Research                             *
		*                           Florham Park NJ                            *
		*                                                                      *
		*                 Glenn Fowler <gsf@research.att.com>                  *
		***********************************************************************/
//This is a MODIFIED version of the original code

#ifndef ROWMINLEX_H_INCLUDED

#define ROWMINLEX_H_INCLUDED

#include <memory.h>
#include "tables.h"
#include "t_128.h"

#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

/* Define answer structure */
typedef struct Answer_s {
  unsigned short type;
  short sord[3];
  short rmap1[9], cmap1[9], smap1[10];
  short rmap2[9], cmap2[9], smap2[10];
} Answer_t;

struct subCanoner {
	Answer_t *Oldstk;
	subCanoner();
	~subCanoner();
	void canon(const char* puz, char* can);
	void static subcanon(const char* puz, char* can);
};

extern void rowminlex(const char *in, char *out);
//extern void bandminlex(const char *in, const int band, char *out);
extern void subcanon(const char* puz, char* can);
//extern void subcanon2(const char* puz, char* can); //test
extern int getBandNum(const char* sol);
extern void swapBands23(char* sol);

struct transformer
{
	unsigned int	box;
	unsigned int	map[10];
	unsigned int	row[9];
	unsigned int	col[9];
	unsigned int	aut;
	transformer *next; //0 terminated chain of automorphic transformations for aut > 1

	void byGrid(const char* sol);
	void byPuzzle(const char* sol);
	void byBand(const char* sol, const int band);
	void transform(const char *in, char *out);
	void transformAll(const char *in, char *out);
	void reverseTransform(const char *in, char *out);
	//void toString(char *buf) const;
	transformer() : box(0), aut(0) {
		next = 0;
	}
	const transformer & operator=(const transformer &t) {
		memcpy((transformer*)this, &t, sizeof(transformer));
		if(t.next) {
			transformer *t = new transformer();
			*t = *next;
			next = t;
		}
		return *this;
	}
	~transformer() {
		if(next) {
			delete next;
		}
	}
	NOINLINE void addAutomorph(transformer *test) {
		transformer *t = new transformer();
		*t = *test;
		t->map[0] = 0;
		t->next = next; //???
		next = t;
	}
	NOINLINE void clearAutomorphs() {
		if(next) {
			next->clearAutomorphs();
			next = 0;
		}
	}
	bool isTransforming() const {
		if(aut != 1) return true;
		if(box) return true;
		for(unsigned int i = 0; i < 9; i++) {
			if(row[i] != i || col[i] != i || map[i] != i) return true;
		}
		if(map[9] != 9) return true;
		return false;
	}
};
/*
struct pattern {
	//36 permutations
	struct permutation {
	};
	bm128 bands[3], stacks[3];
	pattern(const char* puz) {
		int src = 0;
		for(int band = 0; band < 3; band++) {
			bands[band].clear();
			stacks[band].clear();
			for(int row = 0; row < 3; row++) {
				for(int col = 0; col < 9; col++) {
					if(puz[9 * r + c]) {
						normal.setBit(row/3
					}
				}
			}
		}
	}
};
*/

#endif // ROWMINLEX_H_INCLUDED
