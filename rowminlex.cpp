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

#include <memory.h>
#include "rowminlex.h"
#include "tables.h"

void inline sort3desc(const int* s, int *t) {
	if(s[0] >= s[1]) {
		if(s[1] >= s[2]) {
			t[0] = s[0], t[1] = s[1], t[2] = s[2];
		}
		else if(s[0] >= s[2]) {
			t[0] = s[0], t[1] = s[2], t[2] = s[1];
		}
		else {
			t[0] = s[2], t[1] = s[0], t[2] = s[1];
		}
	}
	else if(s[1] >= s[2]) {
		if(s[0] >= s[2]) {
			t[0] = s[1], t[1] = s[0], t[2] = s[2];
		}
		else {
			t[0] = s[1], t[1] = s[2], t[2] = s[0];
		}
	}
	else {
		t[0] = s[2], t[1] = s[1], t[2] = s[0];
	}
}

void transformer::byGrid(const char* sol)
{
//	register int			b;
//	register int			d;
//	register int			i;
//	register int			j;
//	register int			k;
//	register int			l;
//	register unsigned int	v;
	int			b;
	int			d;
	int			i;
	int			j;
	int			k;
	int			l;
	unsigned int	v;

	int				pr;
	int				pc;

	transformer test;

	/* initialize the worst canonical candidate */
	aut = 1;
	box = 0;
	for(i = 0; i < 9; i++) {
		map[i] = 10;
		row[i] = col[i] = i;
	}
	map[9] = 10;

	/* search over all boxes */

	for(b = 0; b < 18; b++) {
		for(pr = 0; pr < 6; pr++) {
			for(pc = 0; pc < 6; pc++) {
				test.box = b;

				/* initialize the map */

				for(i = 0; i < 3; i++) {
					test.row[i] = tc.perm[pr][i];
					test.col[i] = tc.perm[pc][i];
				}
				for(i = 1; i <= 9; i++)
					test.map[i] = 0;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[0]]]] = 1;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[1]]]] = 2;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[2]]]] = 3;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[0]]]] = 4;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[1]]]] = 5;

				/* map value 6 and order cols 4&5&6 */

				k = 0;
				for(i = 3; i < 9; i++) {
					if(test.map[(int)sol[tc.swap[b][test.row[0]][i]]] == 4) {
						l = tc.boxOffset[i];
						for(j = 0; j < 3; j++) {
							switch (test.map[(int)sol[tc.swap[b][test.row[0]][j+l]]])
							{
							case 4:
								test.col[3] = j+l;
								k |= 01;
								break;
							case 5:
								test.col[4] = j+l;
								k |= 02;
								break;
							case 0:
								test.map[(int)sol[tc.swap[b][test.row[0]][test.col[5]=j+l]]] = 6;
								k |= 04;
								break;
							}
						}
						break;
					}
				}
				if(k != 7)
					goto next;

				/* map values 7&8&9 */

				for(j = 2; j < 6; j++) {
					if(!test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]]) {
						test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] = k++;
					}
				}

				/* check row 2 cols 3&4&5&6 */

				for(j = 2; j < 6; j++) {
					if((d = (int)test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[1]][col[j]]]])) {
						if(d > 0) {
							goto next;
						}
						break;
					}
				}

				/* order cols 7&8&9 */

				j = (l == 3) ? 6 : 3;
				k = j+1;
				l = j+2;
				if(test.map[(int)sol[tc.swap[b][test.row[0]][j]]] > test.map[(int)sol[tc.swap[b][test.row[0]][k]]]) {
					v = j; j = k; k = v;
				}
				if(test.map[(int)sol[tc.swap[b][test.row[0]][j]]] > test.map[(int)sol[tc.swap[b][test.row[0]][l]]]) {
					v = j; j = l; l = v;
				}
				if(test.map[(int)sol[tc.swap[b][test.row[0]][k]]] > test.map[(int)sol[tc.swap[b][test.row[0]][l]]]) {
					v = k; k = l; l = v;
				}
				test.col[6] = j;
				test.col[7] = k;
				test.col[8] = l;

				/* check row 2 cols 7&8&9 */

				if(!d) {
					for(j = 6; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[1]][col[j]]]])) {
							if(d > 0)
								goto next;
							break;
						}
					}
				}

				/* check row 3 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][test.row[2]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[2]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}

				/* done with the first band */

				/* order row 4 */

				v = 10;
				i = 9;
				while (--i >= 3) {
					if(test.map[(int)sol[tc.swap[b][i][test.col[0]]]] < v) {
						v = test.map[(int)sol[tc.swap[b][i][test.col[0]]]];
						k = i;
					}
				}

				/* check row 4 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][k][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[3]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}
				test.row[3] = k;

				/* order rows 5&6 */

				if(test.map[(int)sol[tc.swap[b][tc.part[k][0]][test.col[0]]]] < test.map[(int)sol[tc.swap[b][tc.part[k][1]][test.col[0]]]]) {
					test.row[4] = tc.part[k][0];
					test.row[5] = tc.part[k][1];
				}
				else {
					test.row[4] = tc.part[k][1];
					test.row[5] = tc.part[k][0];
				}

				/* check rows 5&6 */

				if(!d) {
					for(i = 4; i < 6; i++) {
						for(j = 0; j < 9; j++) {
							if((d = (int)test.map[(int)sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[i]][col[j]]]])) {
								if(d > 0) {
									goto next;
								}
								i = 6;
								break;
							}
						}
					}
				}

				/* done with the second band */

				/* order row 7 */

				v = 10;
				for(i = 2; i < 5; i++) {
					if(test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[0]]]] < v) {
						v = test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[0]]]];
						l = tc.part[k][i];
					}
				}

				/* check row 7 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][l][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[6]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}
				test.row[6] = l;

				/* order rows 8&9 */

				if(test.map[(int)sol[tc.swap[b][tc.part[l][0]][test.col[0]]]] < test.map[(int)sol[tc.swap[b][tc.part[l][1]][test.col[0]]]]) {
					test.row[7] = tc.part[l][0];
					test.row[8] = tc.part[l][1];
				}
				else {
					test.row[7] = tc.part[l][1];
					test.row[8] = tc.part[l][0];
				}

				/* check rows 8&9 */

				if(!d) {
					for(i = 7; i < 9; i++)
						for(j = 0; j < 9; j++)
							if((d = (int)test.map[(int)sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[i]][col[j]]]])) {
								if(d > 0) {
									goto next;
								}
								i = 9;
								break;
							}
				}

				/* check for automorphism */

				if(d) {
					clearAutomorphs();
					*this = test;
					aut = 1;
				}
				else {
					addAutomorph(&test);
					aut++;
				}
