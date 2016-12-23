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

#include "anchor5.h"

//` ` 5 | ` ` ` | ` ` ` 
//` ` ` | ` ` 5 | ` ` ` 
//` ` ` | ` ` ` | ` ` 5 
//------------------------ 
//` 5 ` | 1 2 3 | ` ` ` 
//` ` ` | 4 5 6 | ` ` ` 
//` ` ` | 7 8 9 | ` 5 ` 
//------------------------ 
//5 ` ` | ` ` ` | ` ` ` 
//` ` ` | 5 ` ` | ` ` ` 
//` ` ` | ` ` ` | 5 ` `

//123......
//45612....
//789...1..
//.1.......
//....1....
//.......1.
//..1......
//.....1...
//........1
//123......45612....789...1...1...........1...........1...1...........1...........1

//123......
//4561ab...
//789...1..
//.1.......
//....1....
//.......1.
//..1......
//.....1...
//........1

//a = 2 => N braid or N rope at top band, fixed
//b = 3 => N rope at top band (24%)


//123......
//45612....
//789...1..
//.1.A.....
//....1....
//...E.B.1.
//..1...C..
//.....1.D.
//........1

void transformer5::byGrid(const char* sol)
{
	int			b;
	int			d = -1;
	int			i;
	int			j;
	int			k;
	int			l;

	int				pr;
	int				pc;

	transformer5 test;

	/* initialize the worst canonical candidate */
	aut = 1;
	box = 0;
	for(i = 0; i < 9; i++) {
		map[i] = 10;
		row[i] = col[i] = i;
	}
	map[9] = 10;

	/* search over all boxes */

	for(b = 0; b < 18; b++) { //box position along with transposition
		test.box = b;
		for(pr = 0; pr < 6; pr++) { //row permutation
			test.row[0] = tc.perm[pr][0];
			test.row[1] = tc.perm[pr][1];
			test.row[2] = tc.perm[pr][2];
			for(pc = 0; pc < 6; pc++) { //column permutation

				/* initialize the map */

				test.col[0] = tc.perm[pc][0];
				test.col[1] = tc.perm[pc][1];
				test.col[2] = tc.perm[pc][2];
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[0]]]] = 1;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[1]]]] = 2;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[2]]]] = 3;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[0]]]] = 4;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[1]]]] = 5;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[2]]]] = 6;
				test.map[(int)sol[tc.swap[b][test.row[2]][test.col[0]]]] = 7;
				test.map[(int)sol[tc.swap[b][test.row[2]][test.col[1]]]] = 8;
				test.map[(int)sol[tc.swap[b][test.row[2]][test.col[2]]]] = 9;

				/* order cols 4&5&6 */

				k = 0;
				for(i = 3; i < 9; i++) {
					if(test.map[(int)sol[tc.swap[b][test.row[1]][i]]] == 1) {
						l = tc.boxOffset[i];
						for(j = 0; j < 3; j++) {
							switch (test.map[(int)sol[tc.swap[b][test.row[1]][j+l]]])
							{
							case 1:
								test.col[3] = j+l;
								k |= 01;
								break;
							case 2:
								test.col[4] = j+l;
								k |= 02;
								break;
							default:
								test.col[5] = j+l;
								k |= 04;
								break;
							}
						}
						break;
					}
				}
				if(k != 7) {
					goto next;
				}

				/* order row 4 */

				for(i = 3; i < 9; i++) {
					if(test.map[(int)sol[tc.swap[b][i][test.col[1]]]] == 1) {
						test.row[3] = k = i;
						break;
					}
				}

				/* order rows 5&6 */

				if(test.map[(int)sol[tc.swap[b][tc.part[k][0]][test.col[4]]]] == 1) {
					test.row[4] = tc.part[k][0];
					test.row[5] = tc.part[k][1];
				}
				else if(test.map[(int)sol[tc.swap[b][tc.part[k][1]][test.col[4]]]] == 1) {
					test.row[4] = tc.part[k][1];
					test.row[5] = tc.part[k][0];
				}
				else {
					goto next;
				}

				/* order row 7 */

				for(i = 2; i < 5; i++) {
					if(test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[2]]]] == 1) {
						test.row[6] = l = tc.part[k][i];
						break;
					}
				}

				/* order rows 8&9 */

				if(test.map[(int)sol[tc.swap[b][tc.part[l][0]][test.col[5]]]] == 1) {
					test.row[7] = tc.part[l][0];
					test.row[8] = tc.part[l][1];
				}
				else if(test.map[(int)sol[tc.swap[b][tc.part[l][1]][test.col[5]]]] == 1) {
					test.row[7] = tc.part[l][1];
					test.row[8] = tc.part[l][0];
				}
				else {
					goto next;
				}

				/* order cols 7&8&9 */

				for(i = 6; i < 9; i++) {
					if(test.map[(int)sol[tc.swap[b][test.row[2]][i]]] == 1) {
						test.col[6] = i;
					}
					else if(test.map[(int)sol[tc.swap[b][test.row[5]][i]]] == 1) {
						test.col[7] = i;
					}
					else if(test.map[(int)sol[tc.swap[b][test.row[8]][i]]] == 1) {
						test.col[8] = i;
					}
					else {
						goto next;
					}
				}

				//TODO: define distance, implement and assign to d
				//d = 0;

				///* check rows 8&9 */

				//if(!d) {
				//	for(i = 7; i < 9; i++)
				//		for(j = 0; j < 9; j++)
				//			if((d = (int)test.map[sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[sol[tc.swap[box][row[i]][col[j]]]])) {
				//				if(d > 0) {
				//					goto next;
				//				}
				//				i = 9;
				//				break;
				//			}
				//}

				/* check for automorphism */

				if(d < 0) {
					clearAutomorphs();
					*this = test;
					aut = 1;
					d = 0; //init "this" on first hit
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

//void transformer5::reverseTransform(const char *in, char *out) {
//	int labelMap[10];
//	for(int i = 0; i < 10; i++) {
//		labelMap[map[i]] = i;
//	}
//	for(int r = 0; r < 9; r++) {
//		for(int c = 0; c < 9; c++) {
//			out[tc.swap[box][row[r]][col[c]]] = labelMap[in[9 * r + c]];
//		}
//	}
//}

void anchor5(const char *in, char *out) {
	transformer5 tr;
	tr.byGrid(in);
	tr.transform(in, out);
}

//#include <stdio.h>
//extern void test() {
//	char in[81], out[648][81];
//	for(int i = 0; i < 81; i++) {
//		in[i] = "594231786786945132123768954965173248378492561241856397432619875619587423857324619"[i] - '0';
//	}
//	transformer5 tr;
//	tr.byGrid(in);
//	for(int i = 0; i < 81; i++) {
//		printf("%c", in[i] + '0');
//	}
//	printf("\t%d\n", tr.aut);
//	tr.transformAll(in, out[0]);
//	for(int a = 0; a < tr.aut; a++) {
//		for(int i = 0; i < 81; i++) {
//			printf("%c", out[a][i] + '0');
//		}
//		printf("\n");
//	}
//}
