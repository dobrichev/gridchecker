#include <malloc.h>
#include "rowminlex.h"

//bug
//.....1..2..1.3..4..5.2..6.....7...8...7.2.3..9....3..4..9.5..2..8.6.....4.......9 is canonicalizing to
//.....1..2..3....4..5..6.1....63...5..7..2.8..8....6.....9.4.....6.7..9..7....2..1
/*******************************************************************************
                          Disclaimer and License

		    Copyright (c) 2007 Michael Deverin

Suboutine "RowMinStdForm" converts a standard Sudoku puzzle (using the normal
set of possible row and column transpositions, and clue substitutions) to
Row-Minimal Standard Form (the smallest 81-digit number after rows are 
concatenated).

This software is public domain.  No explicit or implicit warranty is given.
If you use this code or ideas herein in any substantial way, a credit would
be appreciated.  Suggested improvements and bug reports are also welcome.

Language: C++
Author: Michael Deverin (devfam47@gmail.com)
Date: Jan.15.2007

********************************************************************************
                         Subroutine Description

char* subcanon(Canon_t* canon, Grid_t* grid, int stdform[81], int empty, int novalues)

The input array is a standard puzzle, with known clues coded 1-9 and unknown
clues coded as zero.  As usual, no clue should be repeated within a row,
column, or 3x3 box.  No error checking is done.

Gordon Royle maintains a collection of distinct 17-clue Sudoku puzzles.
He says, "Barring mistakes, these are guaranteed to be mathematically
inequivalent in that no two of them can be translated to each other by any
combination of the following operations:

   1) Permutations of the 9 symbols.
   2) Transposing the matrix. 
   3) Permuting rows within a single block.
   4) Permuting columns within a single block.
   5) Permuting the blocks row-wise.
   6) Permuting the blocks column-wise."

To most Sudoku enthusiasts, including myself, it is unclear how he converts
a candidate "new" 17-clue puzzle to his "base" form, so that he can compare
it to those already in his collection.

This routine converts a puzzle to "Row Minimal Standard Form".  Using
combinations of the operations listed above, the output "stdform" array
represents the smallest possible 81-digit number.

   Sample input (Royle #1) and output:
   a) 000000010400000000020000000000050407008000300001090000300400200
      050100000000806000
   b) 000000001000000020000003000000040500006000300007810000010020004
      030000070950000000

********************************************************************************
                              Time Trials

Time trials were conducted on a "Gateway 700S PC" that runs with a
"Intel 2.26 GHz Pentium 4 Processor".  Based on 1,000,000 trials for each
clue count, the number of conversions per second was:

  00 110339   10 24707   20 28658   30 30760   40 30942   50 30509   60 27455
  01 110181   11 25429   21 29252   31 31060   41 30310   51 29808   61 26850
  02   3208   12 25213   22 29423   32 30626   42 32094   52 30381   62 26823
  03   7108   13 26339   23 29627   33 31773   43 30839   53 29932   63 25905
  04  11506   14 26956   24 29775   34 32031   44 32059   54 30321   64 26303
  05  15023   15 27960   25 30294   35 30521   45 32036   55 29328   65 25923
  06  17816   16 28324   26 31260   36 30833   46 32050   56 28492   66 25610
  07  20710   17 28036   27 30857   37 30149   47 31639   57 28616   67 26132
  08  21932   18 28914   28 30298   38 32378   48 30640   58 28627   68 24963
  09  23505   19 28929   29 30401   39 32616   49 31194   59 27249   69 23546

  70  22490   73 18467   76 11936   79  2980   On Gordon Royle's set of 36628
  71  21431   74 16671   77  8848   80  2968   17-clue puzzles, the conversion
  72  19563   75 14524   78  6302   81  4917   rate was 25053 puzzles/second.
*******************************************************************************/
/* Set maximum stack sizes */
//#define MAXANS		5184	/* Test max = 3456 */
#define MAXANS		120000	/* Test max = 3456 */
//#define MAXANS		3359352 /* works for 81x1 pattern */

///* Define answer structure */
//typedef struct Answer_s {
//  unsigned short type;
//  short sord[3];
//  short rmap1[9], cmap1[9], smap1[10];
//  short rmap2[9], cmap2[9], smap2[10];
//} Answer_t;

extern void subcanon(const char* puz, char* can);