next:
				;
			}
		}
	}
	map[0] = 0;
}

struct subtransformer {
	int transpose;
	int bands[3];
	int stacks[3];
	int rows[3][3];
	int cols[3][3];
	int labels[9];
};

//const subtransformer esubtransformer = { //-1 = yet unknown
//	0,
//	{-1,-1,-1},
//	{-1,-1,-1},
//	{-1,-1,-1,-1,-1,-1,-1,-1,-1},
//	{-1,-1,-1,-1,-1,-1,-1,-1,-1},
//	{-1,-1,-1,-1,-1,-1,-1,-1,-1}
//};

void transformer::byPuzzle(const char* sol) {}

//void transformer::byPuzzle(const char* sol)
//{
//	register int			b;
//	register int			d;
//	register int			i;
//	register int			j;
//	register int			k;
//	register int			l;
//	register int			v;
//
//	int				pr;
//	int				pc;
//
//	//transformer test;
//
//	//do some preprocessing
//
//	//count the non-givens per triplet
//	int maxNonGivensPerTriplet[54];
//	//int maxNonGivensPerTriplets = 0;
//	for(i = 0; i < 54; i++) {
//		maxNonGivensPerTriplet[i] = 0;
//	}
//
//	for(i = 0; i < 81; i++) {
//		if(0 == sol[i]) {
//			maxNonGivensPerTriplet[3 * rowByCellIndex[i] + stackByCellIndex[i]]++;
//			maxNonGivensPerTriplet[27 + 3 * colByCellIndex[i] + bandByCellIndex[i]]++;
//		}
//	}
//
//	//for(i = 0; i < 54; i++) {
//	//	if(maxNonGivensPerTriplets < maxNonGivensPerTriplet[i]) {
//	//		maxNonGivensPerTriplets = maxNonGivensPerTriplet[i];
//	//	}
//	//}
//
//	//count the maximum adjacent non-givens per row & col
//	//obtain max empty cells at the start of each band
//	int maxNonGivensPerRow[18];
//	int maxNonGivensPerBand[6];
//	int maxNonGivensPerTranspose[2];
//	int maxNonGivensPerRows = 0;
//	int maxNonGivensPerBands = 0;
//	int maxNonGivensPerTransposes = 0;
//	int ord3[3];
//	for(int band = 0, rc = 0; band < 6; band++) {
//		for(int rowInBand = 0; rowInBand < 3; rowInBand++, rc++) {
//			sort3desc(&maxNonGivensPerTriplet[3 * rc], ord3);
//			int r = 0;
//			for(int j = 0; j < 3; j++) {
//				r += ord3[j];
//				if(ord3[j] < 3) //non-empty triplet
//					break;
//			}
//			maxNonGivensPerRow[rc] = r;
//			if(maxNonGivensPerRows < r) {
//				maxNonGivensPerRows = r;
//			}
//		}
//		sort3desc(&maxNonGivensPerRow[rc - 3], ord3);
//		int r = 0;
//		for(int j = 0; j < 3; j++) {
//			r += ord3[j];
//			if(ord3[j] < 9) //non-empty row
//				break;
//		}
//		maxNonGivensPerBand[band] = r;
//		if(maxNonGivensPerBands < r) {
//			maxNonGivensPerBands = r;
//		}
//	}
//	for(int tr = 0; tr < 2; tr++) {
//		sort3desc(&maxNonGivensPerBand[3 * tr], ord3);
//		int r = 0;
//		for(int j = 0; j < 3; j++) {
//			r += ord3[j];
//			if(ord3[j] < 27) //non-empty band
//				break;
//		}
//		maxNonGivensPerTranspose[tr] = r;
//		if(maxNonGivensPerTransposes < r) {
//			maxNonGivensPerTransposes = r;
//		}
//	}
//	//the information for the maximum non-givens at top is collected
//
//
//	/* initialize the worst canonical candidate */
//	//aut = 1;
//	//box = 0;
//	//for(i = 0; i < 9; i++) {
//	//	map[i] = 10;
//	//	row[i] = col[i] = i;
//	//}
//	//map[9] = 10;
//	subtransformer test;
//	char ttsol[81];
//	const char (*tsol)[3][3][3][3] = (const char(*)[3][3][3][3])sol;
//	for(int tr = 0; tr < 2; tr++) { //transpose
//		if(maxNonGivensPerTranspose[tr] < maxNonGivensPerTransposes)
//			continue;
//		test = esubtransformer; //structure copy
//		test.transpose = tr;
//		if(tr) {
//			tsol = (const char(*)[3][3][3][3])ttsol; //points to transposed puzzle
//			for(int r = 0; r < 9; r++) {
//				for(int c = 0; c < 9; c++) {
//					(*(char(*)[9][9])ttsol)[r][c] = (*(const char(*)[9][9])sol)[c][r];
//				}
//			}
//		}
//		for(int topBand = 0; topBand < 3; topBand++) {
//			if(maxNonGivensPerBand[3 * tr + topBand] < maxNonGivensPerBands)
//				continue;
//			test.bands[0] = topBand;
//			int rowBase = 9 * tr + 3 * topBand;
//			if(maxNonGivensPerBand[3 * tr + topBand] == 27) { //empty top band, row order doesn't matter
//				test.rows[0][0] = test.rows[0][1] = test.rows[0][2] = rowBase; //map the first row of the empty band to all rows of band 1
//			}
//			else { //iterate row order of the top band
//				for(int topRow = 0; topRow < 3; topRow++) {
//					if(maxNonGivensPerRow[rowBase + topRow] < maxNonGivensPerRows)
//						continue;
//					test.rows[0][0] = topRow;
//					switch(maxNonGivensPerRows / 3) { //empty triplets in the top row
//						case 3: //empty top row
//							if(maxNonGivensPerBands >= 18) { //2 empty rows
//								for(int secRow = 0; secRow < 3; secRow++) {
//									if(secRow == topRow)
//										continue;
//									if(maxNonGivensPerRow[rowBase + topRow] != 9)
//										continue;
//									test.rows[0][1] = secRow;
//									for(int thirdRow = 0; thirdRow < 3; thirdRow++) {
//										if(thirdRow == topRow)
//											continue;
//										if(thirdRow == secRow)
//											continue;
//										test.rows[0][2] = thirdRow;
//										int tripletBase = 27 * tr + 9 * topBand + 3 * thirdRow;
//										switch((maxNonGivensPerBands - 18) / 3) { //empty triplets in the third row
//											case 2: //map the third stack and iterate its columns
//												for(int nonemptyTriplet = 0; nonemptyTriplet < 3; nonemptyTriplet++) {
//													if(maxNonGivensPerTriplet[tripletBase + nonemptyTriplet] < 3) {
//														test.stacks[2] = nonemptyTriplet;
//														switch(maxNonGivensPerTriplet[tripletBase + nonemptyTriplet]) {
//															case 2:
//																for(int c = 0; c < 3; c++) {
//																	if(char l1 = *tsol[topBand][thirdRow][nonemptyTriplet][c]) {
//																		test.cols[2][2] = c;
//																		test.labels[l1] = 1;
//																		break;
//																	}
//																}
//																//iterateRow();
//																break;
//														}
//														break;
//													}
//												}
//												break;
//											case 1:
//												break;
//											default: //0
//												break;
//										}
//										break;
//									}
//									break;
//								}
//							}
//							break;
//						case 2: //2 empty triplets
//							break;
//						case 1: //1 empty triplet
//							break;
//						default: //0 empty triplets
//					}
//				}
//			}
//
//
//			for(int midBand = 0; midBand < 3; midBand++) {
//				if(midBand == topBand)
//					continue;
//				if(maxNonGivensPerBand[3 * tr + topBand] == 27) { //empty top band
//					if(maxNonGivensPerBand[3 * tr + midBand] != 27 && //non-empty band
//						maxNonGivensPerBand[3 * tr + midBand] + 27 < maxNonGivensPerTransposes) //this midBand is a wrong choice
//						continue;
//				}
//				else { //non-empty top band
//				}
//				for(int bottomBand = 0; bottomBand < 3; bottomBand++) {
//					if(bottomBand == topBand || bottomBand == midBand)
//						continue;
//					//we entered a band permutation starting with max empty cells
//					//row, stack, and column permutations are still unknown
//				}
//			}
//		}
//	}
//
//	return;
//
//	/* search over all boxes */
//
////	for(b = 0; b < 18; b++) { //box permutation, 9 + transposed 9 = 18, which box is placed at top left
////		int tr = b / 9;
////		if(maxNonGivensPerTranspose[tr] < maxNonGivensPerTransposes)
////			continue;
////		int band = b / 3;
////		if(maxNonGivensPerBand[band] < maxNonGivensPerBands)
////			continue;
////		test.box = b;
////		for(pr = 0; pr < 6; pr++) { //row permutation for top left box
////			static const int topRowPerPermutationOfRow[6] = {0,0,1,2,1,2};
////			int toprow = band * 3 + topRowPerPermutationOfRow[pr];
////			if(maxNonGivensPerRow[toprow] < maxNonGivensPerRows)
////				continue;
////	//{ /* perm */
////	//	{ 0, 1, 2 },
////	//	{ 0, 2, 1 },
////	//	{ 1, 0, 2 },
////	//	{ 1, 2, 0 },
////	//	{ 2, 0, 1 },
////	//	{ 2, 1, 0 }
////	//},
////
////			for(i = 0; i < 3; i++) { // for first 3 rows, the permutation is the same as of the top left box
////				test.row[i] = tc.perm[pr][i];
////			}
////			for(pc = 0; pc < 6; pc++) { //column permutation for top left box
////
////				/* initialize the map */
////
////				for(i = 0; i < 3; i++) { // for first 3 columns, the permutation is the same as of the top left box
////					test.col[i] = tc.perm[pc][i];
////				}
////				for(i = 1; i <= 9; i++)
////					test.map[i] = 0;
////				test.map[sol[tc.swap[b][test.row[0]][test.col[0]]]] = 1;
////				test.map[sol[tc.swap[b][test.row[0]][test.col[1]]]] = 2;
////				test.map[sol[tc.swap[b][test.row[0]][test.col[2]]]] = 3;
////				test.map[sol[tc.swap[b][test.row[1]][test.col[0]]]] = 4;
////				test.map[sol[tc.swap[b][test.row[1]][test.col[1]]]] = 5;
////
////				/* map value 6 and order cols 4&5&6 */
////
////				k = 0;
////				for(i = 3; i < 9; i++) {
////					if(test.map[sol[tc.swap[b][test.row[0]][i]]] == 4) {
////						l = tc.boxOffset[i];
////						for(j = 0; j < 3; j++) {
////							switch (test.map[sol[tc.swap[b][test.row[0]][j+l]]])
////							{
////							case 4:
////								test.col[3] = j+l;
////								k |= 01;
////								break;
////							case 5:
////								test.col[4] = j+l;
////								k |= 02;
////								break;
////							case 0:
////								test.map[sol[tc.swap[b][test.row[0]][test.col[5]=j+l]]] = 6;
////								k |= 04;
////								break;
////							}
////						}
////						break;
////					}
////				}
////				if(k != 7)
////					goto next;
////
////				/* map values 7&8&9 */
////
////				for(j = 2; j < 6; j++) {
////					if(!test.map[sol[tc.swap[b][test.row[1]][test.col[j]]]]) {
////						test.map[sol[tc.swap[b][test.row[1]][test.col[j]]]] = k++;
////					}
////				}
////
////				/* check row 2 cols 3&4&5&6 */
////
////				for(j = 2; j < 6; j++) {
////					if((d = (int)test.map[sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[sol[tc.swap[box][row[1]][col[j]]]])) {
////						if(d > 0) {
////							goto next;
////						}
////						break;
////					}
////				}
////
////				/* order cols 7&8&9 */
////
////				j = (l == 3) ? 6 : 3;
////				k = j+1;
////				l = j+2;
////				if(test.map[sol[tc.swap[b][test.row[0]][j]]] > test.map[sol[tc.swap[b][test.row[0]][k]]]) {
////					v = j; j = k; k = v;
////				}
////				if(test.map[sol[tc.swap[b][test.row[0]][j]]] > test.map[sol[tc.swap[b][test.row[0]][l]]]) {
////					v = j; j = l; l = v;
////				}
////				if(test.map[sol[tc.swap[b][test.row[0]][k]]] > test.map[sol[tc.swap[b][test.row[0]][l]]]) {
////					v = k; k = l; l = v;
////				}
////				test.col[6] = j;
////				test.col[7] = k;
////				test.col[8] = l;
////
////				/* check row 2 cols 7&8&9 */
////
////				if(!d) {
////					for(j = 6; j < 9; j++) {
////						if((d = (int)test.map[sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[sol[tc.swap[box][row[1]][col[j]]]])) {
////							if(d > 0)
////								goto next;
////							break;
////						}
////					}
////				}
////
////				/* check row 3 */
////
////				if(!d) {
////					for(j = 0; j < 9; j++) {
////						if((d = (int)test.map[sol[tc.swap[b][test.row[2]][test.col[j]]]] - (int)map[sol[tc.swap[box][row[2]][col[j]]]])) {
////							if(d > 0) {
////								goto next;
////							}
////							break;
////						}
////					}
////				}
////
////				/* done with the first band */
////
////				/* order row 4 */
////
////				v = 10;
////				i = 9;
////				while (--i >= 3) {
////					if(test.map[sol[tc.swap[b][i][test.col[0]]]] < v) {
////						v = test.map[sol[tc.swap[b][i][test.col[0]]]];
////						k = i;
////					}
////				}
////
////				/* check row 4 */
////
////				if(!d) {
////					for(j = 0; j < 9; j++) {
////						if((d = (int)test.map[sol[tc.swap[b][k][test.col[j]]]] - (int)map[sol[tc.swap[box][row[3]][col[j]]]])) {
////							if(d > 0) {
////								goto next;
////							}
////							break;
////						}
////					}
////				}
////				test.row[3] = k;
////
////				/* order rows 5&6 */
////
////				if(test.map[sol[tc.swap[b][tc.part[k][0]][test.col[0]]]] < test.map[sol[tc.swap[b][tc.part[k][1]][test.col[0]]]]) {
////					test.row[4] = tc.part[k][0];
////					test.row[5] = tc.part[k][1];
////				}
////				else {
////					test.row[4] = tc.part[k][1];
////					test.row[5] = tc.part[k][0];
////				}
////
////				/* check rows 5&6 */
////
////				if(!d) {
////					for(i = 4; i < 6; i++) {
////						for(j = 0; j < 9; j++) {
////							if((d = (int)test.map[sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[sol[tc.swap[box][row[i]][col[j]]]])) {
////								if(d > 0) {
////									goto next;
////								}
////								i = 6;
////								break;
////							}
////						}
////					}
////				}
////
////				/* done with the second band */
////
////				/* order row 7 */
////
////				v = 10;
////				for(i = 2; i < 5; i++) {
////					if(test.map[sol[tc.swap[b][tc.part[k][i]][test.col[0]]]] < v) {
////						v = test.map[sol[tc.swap[b][tc.part[k][i]][test.col[0]]]];
////						l = tc.part[k][i];
////					}
////				}
////
////				/* check row 7 */
////
////				if(!d) {
////					for(j = 0; j < 9; j++) {
////						if((d = (int)test.map[sol[tc.swap[b][l][test.col[j]]]] - (int)map[sol[tc.swap[box][row[6]][col[j]]]])) {
////							if(d > 0) {
////								goto next;
////							}
////							break;
////						}
////					}
////				}
////				test.row[6] = l;
////
////				/* order rows 8&9 */
////
////				if(test.map[sol[tc.swap[b][tc.part[l][0]][test.col[0]]]] < test.map[sol[tc.swap[b][tc.part[l][1]][test.col[0]]]]) {
////					test.row[7] = tc.part[l][0];
////					test.row[8] = tc.part[l][1];
////				}
////				else {
////					test.row[7] = tc.part[l][1];
////					test.row[8] = tc.part[l][0];
////				}
////
////				/* check rows 8&9 */
////
////				if(!d) {
////					for(i = 7; i < 9; i++)
////						for(j = 0; j < 9; j++)
////							if((d = (int)test.map[sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[sol[tc.swap[box][row[i]][col[j]]]])) {
////								if(d > 0) {
////									goto next;
////								}
////								i = 9;
////								break;
////							}
////				}
////
////				/* check for automorphism */
////
////				if(d) {
////					clearAutomorphs();
////					*this = test;
////					aut = 1;
////				}
////				else {
////					addAutomorph(&test);
////					aut++;
////				}
////next:
////				;
////			}
////		}
////	}
////	map[0] = 0;
//}