/*******************************************************************************
             Convert Sudoku Puzzle to Row-Minimal Standard Form

This routine converts a Sudoku puzzle to Row-Minimal Standard Form (RMSF),
which is defined as the smallest possible 81-digit number obtainable by
concatenating rows of the transformed 9x9 matrix.

             Input Puzzle                        Output Puzzle
         a b c   d e f   g h i		     h g i   e d f   b a c

    (1)  . 4 .   . . .   9 . .          (5)  . . .   . . .   . . .
    (2)  . 1 8   . 5 .   . . 6          (6)  . . .   . . 1   . 2 3
    (3)  . . 2   . . 4   . . 3          (4)  . 2 4   . 5 .   . . .

    (4)  . . .   5 . .   6 . 8          (2)  . . 2   5 . .   1 . 4
    (5)  . . .   . . .   . . .    =>    (3)  . . 6   . . 7   . . 8
    (6)  6 . 9   . . 1   . . .          (1)  . 3 .   . . .   7 . .

    (7)  8 . .   9 . .   2 . .          (7)  . 8 .   . 3 .   . 4 .
    (8)  5 . .   . 3 .   4 6 .          (8)  2 7 .   6 . .   . 5 .
    (9)  . . 1   . . .   . 7 .          (9)  9 . .   . . .   . . 1

              Input Clues:   1  2  3  4  5  6  7  8  9
              Output Clues:  1  8  6  7  5  2  9  4  3

In the above example, it should not be surprising that input row-5 becomes
the first row in the output grid.  Is there a better way to begin a minimal
number than with nine zeros?  Since there are no other empty input rows (or
columns), this mapping is now 100% certain.

Initially, we must consider both the input puzzle and its transpose as viable
starting grids.  Either may eventually produce the minimal number.  Here, the
transpose has already been eliminated as as candidate.

Since we are mapping a row with all zero cells, any legal arrangement of the
columns will produce the same '000 000 000' number.  These, arrangements can be
expressed as R51[((abc)(def)(ghi))], meaning that input row-5 maps to output
row-1, and that blocks of columns and columns within a block may be permuted.
At this point, it is not necessary (it's counter-productive) to choose any
specific column arrangement.

Since rows must be mapped in blocks of three, either R4 or R6 must follow R5.
Without a little study, you will be convinced that both:

   R42[(abc)(ef)dh(gi)] => '000 001 023' and
   R62[(ghi)(ed)fb(ac)] => '000 001 023'

will give the smallest number for output row-2. To make the hypotheses a bit
more specific, where clues appear within a permutation grouping, let's just
list the choices separately:

   R42[(abc)(ef)dhgi] => '000 001 023'
   R42[(abc)(ef)dhig] => '000 001 023'
   R62[(ghi)(ed)fbac] => '000 001 023'  (correct choice)
   R62[(ghi)(ed)fbca] => '000 001 023'

For R62[(ghi)(ed)fbac], it is implicit that input clues '1', '6' and '9' must
appear as output clues '1', '2' and '3', respectively.  Let's be explicit and
add this to our notation: 

   R42[(abc)(ef)dhgi; ....12.3.] => '000 001 023'
   R42[(abc)(ef)dhig; ....13.2.] => '000 001 023'
   R62[(ghi)(ed)fbac; 1....2..3] => '000 001 023'  (correct choice)
   R62[(ghi)(ed)fbca; 1....3..2] => '000 001 023'

Let's follow the correct mappings for one more row.  Given the first two
row mappings, only one choice remains for the third:

   (1) R51[((abc)(def)(ghi)); .........] => '000 000 000'
   (2) R62[(ghi)(ed)fbac;     1....2..3] => '000 001 023'
   (3) R43[hgiedfbac;         1...52.43] => '024 050 000'

Note that:
   (3) R43[higedfbac;         1...52.83] => '042 050 000'
produces a non-minimal number.

For this particular puzzle, we have learned a lot from mapping three rows:
All nine columns have been 'fixed' and we know substitution values for five
of the clues.

The basic principle:  when a non-zero cell is mapped from input to output
grids, pin down it's new location and identity change.  All putative mappings
within a stack of mappings must produce the same (minimal) number.  If a
new number becomes minimal, start a new stak.  This algorithm involves 
backtracking with a lot of bookkeeping.

*******************************************************************************/


/*******************************************************************************
                  Compute and Return "WhizBang" Block Score
*******************************************************************************/
static int BlockScore (int m[9], const int *perm1, const int *perm2)
{ 
  int score, *r1, *r2, *r3;

  r1 = m + 3*perm1[0];
  r2 = m + 3*perm1[1];
  r3 = m + 3*perm1[2];

  /* Concatenate counts */
  score = (r1[perm2[0]] << 16) ^ (r1[perm2[1]] << 14) ^ (r1[perm2[2]] << 12) ^
          (r2[perm2[0]] << 10) ^ (r2[perm2[1]] <<  8) ^ (r2[perm2[2]] <<  6) ^
          (r3[perm2[0]] <<  4) ^ (r3[perm2[1]] <<  2) ^ (r3[perm2[2]]      );
  
  //Mladen Dobrichev: the following is the same as above
  //int score1 = (r1[perm2[0]] << 16) | (r1[perm2[1]] << 14) | (r1[perm2[2]] << 12) |
  //        (r2[perm2[0]] << 10) | (r2[perm2[1]] <<  8) | (r2[perm2[2]] <<  6) |
  //        (r3[perm2[0]] <<  4) | (r3[perm2[1]] <<  2) | (r3[perm2[2]]      );

  ////MD: patch for closer-to-min-lex
  ////bits are aabbccDDEEFFgghhjj, where abc=first row givens count per triplet
  ////if aa <> 0 they potentially interfere with DD and gg
  ////the easiest way to fix this is to set DD and GG to 3
  //if(r1[perm2[0]]) score |= ((3 << 10) | (3 << 4));
  //if(r1[perm2[1]]) score |= ((3 << 8) | (3 << 2));
  //if(r1[perm2[2]]) score |= ((3 << 6) | (3));
  //if(r2[perm2[0]]) score |= (3 << 4);
  //if(r2[perm2[1]]) score |= (3 << 2);
  //if(r2[perm2[2]]) score |= (3);
  ////MD: not sufficient, still not minlex
  //score |= 0xfff; //0.515" vs 0.421"
  
  return(score);
}

/*******************************************************************************
                            Initialize an Answer
*******************************************************************************/
static void AnswerInit (Answer_t *ans)
{
  int i;
  /* Init "segment order" map. */
  for ( i=0; i < 3; i++ )
      ans->sord[i] = -1;

  /* Init row, row inverse, column, column inverse mappings. */
  for ( i=0; i < 9; i++ )
      ans->rmap1[i] = ans->rmap2[i] =
      ans->cmap1[i] = ans->cmap2[i] = -1;

  /* Init symbol and symbol inverse mappings. */
  for ( i=0; i < 10; i++ )
      ans->smap1[i] = ans->smap2[i] = 0;
}

/*******************************************************************************
				WhizBang

Row and column blocks are "scored."  Only a few will be candidates which might
eventually produce the minimal number.

             Input Puzzle                        Output Puzzle
         a b c   d e f   g h i		     h g i   e d f   b a c

    (1)  . 4 .   . . .   9 . .          (5)  . . .   . . .   . . .
    (2)  . 1 8   . 5 .   . . 6          (6)  . . .   . . 1   . 2 3
    (3)  . . 2   . . 4   . . 3          (4)  . 2 4   . 5 .   . . .

    (4)  . . .   5 . .   6 . 8          (2)  . . 2   5 . .   1 . 4
    (5)  . . .   . . .   . . .    =>    (3)  . . 6   . . 7   . . 8
    (6)  6 . 9   . . 1   . . .          (1)  . 3 .   . . .   7 . .

    (7)  8 . .   9 . .   2 . .          (7)  . 8 .   . 3 .   . 4 .
    (8)  5 . .   . 3 .   4 6 .          (8)  2 7 .   6 . .   . 5 .
    (9)  . . 1   . . .   . 7 .          (9)  9 . .   . . .   . . 1

1) Start by making a 27-long frequency count of clues which occur on
   consecutive 3-long row segments. (This also needs to be done for
   columns.)  
                     Rows 1-3      Rows 4-6      Rows 7-9         
      Row Counts =  101 211 111   012 000 210   111 112 107

2) Treat each set of nine counts separately.  Let's put the counts for
   rows 4-6 into a 3 x 3 matrix.  Then, rearrange matrix cells, using
   separate row and column permutations.  All 36 permutation combinations
   will be tried.

            2 1 0                  Concatenate cells in the new
           ------                  matrix to form a "score".
   (4) 2  | 0 1 2         0 0 0
   (5) 0  | 0 0 0    =>   0 1 2          Score = 000012210
   (6) 1  | 2 1 0         2 1 0

3) Over the whole process, 216 "scores" are computed (36 per block, with
   three row blocks and three column blocks (for the puzzle transpose).
   Save only data associated with each minimum score.

*******************************************************************************/
/* Mladen Dobrichev: the purpose of WhizBang function is to create a list
of all reasonable mapping configurations for the:
a) transpose (ans->type: 0 = no, 1 = transpose);
b) top band row permutations (ans->rmap1[3] and ans->rmap2[3]);
c) stack permutations (ans->sord[3] = one of 036,063,306,360,603,630).
*/
static int WhizBang (Answer_t *stk, char *twopuz[2])
{
	/* Functions & structures */
	Answer_t *ans;

	/* Other declarations */
	int shadow[81];
	const int *perm1, *perm2;
	int rowcount[27], colcount[27], *cp[2], *bp;
	int Score[216], minscore, score, s1; //Score is ordered by 108 * type + 36 * bandIndex + 6 * rowPerm + colPerm
	int save[216], savelen, savlen;
	int type, block, ii, jj, kk, i, j, k;
	char *puz;
	static const int six[] = { 0,1,2, 0,2,1, 1,0,2, 1,2,0, 2,0,1, 2,1,0 };

	cp[0] = rowcount;
	cp[1] = colcount;
	/* Create shadow puzzle of '1's and '0's */
	puz = twopuz[0];
	for ( i=0; i < 81; i++ )
		shadow[i] = puz[i] != 0;

	/* Make frequency count of clues for each 3-long row/column segment. */
	for ( k=j=i=0; i < 27; i++ )
	{
		rowcount[i] = shadow[j] + shadow[j+1] + shadow[j+2] ;  j += 3;
		colcount[i] = shadow[k] + shadow[k+9] + shadow[k+18];  k += 27;
		if ( k >= 81 )
			k -= 80;
	}

	/* Loop over puzzle 'type' (puzzle or its transpose). */
	/* Loop over row blocks, and row & column 3-long permutations.  */
	minscore = 1<<20;
	for ( savelen=k=type=0; type < 2; type++ )
	{
		for ( block=0; block < 27; block += 9 )
		{
			bp = cp[type] + block;
			for ( ii=0; ii < 6; ii++ )
			{
				for ( jj=0; jj < 6; jj++, k++ )
				{
					Score[k] = score = BlockScore(bp,six+3*ii,six+3*jj);
					if (score > minscore)
						continue;
					if (score < minscore)
					{
						minscore = score; savelen = 0; }
					save[savelen++] = k;               
				}
			}
		} /* block */
	} /* type */

	/* If all-zero block is found, apply filters. */
#if 1
	if (minscore == 0)
	{
		/* Filter #1: Save only answers with row permutation 0-1-2. */
		for ( savlen=i=0; i < savelen; i++ )
			if ( ((save[i]/6) % 6) == 0 )
				save[savlen++] = save[i];
		savelen = savlen;

		/* Filter #2: Save only answers with minimal follow-on score. */
		minscore = 1<<20;
		for ( savlen=i=0; i < savelen; i++ )
		{ 
			/* Unpack answer info. */
			/* Note: ii == 0, for all saved from Filter #1. */
			jj = save[i];
			type = jj/108;  jj -= (108*type);
			block = jj/36;  jj -= (36*block);

			/* Find min score for: same type, different block, ii=0, same jj */
			//2011-07-28 MD: Wrong, must combine with any row rermutation ii since it is in other band and therefore is independent
			//To reproduce, test the following isomorphs
			//..................................12.....3..4..5.6...7...4..8....62......97...3..
			//................................1..2..3....45..6..7....5..2...8.7.3.....91.......

			s1 = 1<<20;
			for ( j=0; j < 3; j++ )
			{
				//if ( j != block )
				//{
				//	score = Score[ (108*type)+(36*j)+jj ];
				//	if ( s1 > score )
				//		s1 = score;
				//}
				if ( j != block ) {
					for(ii = 0; ii < 6; ii++) { //fix: iterate all row permutations of the middle band candidate
						score = Score[ (108*type)+(36*j)+(6*ii)+jj ];
						if ( s1 > score )
							s1 = score;
					}
				}
			}

			/* Re-initiate list if new minimal follow-on score. */
			if (s1 > minscore)
				continue;
			if (s1 < minscore) {
				minscore = s1;
				savlen = 0;
			}
			save[savlen++] = save[i];
		}
		savelen = savlen;
	}

	/* If only two zero rows, apply filter. */
	else if ( (minscore>>6) == 0 ) {
		/* Save only answers with row permutation 0-1-2, 0-2-1 or 1-2-0. */
		for ( savlen=i=0; i < savelen; i++ )
		{ jj = save[i];
		type = jj/108;  jj -= (108*type);
		block = jj/36;  jj -= (36*block);  ii = jj/6;
		if ( (ii<=3) && (ii!=2) ) save[savlen++] = save[i];
		}
		savelen = savlen;
	}
#endif
	/* Create initial answer stak. */
	for ( kk=0; kk < savelen; kk++ ) {
		AnswerInit(ans=stk+kk);
		jj = save[kk];
		type = jj/108;  jj -= (108*type);
		block = jj/36;  jj -= (36*block);
		ii = jj/6;	    jj -= (6*ii);
		k = 3*block;		/* Base row */
		perm1 = six + 3*ii;		/* Row perm */
		perm2 = six + 3*jj;		/* Col perm */

		ans->type = type;
		ans->sord[0]  = 3*perm2[0];
		ans->sord[1]  = 3*perm2[1];
		ans->sord[2]  = 3*perm2[2];
		ans->rmap2[0] = j = (perm1[0]+k);	ans->rmap1[j] = 0;
		ans->rmap2[1] = j = (perm1[1]+k);	ans->rmap1[j] = 1;
		ans->rmap2[2] = j = (perm1[2]+k);	ans->rmap1[j] = 2;
	}

	/* Return list length. */
	return(savelen);
}
//int maxLen = 0; //debug
/*******************************************************************************
                                AttachRownum

When starting to map "to" rows 3 thru 8, for every "old" answer, attach a
"from" row number to that answer, and create alternate answers (at the end
of the old stak) when other "from" row choices exist.
*******************************************************************************/
static int AttachRownum (Answer_t *oldstk, int oldlen, int r2)
{
	Answer_t  *ans, *alt;
	int	  newlen, r1min, r1max, r1, first, ii;

	/* Loop over answers in old stak. */
	for ( newlen=oldlen, ii=0; ii < oldlen; ii++ )
	{
		/* Determine range of "from" rows that are eligible to be mapped */
		ans = oldstk + ii;
		if ( (r2 == 3) || (r2 == 6) )
		{ r1min = 0;
		r1max = 9;
		}
		else
		{ r1 = ans->rmap2[ r2 - 1 ];
		r1min = 3*(r1/3);
		r1max = r1min + 3;
		}

		/* Loop over eligible "from" rows */
		for ( first=99, r1=r1min; r1 < r1max; r1++ )
		{
			/* Skip, if already mapped. */
			if ( ans->rmap1[r1] >= 0 ) continue;

			/* Extend answer, but delay mapping "first" answer so that */
			/* copy doesn't get messed up. */
			if (first==99)
			{ first = r1; }
			else
			{ /* Create alternate answer. */
				alt = oldstk+newlen;		newlen++;
				//if(maxLen = newlen + sizeof(ans[0])) maxLen = newlen + sizeof(ans[0]);
				alt[0] = ans[0];		/* Copy structure */
				alt->rmap1[r1] = r2;
				alt->rmap2[r2] = r1;
			}
		}

		/* Map first answer */
		r1 = first;
		ans->rmap1[r1] = r2;
		ans->rmap2[r2] = r1;  
	}

	/* Save new stak length. */
	return(newlen);
}