void transformer::byBand(const char* sol, const int band)
{
//	register int			b;
//	register int			d;
//	register int			i;
//	register int			j;
//	register int			k;
//	register int			l;
//	register unsigned int	v;
	int			b;
	int			d;
	int			i;
	int			j;
	int			k;
	int			l;
	unsigned int	v;

	int				pr;
	int				pc;

	static const int bandbox[6][3] = {{0,1,2},{3,4,5},{6,7,8},{9,10,11},{12,13,14},{15,16,17}};

	transformer test;

	/* initialize the worst canonical candidate */
	aut = 1;
	box = 0;
	for(i = 0; i < 9; i++) {
		map[i] = 10;
		row[i] = col[i] = i;
	}
	map[9] = 10;

	/* search over all boxes */

	for(b = bandbox[band][0]; b <= bandbox[band][2]; b++) {
		for(pr = 0; pr < 6; pr++) {
			for(pc = 0; pc < 6; pc++) {
				test.box = b;

				/* initialize the map */

				for(i = 0; i < 3; i++) {
					test.row[i] = tc.perm[pr][i];
					test.col[i] = tc.perm[pc][i];
				}
				for(i = 1; i <= 9; i++)
					test.map[i] = 0;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[0]]]] = 1;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[1]]]] = 2;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[2]]]] = 3;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[0]]]] = 4;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[1]]]] = 5;

				/* map value 6 and order cols 4&5&6 */

				k = 0;
				for(i = 3; i < 9; i++) {
					if(test.map[(int)sol[tc.swap[b][test.row[0]][i]]] == 4) {
						l = tc.boxOffset[i];
						for(j = 0; j < 3; j++) {
							switch (test.map[(int)sol[tc.swap[b][test.row[0]][j+l]]])
							{
							case 4:
								test.col[3] = j+l;
								k |= 01;
								break;
							case 5:
								test.col[4] = j+l;
								k |= 02;
								break;
							case 0:
								test.map[(int)sol[tc.swap[b][test.row[0]][test.col[5]=j+l]]] = 6;
								k |= 04;
								break;
							}
						}
						break;
					}
				}
				if(k != 7)
					goto next;

				/* map values 7&8&9 */

				for(j = 2; j < 6; j++) {
					if(!test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]]) {
						test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] = k++;
					}
				}

				/* check row 2 cols 3&4&5&6 */

				for(j = 2; j < 6; j++) {
					if((d = (int)test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[1]][col[j]]]])) {
						if(d > 0) {
							goto next;
						}
						break;
					}
				}

				/* order cols 7&8&9 */

				j = (l == 3) ? 6 : 3;
				k = j+1;
				l = j+2;
				if(test.map[(int)sol[tc.swap[b][test.row[0]][j]]] > test.map[(int)sol[tc.swap[b][test.row[0]][k]]]) {
					v = j; j = k; k = v;
				}
				if(test.map[(int)sol[tc.swap[b][test.row[0]][j]]] > test.map[(int)sol[tc.swap[b][test.row[0]][l]]]) {
					v = j; j = l; l = v;
				}
				if(test.map[(int)sol[tc.swap[b][test.row[0]][k]]] > test.map[(int)sol[tc.swap[b][test.row[0]][l]]]) {
					v = k; k = l; l = v;
				}
				test.col[6] = j;
				test.col[7] = k;
				test.col[8] = l;

				/* check row 2 cols 7&8&9 */

				if(!d) {
					for(j = 6; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[1]][col[j]]]])) {
							if(d > 0)
								goto next;
							break;
						}
					}
				}

				/* check row 3 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][test.row[2]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[2]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}

				/* done with the first band */

				/* order row 4 */

				v = 10;
				i = 9;
				while (--i >= 3) {
					if(test.map[(int)sol[tc.swap[b][i][test.col[0]]]] < v) {
						v = test.map[(int)sol[tc.swap[b][i][test.col[0]]]];
						k = i;
					}
				}

				/* check row 4 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][k][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[3]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}
				test.row[3] = k;

				/* order rows 5&6 */

				if(test.map[(int)sol[tc.swap[b][tc.part[k][0]][test.col[0]]]] < test.map[(int)sol[tc.swap[b][tc.part[k][1]][test.col[0]]]]) {
					test.row[4] = tc.part[k][0];
					test.row[5] = tc.part[k][1];
				}
				else {
					test.row[4] = tc.part[k][1];
					test.row[5] = tc.part[k][0];
				}

				/* check rows 5&6 */

				if(!d) {
					for(i = 4; i < 6; i++) {
						for(j = 0; j < 9; j++) {
							if((d = (int)test.map[(int)sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[i]][col[j]]]])) {
								if(d > 0) {
									goto next;
								}
								i = 6;
								break;
							}
						}
					}
				}

				/* done with the second band */

				/* order row 7 */

				v = 10;
				for(i = 2; i < 5; i++) {
					if(test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[0]]]] < v) {
						v = test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[0]]]];
						l = tc.part[k][i];
					}
				}

				/* check row 7 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][l][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[6]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}
				test.row[6] = l;

				/* order rows 8&9 */

				if(test.map[(int)sol[tc.swap[b][tc.part[l][0]][test.col[0]]]] < test.map[(int)sol[tc.swap[b][tc.part[l][1]][test.col[0]]]]) {
					test.row[7] = tc.part[l][0];
					test.row[8] = tc.part[l][1];
				}
				else {
					test.row[7] = tc.part[l][1];
					test.row[8] = tc.part[l][0];
				}

				/* check rows 8&9 */

				if(!d) {
					for(i = 7; i < 9; i++)
						for(j = 0; j < 9; j++)
							if((d = (int)test.map[(int)sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[i]][col[j]]]])) {
								if(d > 0) {
									goto next;
								}
								i = 9;
								break;
							}
				}

				/* check for automorphism */

				if(d) {
					clearAutomorphs();
					*this = test;
					aut = 1;
				}
				else {
					addAutomorph(&test);
					aut++;
				}