/*******************************************************************************
*******************************************************************************/
static int NextPerm (int *perm, int len)
{
  /* *** Note:  0 <= len <= 20 *** */
  int copy[20], ii, i, j, k;

  /* Compute initial permutation. */
  if ( perm[0] < 0 )
  { if ( len <= 0 ) return(0);			/* No perms exist! */
    for ( i=0; i < len; i++ ) perm[i] = i;
    return(1);
  }

  /* Check right-to-left for first place where perm[i] < perm[i+1]. */
  /* Quit, if no such place occurs.  */
  for ( i=len-2; (i >= 0) && (perm[i] > perm[i+1]); i-- );
  if ( i < 0 ) return(0);

  /* Make copy of right part of permutation. */
  for ( ii=j=i; j < len; j++ ) copy[j] = perm[j];

  /* Search right-to-left for first value greater than perm[i] */
  for ( j=len-1; perm[i] > perm[j]; j-- );

  /* Copy new "base" value. Ascending values are to follow. */
  perm[i++] = copy[j];

  /* Copy values (if any) to the right of position 'j' */
  for ( k=len-1; k > j; k-- ) perm[i++] = copy[k];

  /* Insert old "base" value into ascending values */
  perm[i++] = copy[ii];

  /* Copy values (if any) to the left of position 'j' */
  for ( k=j-1; i < len; k-- ) perm[i++] = copy[k];
  return(1);
}

/*******************************************************************************
*******************************************************************************/
static int CompareKeys (int *a, int *b)
{
  if (a[0] < b[0]) return -1;
  if (a[0] > b[0]) return +1;
  if (a[1] < b[1]) return -1;
  if (a[1] > b[1]) return +1;
  if (a[2] < b[2]) return -1;
  if (a[2] > b[2]) return +1;
  return 0;
}

/*******************************************************************************
*******************************************************************************/
static int ExtendAnswer (Answer_t *ans, int pass,
						 Answer_t *stk, int stklen, int topkey[3], char *twopuz[2])
{
	Answer_t *alt;
	int compare;
	int perm[3], key[3], collist[3], clist[3], clen1, clen2;
	int kk, k1, k2, tt, t1, t2, s1, s2, r1, r2, segix;
	int ii, i;
	char *row;

	/* Initializations. */
	r2 = pass/3;                        /* "To" row number */
	segix = pass%3;                     /* "To" segment index */
	r1 = ans->rmap2[r2];                /* "From" row number */

	row = twopuz[ans->type] + (9*r1);   /* "From" row pointer. */
	kk = ans->sord[segix];              /* First column in "from" segment. */
	k2 = 3 + (k1 = kk);
	t2 = 3 + (t1 = 3*segix);

	/* Find unmapped columns that have clues. */
	for ( clen1=0, kk=k1; kk < k2; kk++ )
		if ( (row[kk] > 0) && (ans->cmap1[kk] < 0) )
			clist[clen1++] = kk;

	/* Tackon mapped columns that have clues. */
	for ( clen2=clen1, kk=k1; kk < k2; kk++ )
		if ( (row[kk] > 0) && (ans->cmap1[kk] >= 0) )
			clist[clen2++] = kk;

	/* Make copy of column list. */
	for ( i=0; i < clen2; i++ ) collist[i] = clist[i];

	/* Jump-in NextPerm() loop if there are no unmapped columns. */
	perm[0] = -1;
	if ( clen1 == 0 ) goto jump_in;

	/* Loop over permutations of unmapped columns columns having clues. */
	while ( NextPerm(perm,clen1) )
	{
		/* Make copy of "base" answer */
jump_in:
		alt = stk+stklen;
		alt[0] = ans[0];		/* Structure copy */

		/* Choose arrangement of unmapped columns. */
		for ( i=0; i < clen1; i++ )
			collist[i] = clist[ perm[i] ];

		/* Map unmapped column(s). */
		for ( ii=0; ii < clen1; ii++ ) {
			kk = collist[ii];
			for ( tt=t2-1; tt >= t1; tt-- )
				if ( alt->cmap2[tt] < 0 )
					break;     
			alt->cmap1[kk] = tt;
			alt->cmap2[tt] = kk;
		}

		/* Compute key. */
		/* Map symbols when unmapped */
		for ( ii=0; ii < 3; ii++ )
		{
			/* Query "to" columns from left-to-right */
			key[ii] = 0;
			if ( (kk = alt->cmap2[t1+ii]) < 0 ) continue;
			if ( (s1 = row[kk]) <= 0 ) continue;

			/* Determine mapped symbol */
			if ( (s2 = alt->smap1[s1]) <= 0 ) {	  /* Symbol already known */
				for ( s2=1; s2 < 10; s2++ )       /* Find next available */
					if ( alt->smap2[s2] <= 0 )
						break; 
			}

			/* Record info */
			key[ii] = s2;
			alt->smap1[s1] = s2;
			alt->smap2[s2] = s1;
		}

		/* Check for a new top key? */
		if ( (compare = CompareKeys(key,topkey)) < 0 ) {
			stk[0] = alt[0];	/* Structure copy */
			for ( i=0; i < 3; i++ )
				topkey[i] = key[i];
			stklen = 1;
		}
		else if ( compare == 0 )
			stklen++;

	} /* NextPerm() */

	/* Return updated stak length. */
	return(stklen);

} /* End-of-function */

subCanoner::subCanoner() {
	Oldstk = (Answer_t*)malloc(2 * MAXANS * sizeof(Answer_t));
}

subCanoner::~subCanoner() {
	free(Oldstk);
}

void subCanoner::subcanon(const char* puz, char* can) {
	static subCanoner sc;
	sc.canon(puz, can);
}