next:
				;
			}
		}
	}
	map[0] = 0;
}

void transformer::transform(const char *in, char *out) {
	char* s = out;
	bool isPuzzle = false;
	for(int r = 0; r < 9; r++) {
		for(int c = 0; c < 9; c++) {
			char mapped = map[(int)in[tc.swap[box][row[r]][col[c]]]];
			isPuzzle |= (mapped == 0);
			*s++ = mapped;
		}
	}
	if(next && isPuzzle) {
		//transforming a puzzle of automorphic grid
		char aout[81];
		next->transform(in, aout); //minimal of the rest of the transformations
		for(int i = 0; i < 81; i++) {
			if(out[i] > aout[i]) {
				//choose the second isomorph
				memcpy(out, aout, 81);
				break;
			}
			else if(out[i] < aout[i]) {
				//choose the first isomorph
				break;
			}
		}
	}
}

void transformer::transformAll(const char *in, char *out) {
	char* s = out;
	for(int r = 0; r < 9; r++) {
		for(int c = 0; c < 9; c++) {
			char mapped = map[(int)in[tc.swap[box][row[r]][col[c]]]];
			*s++ = mapped;
		}
	}
	if(next) {
		next->transformAll(in, s);
	}
}

void transformer::reverseTransform(const char *in, char *out) {
	int labelMap[10];
	for(int i = 0; i < 10; i++) {
		labelMap[map[i]] = i;
	}
	for(int r = 0; r < 9; r++) {
		for(int c = 0; c < 9; c++) {
			out[tc.swap[box][row[r]][col[c]]] = labelMap[(int)in[9 * r + c]];
		}
	}
}