void subCanoner::canon(const char* puz, char* can) {
	/* Declarations */
	Answer_t *oldstk, *newstk, *ans;   	
	char puzzle[81], transpose[81], *p, *twopuz[2];
	int oldstklen, newstklen;
	int topkey[3], pass, segix, rownum;
	int cluekt, ii, i, j, k;

	Answer_t *Newstk;

	Newstk = Oldstk + MAXANS;
	oldstk = Oldstk;
	newstk = Newstk;
	/* Special check for some stupid input. */
	for ( cluekt=i=0; i < 81; i++ )
		if (puz[i])
		{
			puzzle[i] = puz[i];
			cluekt++;
		}
		else
			puzzle[i] = 0;

	/* Puzzle has either zero or one clue. */
	if (cluekt <= 1)
	{ 
		if (can)
		{
			for ( i=0; i < 81; i++ ) can[i] = 0;
			if (cluekt) can[80] = 1;
		}
		return;
	}

	/* Compute transpose. */
	twopuz[0] = puzzle;          /* Pointer to puzzle */
	twopuz[1] = transpose;       /* Pointer to puzzle transpose */
	for ( k=i=0; i < 9; i++ )
		for ( j=0; j < 81; j += 9 )
			transpose[i+j] = puzzle[k++];

	/* Construct initial stak of partial answers. */
	if ((oldstklen = WhizBang(oldstk, twopuz)) >= MAXANS)
		goto final;

	/* Loop over "to" row numbers. */
	for ( pass=rownum=0; rownum < 9; rownum++ )
	{
		/* Attach "from" row numbers. */
		if (rownum >= 3 && (oldstklen = AttachRownum(oldstk,oldstklen,rownum)) >= MAXANS)
			goto final;

		/* Loop over "to" segments */
		for ( segix=0; segix < 3; segix++, pass++ )
		{
			/* Initialize "new" stak and best key. */
			for ( newstklen=i=0; i < 3; i++ ) topkey[i] = 9;

			/* Loop over old stak answers. */
			for ( ii=0; ii < oldstklen; ii++ )
			{
				ans = oldstk + ii;
				if ((newstklen = ExtendAnswer(ans,pass,newstk,newstklen,topkey,twopuz)) >= MAXANS)
					goto final;
			}

			/* Swap old stak and new stak pointers. */
			ans = oldstk;
			oldstk = newstk;
			newstk = ans;
			oldstklen = newstklen;

		} /* segix */
	} /* rownum */

	/* Compute final answer */
final:
	ans = oldstk;
	p = twopuz[ans->type];
	if (can)
	{
		for ( k=i=0; i < 9; i++ )
			for ( j=0; j < 9; j++, k++ )
				if ( (ii = ans->cmap2[j]) < 0 )
					can[k] = 0;
				else
					can[k] = (char)ans->smap1[((unsigned char*)p)[(9*ans->rmap2[i]) + ii]];
	}
	//if (canon)
	//{
	//  canon->aut = 0;
	//  canon->box = ans->type ? 9 : 0;
	//  canon->map[0] = 0;
	//  for (i = 0; i < 9; i++)
	//  {
	//    canon->map[i+1] = ans->smap1[i+1];
	//    canon->row[i] = ans->rmap2[i];
	//    canon->col[i] = ans->cmap2[i];
	//  }
	//}
}
//#include <stdio.h> //debug
extern void subcanon(const char* puz, char* can) {
	subCanoner::subcanon(puz, can);
	//printf("/nmaxLen=%d/n", maxLen); //debug
}

#if 0
extern void subcanon(const char* puz, char* can)
{
	/* Declarations */
	Answer_t *oldstk, *newstk, *ans;   	
	char puzzle[81], transpose[81], *p, *twopuz[2];
	int oldstklen, newstklen;
	int topkey[3], pass, segix, rownum;
	int cluekt, ii, i, j, k;

	static Answer_t *Oldstk, *Newstk;

	if (!Oldstk)
	{
		if (!(Oldstk = (Answer_t*)malloc(2 * MAXANS * sizeof(Answer_t))))
			return;
		Newstk = Oldstk + MAXANS;
	}
	oldstk = Oldstk;
	newstk = Newstk;
	/* Special check for some stupid input. */
	for ( cluekt=i=0; i < 81; i++ )
		if (puz[i])
		{
			puzzle[i] = puz[i];
			cluekt++;
		}
		else
			puzzle[i] = 0;

	/* Puzzle has either zero or one clue. */
	if (cluekt <= 1)
	{ 
		if (can)
		{
			for ( i=0; i < 81; i++ ) can[i] = 0;
			if (cluekt) can[80] = 1;
		}
		return;
	}

	/* Compute transpose. */
	twopuz[0] = puzzle;          /* Pointer to puzzle */
	twopuz[1] = transpose;       /* Pointer to puzzle transpose */
	for ( k=i=0; i < 9; i++ )
		for ( j=0; j < 81; j += 9 )
			transpose[i+j] = puzzle[k++];

	/* Construct initial stak of partial answers. */
	if ((oldstklen = WhizBang(oldstk, twopuz)) >= MAXANS)
		goto final;

	/* Loop over "to" row numbers. */
	for ( pass=rownum=0; rownum < 9; rownum++ )
	{
		/* Attach "from" row numbers. */
		if (rownum >= 3 && (oldstklen = AttachRownum(oldstk,oldstklen,rownum)) >= MAXANS)
			goto final;

		/* Loop over "to" segments */
		for ( segix=0; segix < 3; segix++, pass++ )
		{
			/* Initialize "new" stak and best key. */
			for ( newstklen=i=0; i < 3; i++ ) topkey[i] = 9;

			/* Loop over old stak answers. */
			for ( ii=0; ii < oldstklen; ii++ )
			{
				ans = oldstk + ii;
				if ((newstklen = ExtendAnswer(ans,pass,newstk,newstklen,topkey,twopuz)) >= MAXANS)
					goto final;
			}

			/* Swap old stak and new stak pointers. */
			ans = oldstk;
			oldstk = newstk;
			newstk = ans;
			oldstklen = newstklen;

		} /* segix */
	} /* rownum */

	/* Compute final answer */
final:
	ans = oldstk;
	p = twopuz[ans->type];
	if (can)
	{
		for ( k=i=0; i < 9; i++ )
			for ( j=0; j < 9; j++, k++ )
				if ( (ii = ans->cmap2[j]) < 0 )
					can[k] = 0;
				else
					can[k] = (char)ans->smap1[((unsigned char*)p)[(9*ans->rmap2[i]) + ii]];
	}
	//if (canon)
	//{
	//  canon->aut = 0;
	//  canon->box = ans->type ? 9 : 0;
	//  canon->map[0] = 0;
	//  for (i = 0; i < 9; i++)
	//  {
	//    canon->map[i+1] = ans->smap1[i+1];
	//    canon->row[i] = ans->rmap2[i];
	//    canon->col[i] = ans->cmap2[i];
	//  }
	//}
} /* End-of-routine */
#endif

/*******************************************************************************
				snippet.c
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

static const int twoRowChains[224] = {
      71835,   72075,   74139,   74343,   74907,   75291,   75303,   75339,
      99483,   99867,   99879,   99915,  100455,  100635,  100647,  100743,
     102795,  102939,  102951,  102987,  104523,  104583,  135579,  135783,
     137319,  137499,  137511,  137607,  140391,  140871,  148635,  149019,
     149031,  149067,  149607,  149787,  149799,  149895,  155931,  155943,
     156039,  156231,  156747,  156807,  287451,  288459,  289947,  296667,
     297435,  299739,  301515,  301851,  301863,  302235,  307611,  307815,
     398043,  399819,  400155,  400167,  400539,  401883,  403143,  403227,
     403239,  404583,  411339,  412107,  412443,  412455,  418251,  418503,
     420939,  420999,  422283,  422427,  422439,  422475,  443547,  443931,
     443943,  443979,  444519,  444699,  444711,  444807,  465051,  465291,
     467355,  467559,  468123,  468507,  468519,  468555,  542427,  543195,
     549339,  550599,  550683,  550695,  552039,  553371,  553575,  561627,
     563655,  564327,  594651,  596427,  596763,  596775,  597147,  598491,
     599751,  599835,  599847,  601191,  624327,  624411,  624423,  625095,
     627147,  627399,  629835,  629895,  635163,  635175,  635271,  635463,
     640155,  640539,  640551,  640587,  641127,  641307,  641319,  641415,
     725403,  725607,  727143,  727323,  727335,  727431,  730215,  730695,
    1159899, 1209051, 1230555, 1231323, 1602267, 1618395, 1683915, 1684167,
    1689291, 1690059, 1690395, 1690407, 1774299, 1776075, 1776411, 1776423,
    1776795, 1778139, 1779399, 1779483, 1779495, 1780839, 1860315, 1861323,
    1862811, 1869531, 1870299, 1872603, 1874379, 1874715, 1874727, 1875099,
    1880475, 1880679, 2208219, 2213595, 2214363, 2257371, 2388699, 2404827,
    2519499, 2519751, 2541255, 2541339, 2541351, 2542023, 2560731, 2562507,
    2562843, 2562855, 2563227, 2564571, 2565831, 2565915, 2565927, 2567271,
    2901723, 2902491, 2908635, 2909895, 2909979, 2909991, 2911335, 2912667,
    2912871, 2920923, 2922951, 2923623, 7107291, 7123419, 7451355, 7500507,
    7522011, 7522779,10253019,10269147,11645403,11650779,11651547,11694555
};

static const int twoRowChainRanks[224] = {
    10, 8,10, 8,11,11,10,10,11, 2,11,11,10,11,11,10,10,11,10,11,10, 8, 8,10,11,
    10,11,10,10, 8,10,11,11,10,11,11, 2,11,10,11,11,10, 8,10, 4, 4, 9, 7, 4, 7,
     4,12,12, 9, 9, 9, 7, 7, 1,14, 9, 4, 7,12,12, 9, 7, 7,12,12, 4, 4, 9, 9, 9,
     9, 9, 9,12, 1,12,12,12,12,14,12, 7, 4, 4, 4, 7, 7, 7, 4, 4, 7, 7, 4,12,12,
     9, 9, 9, 4, 4, 9, 4, 7,12,12, 9, 7, 7,14, 1, 9, 7,12,12, 7, 4, 4, 9, 9, 9,
     9, 9, 9,12,14,12,12,12,12, 1,12, 4, 4, 7, 7, 7, 4, 7, 4, 3, 3, 6, 6, 3, 3,
     3, 3, 6, 6, 6, 6, 6, 6, 0,13, 6, 6, 6,13,13, 6, 3, 3, 6, 3, 3, 3, 3, 6, 6,
     6, 3, 3, 3, 6, 6, 3, 3, 3, 3, 3, 6, 6, 6, 6, 6, 6,13,13, 6, 6, 6,13, 0, 6,
     3, 3, 3, 3, 6, 6, 6, 3, 3, 3, 3, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
};

  // Function prototypes
int TwoRowChainer(const char *r1, const char *r2);
void FatalError(const char *routine);

/*******************************************************************************
			      TwoRowRanker()
Purpose:
  Given two full rows from the same band, this routine returns a number [0,14]
  which ranks how well the rows would be as the first two rows (either order)
  in a Row Minlex Form matrix.  Rank=0 is best.
*******************************************************************************/
int TwoRowRanker (const char *r1, const char *r2)
{
  // Use a binary search of this pre-computed table to find the output of
  // TwoRowChainer().  The "rank" is in the corresponing spot in array twoRowChainRanks[].

  // Compute two-row cycle structure representative
  int i, j, n;
  int x = TwoRowChainer(r1, r2);

  // Find representative in twoRowChains (binary search).
  for ( i=0, j=224; i < j; )
  {
    n = (i+j)>>1;
    if ( twoRowChains[n] < x ) { i = n+1; continue; }
    if ( twoRowChains[n] > x ) { j = n;   continue; }
    return(twoRowChainRanks[n]);
  }

  // Fatal error.
  FatalError("TwoRowRanker()");
  return 0;

} // End-of-function