void rowminlex(const char *in, char *out) {
	transformer tr;
	tr.byGrid(in);
	tr.transform(in, out);
}
void subcanon2(const char *in, char *out) {
	transformer tr;
	tr.byPuzzle(in);
	tr.transform(in, out);
}

//void bandminlex(const char *in, const int band, char *out) {
//	transformer tr;
//	tr.byBand(in, band);
//	tr.transform(in, out);
//}

int getBandNum(const char* sol) {
	for(int b = 0; b < 416; b++) {
		if(0 == memcmp(bands[b], sol, 27)) {
			return b + 1;
		}
	}
	return 0;
}

void swapBands23(char* sol) {
	char buf[27];
	memcpy(buf, sol + 27, 27);
	memcpy(sol + 27, sol + 54, 27);
	memcpy(sol + 54, buf, 27);
}


//#include <stdio.h>
//extern void test() {
//	char in[81], out[81], out2[81];
//	for(int i = 0; i < 81; i++) {
//		in[i] = "594231786786945132123768954965173248378492561241856397432619875619587423857324619"[i] - '0';
//	}
//	for(int i = 0; i < 81; i++) {
//		printf("%c", in[i] + '0');
//	}
//	printf("\n");
//	transformer tr;
//	tr.byGrid(in);
//	tr.transform(in, out);
//	for(int i = 0; i < 81; i++) {
//		printf("%c", out[i] + '0');
//	}
//	printf("\n");
//	tr.reverseTransform(out, out2);
//	for(int i = 0; i < 81; i++) {
//		printf("%c", out2[i] + '0');
//	}
//	printf("\n");
//}