/*******************************************************************************
			     TwoRowChainer()
Purpose:
  Using chaining, compute the "abc" cycle structure of two rows from the
  same band.
Return value:
  After the cycle structure is computed, all cycles are stuffed into a single
  word.  The "abc" values are coded as 0,1,2 and cycle separators coded as 3.
  Each coded value occupies 2-bits.  The stuffed word is returned and is
  meant to used as input to TwoRowRanker().
*******************************************************************************/
int TwoRowChainer (const char *r1, const char *r2)
{
  // Declarations
  int map[9], cycle[4], len, abc, i, j, n, x;

  for(i = 0; i < 9; i++)
	  map[r1[i] - 1] = i;

  // Loop over possible start-of-cycle positions
  for(n = i = 0; i < 9; i++)
  {
    // Check for start-of-cycle
    if(map[x = r1[i] - 1] < 0)
		continue;

    // Initialize cycle
    abc = i / 3;			// ABC value
    len = 0;				// Cycle length
    j = i;					// Chain index

    // Chain values to find next cycle entry
    newentry:
    map[x] = -1;
    len++;
    if (map[x = r2[j] - 1] >= 0) {
		abc = (abc << 2) | (j = map[x]) / 3;
		goto newentry;
	}

    // End-of-cycle
    cycle[n++] = abc | (len << 18);

  } // i

  // Pack cycles into one word (use binary '11' as separator)
  for(x = i = 0; i < n; i++)
  {
    len = cycle[i] >> 18;
    x = (x << (2 * len)) | (cycle[i] & 0777777);
    x = (x << 2) | 03;
  }

  // Return packed cycle structure
  return(x);

} // End-of-function


/*******************************************************************************
				Fatal Error()
*******************************************************************************/
void FatalError (const char *routine)
{
  // Declarations
  //int corner[9] = { 0,3,6,27,30,33,54,57,60 };
  //int offset[9] = { 0,1,2,9,10,11,18,19,20 };
  //int *puz, *p1, *p, ii, i, j, n, k, x, kt=0;

  fprintf(stderr,"*** Sorry, but the program has died!\n");
  fprintf(stderr,"Error detected by routine \"%s\".\n\n",routine);

 // // Output the puzzle
 // puz = TwoPuz[0];
 // for ( n=i=0; i < 9; i++ )
 // { for ( j=0; j < 3; j++ )
 //   { for ( k=0; k < 3; k++ )
 //     { x = puz[n++];
	//if ( (x<0) || (x>9) )
	//   { fprintf(stderr," x"); kt++; } else
	//if ( x==0 )
	//   fprintf(stderr," ."); else
	//   fprintf(stderr," %d",x);
 //     }
 //     fprintf(stderr," ");
 //   }
 //   if ( (i%3)==2 ) fprintf(stderr,"\n");
 //   fprintf(stderr,"\n");
 // }

 // // Out-of-range clue found.
 // if (kt)
 // { fprintf(stderr,"*** %d out-of-range clue(s) ('x') found!\n",kt);
 //   exit(666);
 // }

 // // Check for duplicates
 // for ( ii=0; ii < 9; ii++ )
 // {
 //   // Check row for duplicates
 //   p = puz + (9*ii);
 //   for ( x=i=0; i < 9; i++ )
 //   { if ( p[i] && ((x>>p[i])&1) )
 //        { fprintf(stderr,"Duplicate clue(s) found in row %d.\n",ii+1);
 //          exit(666); }
 //     x |= (1<<p[i]);
 //   }

 //   // Check column for duplicates
 //   p = puz+ii;
 //   for ( x=i=0; i < 81; i += 9 )
 //   { if ( p[i] && ((x>>p[i])&1) )
 //        { fprintf(stderr,"Duplicate clue(s) found in row %d.\n",ii+1);
 //          exit(666); }
 //     x |= (1<<p[i]);
 //   }

 //   // Check box for duplicates
 //   p1 = puz + corner[ii];
 //   for ( x=i=0; i < 9; i++ )
 //   { p = p1 + offset[i];
 //     if ( (*p) && ((x>>(*p))&1) )
 //        { fprintf(stderr,"Duplicate clue(s) found in box %d.\n",ii+1);
 //          exit(666); }
 //     x |= (1<<(*p));
 //   }

 // } // ii

 // fprintf(stderr,"*** How strange, the puzzle seems to be error free!\n");
 // fprintf(stderr,"    Contact: Mike Deverin at devfam47@gmail.com\n");
  exit(0);
}

//void getBandRanks() {
//	for(int i = 0; i < 416; i++) {
//		int r12 = TwoRowRanker(&bands[i][0], &bands[i][9]);
//		int r13 = TwoRowRanker(&bands[i][0], &bands[i][18]);
//		int r23 = TwoRowRanker(&bands[i][9], &bands[i][18]);
//		printf("%d\t%d\t%d\t%d\n", i, r12, r13, r23);
//	}
//}
//extern void test() {getBandRanks();}

//Rank    Grids
//0   488843212
//1   519376601
//2    68420279
//3  3867134553
//4   475255539
//5    30137776
//6    22132319
//7     1376973
//8       31987
//9       20184
//10       1077
//11         34
//12          3
//13          1
//14          0
//=============
//   5472730538
