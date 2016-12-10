#include "solver.h"
#include "grid.h"
#include "options.h"
#include "ch81.h"
#include <algorithm>

//uaCollector::uaCollector(grid *theGrid)
//: g(*theGrid), unknowns(NULL), unknownsSize(81), uniqueSize(0), firstInvalidUa(maxSolutions), numInvalidUas(0) {
//	uaArray = bm128::allocate(maxSolutions); //TODO: reduce the array size
//	uaMaxSize = opt.uaOpt->maxuasize;
//	uaLimitedBySize = (uaMaxSize < 81);
//}
//void x(uaCollector &c, const char *pp) {
//	int rxto[10];
//	char p[81];
//	for(rxto[1] = 1; rxto[1] <= 1; rxto[1]++) {
//		for(rxto[2] = 1; rxto[2] <= 2; rxto[2]++) {
//			if(rxto[2] == rxto[1]) continue;
//			for(rxto[3] = 1; rxto[3] <= 3; rxto[3]++) {
//				if(rxto[3] == rxto[1] || rxto[3] == rxto[2]) continue;
//				for(rxto[4] = 1; rxto[4] <= 4; rxto[4]++) {
//					if(rxto[4] == rxto[1] || rxto[4] == rxto[2] || rxto[4] == rxto[3]) continue;
//					for(rxto[5] = 1; rxto[5] <= 5; rxto[5]++) {
//						if(rxto[5] == rxto[1] || rxto[5] == rxto[2] || rxto[5] == rxto[3] || rxto[5] == rxto[4]) continue;
//						for(rxto[6] = 1; rxto[6] <= 9; rxto[6]++) {
//							if(rxto[6] == rxto[1] || rxto[6] == rxto[2] || rxto[6] == rxto[3] || rxto[6] == rxto[4] || rxto[6] == rxto[5]) continue;
//							for(rxto[7] = 1; rxto[7] <= 9; rxto[7]++) {
//								if(rxto[7] == rxto[1] || rxto[7] == rxto[2] || rxto[7] == rxto[3] || rxto[7] == rxto[4] || rxto[7] == rxto[5] || rxto[7] == rxto[6]) continue;
//								for(rxto[8] = 1; rxto[8] <= 9; rxto[8]++) {
//									if(rxto[8] == rxto[1] || rxto[8] == rxto[2] || rxto[8] == rxto[3] || rxto[8] == rxto[4] || rxto[8] == rxto[5] || rxto[8] == rxto[6] || rxto[8] == rxto[7]) continue;
//									for(rxto[9] = 1; rxto[9] <= 9; rxto[9]++) {
//										if(rxto[9] == rxto[1] || rxto[9] == rxto[2] || rxto[9] == rxto[3] || rxto[9] == rxto[4] || rxto[9] == rxto[5] || rxto[9] == rxto[6] || rxto[9] == rxto[7] || rxto[9] == rxto[8]) continue;
//										for(int i = 0; i < 81; i++) {
//											p[i] = rxto[pp[i]];
//										}
//										c.addMorph(p);
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}
//
//void uaCollector::byMorphs() {
//	bool addMe = false;
//	char tp[9][9]; //transposed
//	char (*p99[2])[9][9]; //original & transposed
//	p99[0] = (char (*)[9][9])g.digits;
//	p99[1] = &tp;
//	for(int r = 0; r < 9; r++) { //transpose
//		for(int c = 0; c < 9; c++) {
//			tp[c][r] = (*p99[0])[r][c];
//		}
//	}
//	static const int spi[6][9] = { //perform on all rows on original to permute stacks
//		{0,1,2, 3,4,5, 6,7,8}, //123
//		{0,1,2, 6,7,8, 3,4,5}, //132
//		{3,4,5, 0,1,2, 6,7,8}, //213
//		{3,4,5, 6,7,8, 0,1,2}, //231
//		{6,7,8, 0,1,2, 3,4,5}, //312
//		{6,7,8, 3,4,5, 0,1,2}, //321
//	};
//	static const int cpi[6][3] = {
//		{0,1,2}, //123
//		{0,2,1}, //132
//		{1,0,2}, //213
//		{1,2,0}, //231
//		{2,0,1}, //312
//		{2,1,0}, //321
//	};
//	for(int transpose = 0; transpose < 2; transpose++) {
//		for(int sp = 0; sp < 6; sp++) { //permute stacks
//			char spp[9][9]; //puzzle with permuted stacks
//			for(int r = 0; r < 9; r++) {
//				for(int c = 0; c < 9; c++) {
//					spp[r][c] = (*p99[transpose])[r][spi[sp][c]];
//				}
//			}
//			for(int s1cp = 0; s1cp < 6; s1cp++) { //stack 1 column permutation
//				char s1cpp[9][9]; //puzzle with permuted columns in stack 1
//				memcpy(s1cpp, spp, 81);
//				for(int r = 0; r < 9; r++) {
//					for(int c = 0; c < 3; c++) {
//						s1cpp[r][c] = spp[r][cpi[s1cp][c]];
//					}
//				}
//				for(int s2cp = 0; s2cp < 6; s2cp++) { //stack 2 column permutation
//					char s2cpp[9][9]; //puzzle with permuted columns in stack 2
//					memcpy(s2cpp, s1cpp, 81);
//					for(int r = 0; r < 9; r++) {
//						for(int c = 0; c < 3; c++) {
//							s2cpp[r][c + 3] = s1cpp[r][cpi[s2cp][c] + 3];
//						}
//					}
//					for(int s3cp = 0; s3cp < 6; s3cp++) { //stack 3 column permutation
//						char s3cpp[9][9]; //puzzle with permuted columns in stack 3
//						memcpy(s3cpp, s2cpp, 81);
//						for(int r = 0; r < 9; r++) {
//							for(int c = 0; c < 3; c++) {
//								s3cpp[r][c + 6] = s2cpp[r][cpi[s3cp][c] + 6];
//							}
//						}
//						for(int bp = 0; bp < 6; bp++) { //band permutation
//							char bpp[9][9]; //puzzle with permuted bands
//							for(int b = 0; b < 3; b++) {
//								memcpy(bpp[3 * b], s3cpp[3 * cpi[bp][b]], 27);
//							}
//							char b1rpp[9][9]; //puzzle with permuted rows in band 1
//							memcpy(b1rpp[3], bpp[3], 54); //rows 4..9
//							for(int b1rp = 0; b1rp < 6; b1rp++) { //band 1 row permutation
//								for(int r = 0; r < 3; r++) {
//									memcpy(b1rpp[r], bpp[cpi[b1rp][r]], 9);
//								}
//								char b2rpp[9][9]; //puzzle with permuted rows in band 2
//								memcpy(b2rpp, b1rpp, 81);
//								for(int b2rp = 0; b2rp < 6; b2rp++) { //band 2 row permutation
//									for(int r = 0; r < 3; r++) {
//										memcpy(b2rpp[r + 3], b1rpp[cpi[b2rp][r] + 3], 9);
//									}
//									char b3rpp[9][9]; //puzzle with permuted rows in band 3
//									memcpy(b3rpp, b2rpp, 54); //rows 1..6
//									for(int b3rp = 0; b3rp < 6; b3rp++) { //band 3 row permutation
//										for(int r = 0; r < 3; r++) {
//											memcpy(b3rpp[r + 6], b2rpp[cpi[b3rp][r] + 6], 9);
//										}
//										x(*this, (char*)b3rpp);
//										//addMorph((char*)b3rpp);
//									}
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}
//
uaCollector::uaCollector(grid *theGrid, const unsigned char *theUnknowns, const int theUnknownsSize)
: g(*theGrid), unknowns(theUnknowns), unknownsSize(theUnknownsSize), uniqueSize(0), firstInvalidUa(maxSolutions), numInvalidUas(0) {
	uaArray = bm128::allocate(maxSolutions); //TODO: reduce the array size
	uaMaxSize = opt.uaOpt != NULL ? opt.uaOpt->maxuasize : 90;
	uaLimitedBySize = (uaMaxSize < 81);
}
uaCollector::~uaCollector() {
	bm128::deallocate(uaArray);
}

bool uaCollector::addSolution(const char *sol) {
	//compose bitmask with differences between original grid & solution
	bm128 us;
	us.clear();
	int uaSize = 0;
	if(uaLimitedBySize) {
		for(int i = 0; i < unknownsSize; i++) {
			if(sol[unknowns[i]] != g.digits[unknowns[i]]) {
				us.setBit(unknowns[i]);
				if(++uaSize > uaMaxSize) {
					return false; //skip larger UA
				}
			}
		}
	}
	else {
		for(int i = 0; i < unknownsSize; i++) {
			if(sol[unknowns[i]] != g.digits[unknowns[i]]) {
				us.setBit(unknowns[i]);
			}
		}
	}
	//do some tests
	for(int y = 0; y < uniqueSize; y++) {
		if(uaArray[y].isSubsetOf(us)) {
			return false; //skip supersets & duplicates
		}
		if(us.isSubsetOf(uaArray[y])) {
			if(!uaArray[y].isInvalid()) {
				uaArray[y].invalidate(); //mark the superset (many can exist)
				if(firstInvalidUa > y)
					firstInvalidUa = y;
				numInvalidUas++;
			}
		}
	}
	//passed the tests, add at the end of list
	if(numInvalidUas > 500 || uniqueSize >= maxSolutions) {
		//attempt compressing the uaArray by removing the invalid elements
		int compressedSize = firstInvalidUa;
		for(int i = firstInvalidUa + 1; i < uniqueSize; i++) {
			if(!uaArray[i].isInvalid()) { //skip marked
				uaArray[compressedSize++] = uaArray[i];
			}
		}
		if(compressedSize  >= maxSolutions) {
			//nothing to do, return error
			return true;
		}
		uniqueSize = compressedSize;
		firstInvalidUa = maxSolutions;
		numInvalidUas = 0;
		//fprintf(stderr, "*");
	}
	uaArray[uniqueSize++] = us;
	return false;
}
unsigned long long uaCollector::runSolver() {
	unsigned long long solutions;
	char gg[81];
	//copy the grid
	for(int i = 0; i < 81; i++)
		gg[i] = g.digits[i];
	//clear the cells of interest
	for(int i = 0; i < unknownsSize; i++)
		gg[unknowns[i]] = 0;
	solutions = solve(g.gridBM, gg, this); //for each solution except original it calls addSolution
	return solutions;
}
void uaCollector::addUaToGrid() {
	uset ua;
	for(int x = 0; x < uniqueSize; x++) {
		if(!uaArray[x].isInvalid()) { //skip marked
			ua = uaArray[x];
			ua.positionsByBitmap();
			g.usetsBySize.insert(ua);
		}
	}
}

/////////// grid
const char* grid::fileUASuffix = ".unav.txt";
const char* grid::filePuzSuffix = ".puzzles.txt";
const char* grid::fileDigitSuffix = ".digits.txt";
const char* grid::file2dSuffix = ".digitpairs.txt";
const char* grid::file3dSuffix = ".digittriplets.txt";
const char* grid::fileBoxSuffix = ".boxes.txt";
const char* grid::file2bSuffix = ".boxpairs.txt";
const char* grid::file3bSuffix = ".boxtriplets.txt";
const char* grid::fileBandSuffix = ".bands.txt";

void grid::findUA2digits() {
	// By Guenter Stertenbrink, sterten@aol.com
	// Website: http://magictour.free.fr/sudoku.htm
	int G[88][88],U[88],V[18];

	for(int i = 0; i < 88; i++)
		for(int j = 0; j < 88; j++)
			G[i][i] = 0;
	for(int i = 0; i < 81; i++) {
		for(int j = 0; j < 81; j++) {
			if(i != j && (rowByCellIndex[i] == rowByCellIndex[j] || colByCellIndex[i] == colByCellIndex[j] || boxByCellIndex[i] == boxByCellIndex[j])) {
				G[i][j] = 1; //cells at i and j are in a group (row or col or box)
			}
		}
	}

	for(int i1 = 1; i1 <= 8; i1++) {
		for(int i2 = i1 + 1; i2 <= 9; i2++) { //a pair of digits i1, i2
			for(int k = 0, z = 0; k < 81; k++) {
				if(digits[k] == i1 || digits[k] == i2) {
					V[z++] = k; //cell list for this pair
				}
			}

			for(int i = 0; i < 81; i++) {
				U[i] = 0;
			}

			//U[0] = 1;
			int c = 0;

			while(1) {
				c++;
				int k, found;
				for(k = 0; k < 18 && U[V[k]] != 0; k++)
					;
				if(k >= 18)
					break;

				U[V[k]] = c;

				do {
					found = 0;
					for(int u = 0; u < 18; u++)
						for(int v = 0; v < 18; v++) {
							if(G[V[u]][V[v]] > 0 && U[V[u]] == 0 && U[V[v]] == c) {
								found = 1;
								U[V[u]] = c;
							}
							if(G[V[u]][V[v]] > 0 && U[V[u]] == c && U[V[v]] == 0) {
								found = 1;
								U[V[v]] = c;
							}
						}
				} while(found);
			}

			for(int c1 = 1; c1 < c; c1++) {
				uset us;
				us.clear();
				for(int i = 0; i < 9; i++) {
					for(int j = 0; j < 9; j++) {
						if(U[i * 9 + j] == c1) {
							us.setBit(i * 9 + j);
						}
					}
				}
				us.positionsByBitmap();
				//char buf[1000];
				//us.toString(buf);
				//printf("%s\n", buf);
				usetsBySize.insert(us);
			}
		}
	}
}

void grid::findUA4cells() {
	static const int gr4unav[81][6][3] =
	{
		{{12, 3, 9},{13, 4, 9},{14, 5, 9},{28, 1,27},{37, 1,36},{46, 1,45}},
		{{12, 3,10},{13, 4,10},{14, 5,10},{29, 2,28},{38, 2,37},{47, 2,46}},
		{{12, 3,11},{13, 4,11},{14, 5,11},{27, 0,29},{36, 0,38},{45, 0,47}},
		{{15, 6,12},{16, 7,12},{17, 8,12},{31, 4,30},{40, 4,39},{49, 4,48}},
		{{15, 6,13},{16, 7,13},{17, 8,13},{32, 5,31},{41, 5,40},{50, 5,49}},
		{{15, 6,14},{16, 7,14},{17, 8,14},{30, 3,32},{39, 3,41},{48, 3,50}},
		{{ 9, 0,15},{10, 1,15},{11, 2,15},{34, 7,33},{43, 7,42},{52, 7,51}},
		{{ 9, 0,16},{10, 1,16},{11, 2,16},{35, 8,34},{44, 8,43},{53, 8,52}},
		{{ 9, 0,17},{10, 1,17},{11, 2,17},{33, 6,35},{42, 6,44},{51, 6,53}},
		{{21,12,18},{22,13,18},{23,14,18},{28,10,27},{37,10,36},{46,10,45}},
		{{21,12,19},{22,13,19},{23,14,19},{29,11,28},{38,11,37},{47,11,46}},
		{{21,12,20},{22,13,20},{23,14,20},{27, 9,29},{36, 9,38},{45, 9,47}},
		{{24,15,21},{25,16,21},{26,17,21},{31,13,30},{40,13,39},{49,13,48}},
		{{24,15,22},{25,16,22},{26,17,22},{32,14,31},{41,14,40},{50,14,49}},
		{{24,15,23},{25,16,23},{26,17,23},{30,12,32},{39,12,41},{48,12,50}},
		{{18, 9,24},{19,10,24},{20,11,24},{34,16,33},{43,16,42},{52,16,51}},
		{{18, 9,25},{19,10,25},{20,11,25},{35,17,34},{44,17,43},{53,17,52}},
		{{18, 9,26},{19,10,26},{20,11,26},{33,15,35},{42,15,44},{51,15,53}},
		{{ 3, 0,21},{ 4, 0,22},{ 5, 0,23},{28,19,27},{37,19,36},{46,19,45}},
		{{ 3, 1,21},{ 4, 1,22},{ 5, 1,23},{29,20,28},{38,20,37},{47,20,46}},
		{{ 3, 2,21},{ 4, 2,22},{ 5, 2,23},{27,18,29},{36,18,38},{45,18,47}},
		{{ 6, 3,24},{ 7, 3,25},{ 8, 3,26},{31,22,30},{40,22,39},{49,22,48}},
		{{ 6, 4,24},{ 7, 4,25},{ 8, 4,26},{32,23,31},{41,23,40},{50,23,49}},
		{{ 6, 5,24},{ 7, 5,25},{ 8, 5,26},{30,21,32},{39,21,41},{48,21,50}},
		{{ 0, 6,18},{ 1, 6,19},{ 2, 6,20},{34,25,33},{43,25,42},{52,25,51}},
		{{ 0, 7,18},{ 1, 7,19},{ 2, 7,20},{35,26,34},{44,26,43},{53,26,52}},
		{{ 0, 8,18},{ 1, 8,19},{ 2, 8,20},{33,24,35},{42,24,44},{51,24,53}},

		{{39,30,36},{40,31,36},{41,32,36},{55,28,54},{64,28,63},{73,28,72}},
		{{39,30,37},{40,31,37},{41,32,37},{56,29,55},{65,29,64},{74,29,73}},
		{{39,30,38},{40,31,38},{41,32,38},{54,27,56},{63,27,65},{72,27,74}},
		{{42,33,39},{43,34,39},{44,35,39},{58,31,57},{67,31,66},{76,31,75}},
		{{42,33,40},{43,34,40},{44,35,40},{59,32,58},{68,32,67},{77,32,76}},
		{{42,33,41},{43,34,41},{44,35,41},{57,30,59},{66,30,68},{75,30,77}},
		{{36,27,42},{37,28,42},{38,29,42},{61,34,60},{70,34,69},{79,34,78}},
		{{36,27,43},{37,28,43},{38,29,43},{62,35,61},{71,35,70},{80,35,79}},
		{{36,27,44},{37,28,44},{38,29,44},{60,33,62},{69,33,71},{78,33,80}},
		{{48,39,45},{49,40,45},{50,41,45},{55,37,54},{64,37,63},{73,37,72}},
		{{48,39,46},{49,40,46},{50,41,46},{56,38,55},{65,38,64},{74,38,73}},
		{{48,39,47},{49,40,47},{50,41,47},{54,36,56},{63,36,65},{72,36,74}},
		{{51,42,48},{52,43,48},{53,44,48},{58,40,57},{67,40,66},{76,40,75}},
		{{51,42,49},{52,43,49},{53,44,49},{59,41,58},{68,41,67},{77,41,76}},
		{{51,42,50},{52,43,50},{53,44,50},{57,39,59},{66,39,68},{75,39,77}},
		{{45,36,51},{46,37,51},{47,38,51},{61,43,60},{70,43,69},{79,43,78}},
		{{45,36,52},{46,37,52},{47,38,52},{62,44,61},{71,44,70},{80,44,79}},
		{{45,36,53},{46,37,53},{47,38,53},{60,42,62},{69,42,71},{78,42,80}},
		{{30,27,48},{31,27,49},{32,27,50},{55,46,54},{64,46,63},{73,46,72}},
		{{30,28,48},{31,28,49},{32,28,50},{56,47,55},{65,47,64},{74,47,73}},
		{{30,29,48},{31,29,49},{32,29,50},{54,45,56},{63,45,65},{72,45,74}},
		{{33,30,51},{34,30,52},{35,30,53},{58,49,57},{67,49,66},{76,49,75}},
		{{33,31,51},{34,31,52},{35,31,53},{59,50,58},{68,50,67},{77,50,76}},
		{{33,32,51},{34,32,52},{35,32,53},{57,48,59},{66,48,68},{75,48,77}},
		{{27,33,45},{28,33,46},{29,33,47},{61,52,60},{70,52,69},{79,52,78}},
		{{27,34,45},{28,34,46},{29,34,47},{62,53,61},{71,53,70},{80,53,79}},
		{{27,35,45},{28,35,46},{29,35,47},{60,51,62},{69,51,71},{78,51,80}},

		{{66,57,63},{67,58,63},{68,59,63},{ 1, 0,55},{10, 9,55},{19,18,55}},
		{{66,57,64},{67,58,64},{68,59,64},{ 2, 1,56},{11,10,56},{20,19,56}},
		{{66,57,65},{67,58,65},{68,59,65},{ 0, 2,54},{ 9,11,54},{18,20,54}},
		{{69,60,66},{70,61,66},{71,62,66},{ 4, 3,58},{13,12,58},{22,21,58}},
		{{69,60,67},{70,61,67},{71,62,67},{ 5, 4,59},{14,13,59},{23,22,59}},
		{{69,60,68},{70,61,68},{71,62,68},{ 3, 5,57},{12,14,57},{21,23,57}},
		{{63,54,69},{64,55,69},{65,56,69},{ 7, 6,61},{16,15,61},{25,24,61}},
		{{63,54,70},{64,55,70},{65,56,70},{ 8, 7,62},{17,16,62},{26,25,62}},
		{{63,54,71},{64,55,71},{65,56,71},{ 6, 8,60},{15,17,60},{24,26,60}},
		{{75,66,72},{76,67,72},{77,68,72},{ 1, 0,64},{10, 9,64},{19,18,64}},
		{{75,66,73},{76,67,73},{77,68,73},{ 2, 1,65},{11,10,65},{20,19,65}},
		{{75,66,74},{76,67,74},{77,68,74},{ 0, 2,63},{ 9,11,63},{18,20,63}},
		{{78,69,75},{79,70,75},{80,71,75},{ 4, 3,67},{13,12,67},{22,21,67}},
		{{78,69,76},{79,70,76},{80,71,76},{ 5, 4,68},{14,13,68},{23,22,68}},
		{{78,69,77},{79,70,77},{80,71,77},{ 3, 5,66},{12,14,66},{21,23,66}},
		{{72,63,78},{73,64,78},{74,65,78},{ 7, 6,70},{16,15,70},{25,24,70}},
		{{72,63,79},{73,64,79},{74,65,79},{ 8, 7,71},{17,16,71},{26,25,71}},
		{{72,63,80},{73,64,80},{74,65,80},{ 6, 8,69},{15,17,69},{24,26,69}},
		{{57,54,75},{58,54,76},{59,54,77},{ 1, 0,73},{10, 9,73},{19,18,73}},
		{{57,55,75},{58,55,76},{59,55,77},{ 2, 1,74},{11,10,74},{20,19,74}},
		{{57,56,75},{58,56,76},{59,56,77},{ 0, 2,72},{ 9,11,72},{18,20,72}},
		{{60,57,78},{61,57,79},{62,57,80},{ 4, 3,76},{13,12,76},{22,21,76}},
		{{60,58,78},{61,58,79},{62,58,80},{ 5, 4,77},{14,13,77},{23,22,77}},
		{{60,59,78},{61,59,79},{62,59,80},{ 3, 5,75},{12,14,75},{21,23,75}},
		{{54,60,72},{55,60,73},{56,60,74},{ 7, 6,79},{16,15,79},{25,24,79}},
		{{54,61,72},{55,61,73},{56,61,74},{ 8, 7,80},{17,16,80},{26,25,80}},
		{{54,62,72},{55,62,73},{56,62,74},{ 6, 8,78},{15,17,78},{24,26,78}}
	};
	for(int i = 0; i < 81; i++) {
		int c = digits[i]; //%5 faster than char c
		if(c == digits[gr4unav[i][0][0]]) {
			if(digits[gr4unav[i][0][1]] == digits[gr4unav[i][0][2]]) {
				uset u;
				u.clear();
				u.setBit(i);
				u.setBit(gr4unav[i][0][0]);
				u.setBit(gr4unav[i][0][1]);
				u.setBit(gr4unav[i][0][2]);
				u.positionsByBitmap();
				usetsBySize.insert(u);
			}
		}
		else if(c == digits[gr4unav[i][1][0]]) {
			if(digits[gr4unav[i][1][1]] == digits[gr4unav[i][1][2]]) {
				uset u;
				u.clear();
				u.setBit(i);
				u.setBit(gr4unav[i][1][0]);
				u.setBit(gr4unav[i][1][1]);
				u.setBit(gr4unav[i][1][2]);
				u.positionsByBitmap();
				usetsBySize.insert(u);
			}
		}
		else if(c == digits[gr4unav[i][2][0]]) {
			if(digits[gr4unav[i][2][1]] == digits[gr4unav[i][2][2]]) {
				uset u;
				u.clear();
				u.setBit(i);
				u.setBit(gr4unav[i][2][0]);
				u.setBit(gr4unav[i][2][1]);
				u.setBit(gr4unav[i][2][2]);
				u.positionsByBitmap();
				usetsBySize.insert(u);
			}
		}
		if(c == digits[gr4unav[i][3][0]]) {
			if(digits[gr4unav[i][3][1]] == digits[gr4unav[i][3][2]]) {
				uset u;
				u.clear();
				u.setBit(i);
				u.setBit(gr4unav[i][3][0]);
				u.setBit(gr4unav[i][3][1]);
				u.setBit(gr4unav[i][3][2]);
				u.positionsByBitmap();
				usetsBySize.insert(u);
			}
		}
		else if(c == digits[gr4unav[i][4][0]]) {
			if(digits[gr4unav[i][4][1]] == digits[gr4unav[i][4][2]]) {
				uset u;
				u.clear();
				u.setBit(i);
				u.setBit(gr4unav[i][4][0]);
				u.setBit(gr4unav[i][4][1]);
				u.setBit(gr4unav[i][4][2]);
				u.positionsByBitmap();
				usetsBySize.insert(u);
			}
		}
		else if(c == digits[gr4unav[i][5][0]]) {
			if(digits[gr4unav[i][5][1]] == digits[gr4unav[i][5][2]]) {
				uset u;
				u.clear();
				u.setBit(i);
				u.setBit(gr4unav[i][5][0]);
				u.setBit(gr4unav[i][5][1]);
				u.setBit(gr4unav[i][5][2]);
				u.positionsByBitmap();
				usetsBySize.insert(u);
			}
		}
	}
}
void grid::findUA6cells() {
	//two-digit UA6 temporarily are commented out since they are covered by findUA2digits
	typedef unsigned short uabm;
	static const uabm val2bm[10] =	{0,1,2,4,8,16,32,64,128,256};
	//static const int adjacent[9] = {1,2,0,4,5,3,7,8,6};
	//static const int band[9] = {0,0,0,1,1,1,2,2,2};
	static const int gr6_3unav[27][3] = //compare each of the 27 triplets to downside(rightside) box triplets (box modulo 3)
	{
		{ 9,12,15},{10,13,16},{11,14,17},{ 9,12,15},{10,13,16},{11,14,17},{ 9,12,15},{10,13,16},{11,14,17},
		{18,21,24},{19,22,25},{20,23,26},{18,21,24},{19,22,25},{20,23,26},{18,21,24},{19,22,25},{20,23,26},
		{ 0, 3, 6},{ 1, 4, 7},{ 2, 5, 8},{ 0, 3, 6},{ 1, 4, 7},{ 2, 5, 8},{ 0, 3, 6},{ 1, 4, 7},{ 2, 5, 8}
	};
	static const int gr6_3y[27] = //transposed triplets base
	{
		 0,27,54, 1,28,55, 2,29,56, 3,30,57, 4,31,58, 5,32,59, 6,33,60, 7,34,61, 8,35,62
	};
//	static const int gr6_1unav[27][6] = //compare each of the 27 duets in band 1 to the downside(rightside) two boxes
//	{
//		{27,36,45,54,63,72},{28,37,46,55,64,73},{29,38,47,56,65,74},{30,39,48,57,66,75},{31,40,49,58,67,76},{32,41,50,59,68,77},{33,42,51,60,69,78},{34,43,52,61,70,79},{35,44,53,62,71,80},
//		{27,36,45,54,63,72},{28,37,46,55,64,73},{29,38,47,56,65,74},{30,39,48,57,66,75},{31,40,49,58,67,76},{32,41,50,59,68,77},{33,42,51,60,69,78},{34,43,52,61,70,79},{35,44,53,62,71,80},
//		{27,36,45,54,63,72},{28,37,46,55,64,73},{29,38,47,56,65,74},{30,39,48,57,66,75},{31,40,49,58,67,76},{32,41,50,59,68,77},{33,42,51,60,69,78},{34,43,52,61,70,79},{35,44,53,62,71,80}
//	};
//	static const int gr6_2unav[27][12] = //if a duet from band 1 matches a duet from band 2 ([0..2] or [3..5]),
//										 //check if it matches a duet in respective column in band3 ([6..8] or [9..11])
//	{
//		{28,37,46,29,38,47,56,65,74,55,64,73},{27,36,45,29,38,47,56,65,74,54,63,72},{27,36,45,28,37,46,55,64,73,54,63,72},
//		{31,40,49,32,41,50,59,68,77,58,67,76},{30,39,48,32,41,50,59,68,77,57,66,75},{30,39,48,31,40,49,58,67,76,57,66,75},
//		{34,43,52,35,44,53,62,71,80,61,70,79},{33,42,51,35,44,53,62,71,80,60,69,78},{33,42,51,34,43,52,61,70,79,60,69,78},
//		{28,37,46,29,38,47,56,65,74,55,64,73},{27,36,45,29,38,47,56,65,74,54,63,72},{27,36,45,28,37,46,55,64,73,54,63,72},
//		{31,40,49,32,41,50,59,68,77,58,67,76},{30,39,48,32,41,50,59,68,77,57,66,75},{30,39,48,31,40,49,58,67,76,57,66,75},
//		{34,43,52,35,44,53,62,71,80,61,70,79},{33,42,51,35,44,53,62,71,80,60,69,78},{33,42,51,34,43,52,61,70,79,60,69,78},
//		{28,37,46,29,38,47,56,65,74,55,64,73},{27,36,45,29,38,47,56,65,74,54,63,72},{27,36,45,28,37,46,55,64,73,54,63,72},
//		{31,40,49,32,41,50,59,68,77,58,67,76},{30,39,48,32,41,50,59,68,77,57,66,75},{30,39,48,31,40,49,58,67,76,57,66,75},
//		{34,43,52,35,44,53,62,71,80,61,70,79},{33,42,51,35,44,53,62,71,80,60,69,78},{33,42,51,34,43,52,61,70,79,60,69,78}
//	};

	uabm bm[81];
	uabm triplets_x[27];
	uabm triplets_y[27];
	//uabm duets_x[81];
	//uabm duets_y[81];
	uabm c;
	//uabm cc;
	int i, j, k;
	//int n1 = 0, n2 = 0, n3 = 0, n4 = 0;

	//char const (*m)[9][9];
	//m = (const char (*)[9][9])digits;

	for(i = 0; i < 81; i++) {
		bm[i] = val2bm[(int)digits[i]];
	}
	for(i = j = k = 0; i < 27; i++, j += 3) {
		k = gr6_3y[i];
		triplets_x[i] = bm[j] | bm[j + 1] | bm[j + 2];	//3 triplets per row
		triplets_y[i] = bm[k] | bm[k + 9] | bm[k + 18];	//3 triplets per column
		//duets_x[j] = bm[j] | bm[j + 1];					//3 duets per row per stack
		//duets_x[j + 1] = bm[j + 1] | bm[j + 2];
		//duets_x[j + 2] = bm[j] | bm[j + 2];
		//duets_y[j] = bm[k] | bm[k + 9];					//3 duets per column per band
		//duets_y[j + 1] = bm[k + 9] | bm[k + 18];
		//duets_y[j + 2] = bm[k] | bm[k + 18];
	}

	//in the next loop i is the triplet index [0..26]
	//col search  row search
	//00 01 02    00 03 06 09 12 15 18 21 24
	//03 04 05    01 04 07 10 13 16 19 22 25
	//06 07 08    02 05 08 11 14 17 20 23 26
	//09 10 11
	//12 13 14
	//15 16 17
	//18 19 20
	//21 22 23
	//24 25 26
	for(i = 0; i < 27; i++) {
		//=== first by columns ===
		//type 3
		// ABC *** ***
		// *** *** ***
		// *** *** ***
		//
		// BCA *** ***
		// *** *** ***
		// *** *** ***
		//
		// *** *** ***
		// *** *** ***
		// *** *** ***
		c = triplets_x[i];
		if(c == triplets_x[gr6_3unav[i][0]]) {
			uset u;
			u.clear();
			u.setBit(i * 3);
			u.setBit(i * 3 + 1);
			u.setBit(i * 3 + 2);
			u.setBit(gr6_3unav[i][0] * 3);
			u.setBit(gr6_3unav[i][0] * 3 + 1);
			u.setBit(gr6_3unav[i][0] * 3 + 2);
			u.positionsByBitmap();
			usetsBySize.insert(u);
		}
		if(c == triplets_x[gr6_3unav[i][1]]) {
			uset u;
			u.clear();
			u.setBit(i * 3);
			u.setBit(i * 3 + 1);
			u.setBit(i * 3 + 2);
			u.setBit(gr6_3unav[i][1] * 3);
			u.setBit(gr6_3unav[i][1] * 3 + 1);
			u.setBit(gr6_3unav[i][1] * 3 + 2);
			u.positionsByBitmap();
			usetsBySize.insert(u);
		}
		if(c == triplets_x[gr6_3unav[i][2]]) {
			uset u;
			u.clear();
			u.setBit(i * 3);
			u.setBit(i * 3 + 1);
			u.setBit(i * 3 + 2);
			u.setBit(gr6_3unav[i][2] * 3);
			u.setBit(gr6_3unav[i][2] * 3 + 1);
			u.setBit(gr6_3unav[i][2] * 3 + 2);
			u.positionsByBitmap();
			usetsBySize.insert(u);
		}
		////type 1
		//// AB* *** ***
		//// *** *** ***
		//// *** *** ***
		////
		//// BC* *** ***
		//// *** *** *** or AB,CA,BC
		//// *** *** ***
		////
		//// CA* *** ***
		//// *** *** ***
		//// *** *** ***
		//c = duets_x[i];
		//if(BitCount[cc = c | duets_x[gr6_1unav[i][0]]] == 3) {
		//	if(cc == (c | duets_x[gr6_1unav[i][3]])) { n1++;}
		//	else if(cc == (c | duets_x[gr6_1unav[i][4]])) { n1++;}
		//	else if(cc == (c | duets_x[gr6_1unav[i][5]])) { n1++;}
		//}
		//if(BitCount[cc = c | duets_x[gr6_1unav[i][1]]] == 3) {
		//	if(cc == (c | duets_x[gr6_1unav[i][3]])) { n1++;}
		//	else if(cc == (c | duets_x[gr6_1unav[i][4]])) { n1++;}
		//	else if(cc == (c | duets_x[gr6_1unav[i][5]])) { n1++;}
		//}
		//if(BitCount[cc = c | duets_x[gr6_1unav[i][2]]] == 3) {
		//	if(cc == (c | duets_x[gr6_1unav[i][3]])) { n1++;}
		//	else if(cc == (c | duets_x[gr6_1unav[i][4]])) { n1++;}
		//	else if(cc == (c | duets_x[gr6_1unav[i][5]])) { n1++;}
		//}
		////type 2
		//// AB* *** ***
		//// *** *** ***
		//// *** *** ***
		////
		//// B*A *** ***
		//// *** *** ***
		//// *** *** ***
		////
		//// *AB *** ***
		//// *** *** ***
		//// *** *** ***
		//if(c == duets_x[gr6_2unav[i][0]]) { //compare [0,1,2] to [6,7,8]
		//	if(c == duets_x[gr6_2unav[i][6]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][7]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][8]])
		//		n2++;
		//}
		//else if(c == duets_x[gr6_2unav[i][1]]) {
		//	if(c == duets_x[gr6_2unav[i][6]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][7]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][8]])
		//		n2++;
		//}
		//else if(c == duets_x[gr6_2unav[i][2]]) {
		//	if(c == duets_x[gr6_2unav[i][6]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][7]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][8]])
		//		n2++;
		//}
		//else if(c == duets_x[gr6_2unav[i][3]]) { //compare [3,4,5] to [9,10,11]
		//	if(c == duets_x[gr6_2unav[i][9]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][10]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][11]])
		//		n2++;
		//}
		//else if(c == duets_x[gr6_2unav[i][4]]) {
		//	if(c == duets_x[gr6_2unav[i][9]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][10]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][11]])
		//		n2++;
		//}
		//else if(c == duets_x[gr6_2unav[i][5]]) {
		//	if(c == duets_x[gr6_2unav[i][9]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][10]])
		//		n2++;
		//	else if(c == duets_x[gr6_2unav[i][11]])
		//		n2++;
		//}

		//=== then by rows ===
		//a copy/paste of the code by columns, only _x is replaced with _y
		//type 3
		c = triplets_y[i];
		if(c == triplets_y[gr6_3unav[i][0]]) {
			uset u;
			int i2 = (i % 3) * 27; //band base
			int i1 = i2 + i / 3;   //band base + column offset
			i2 += gr6_3unav[i][0] / 3; //band base + secondary column offset
			u.clear();
			u.setBit(i1);
			u.setBit(i1 + 9);
			u.setBit(i1 + 18);
			u.setBit(i2);
			u.setBit(i2 + 9);
			u.setBit(i2 + 18);
			u.positionsByBitmap();
			usetsBySize.insert(u);
		}
		if(c == triplets_y[gr6_3unav[i][1]]) {
			uset u;
			int i2 = (i % 3) * 27; //band base
			int i1 = i2 + i / 3;   //band base + column offset
			i2 += gr6_3unav[i][1] / 3; //band base + secondary column offset
			u.clear();
			u.setBit(i1);
			u.setBit(i1 + 9);
			u.setBit(i1 + 18);
			u.setBit(i2);
			u.setBit(i2 + 9);
			u.setBit(i2 + 18);
			u.positionsByBitmap();
			usetsBySize.insert(u);
		}
		if(c == triplets_y[gr6_3unav[i][2]]) {
			uset u;
			int i2 = (i % 3) * 27; //band base
			int i1 = i2 + i / 3;   //band base + column offset
			i2 += gr6_3unav[i][2] / 3; //band base + secondary column offset
			u.clear();
			u.setBit(i1);
			u.setBit(i1 + 9);
			u.setBit(i1 + 18);
			u.setBit(i2);
			u.setBit(i2 + 9);
			u.setBit(i2 + 18);
			u.positionsByBitmap();
			usetsBySize.insert(u);
		}
		////type 1
		//c = duets_y[i];
		//if(BitCount[cc = c | duets_y[gr6_1unav[i][0]]] == 3) {
		//	if(cc == (c | duets_y[gr6_1unav[i][3]])) { n1++;}
		//	else if(cc == (c | duets_y[gr6_1unav[i][4]])) { n1++;}
		//	else if(cc == (c | duets_y[gr6_1unav[i][5]])) { n1++;}
		//}
		//if(BitCount[cc = c | duets_y[gr6_1unav[i][1]]] == 3) {
		//	if(cc == (c | duets_y[gr6_1unav[i][3]])) { n1++;}
		//	else if(cc == (c | duets_y[gr6_1unav[i][4]])) { n1++;}
		//	else if(cc == (c | duets_y[gr6_1unav[i][5]])) { n1++;}
		//}
		//if(BitCount[cc = c | duets_y[gr6_1unav[i][2]]] == 3) {
		//	if(cc == (c | duets_y[gr6_1unav[i][3]])) { n1++;}
		//	else if(cc == (c | duets_y[gr6_1unav[i][4]])) { n1++;}
		//	else if(cc == (c | duets_y[gr6_1unav[i][5]])) { n1++;}
		//}
		////type 2
		//if(c == duets_y[gr6_2unav[i][0]]) {
		//	if(c == duets_y[gr6_2unav[i][6]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][7]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][8]])
		//		n2++;
		//}
		//else if(c == duets_y[gr6_2unav[i][1]]) {
		//	if(c == duets_y[gr6_2unav[i][6]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][7]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][8]])
		//		n2++;
		//}
		//else if(c == duets_y[gr6_2unav[i][2]]) {
		//	if(c == duets_y[gr6_2unav[i][6]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][7]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][8]])
		//		n2++;
		//}
		//else if(c == duets_y[gr6_2unav[i][3]]) {
		//	if(c == duets_y[gr6_2unav[i][9]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][10]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][11]])
		//		n2++;
		//}
		//else if(c == duets_y[gr6_2unav[i][4]]) {
		//	if(c == duets_y[gr6_2unav[i][9]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][10]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][11]])
		//		n2++;
		//}
		//else if(c == duets_y[gr6_2unav[i][5]]) {
		//	if(c == duets_y[gr6_2unav[i][9]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][10]])
		//		n2++;
		//	else if(c == duets_y[gr6_2unav[i][11]])
		//		n2++;
		//}
	}
	////type 4
	//// AB* *** ***
	//// *** *** ***
	//// *** *** ***
	////
	//// B** A** ***
	//// *A* B** ***
	//// *** *** ***
	////
	//// *** *** ***
	//// *** *** ***
	//// *** *** ***
	//char digitRow[9][9], digitCol[9][9];
	//int row, col, col2, col3, row2, row3, rowband;
	//char a, b;
	//for(row = 0; row < 9; row++) {
	//	for(col = 0; col < 9; col++) {
	//		a = (*m)[row][col] - 1;
	//		digitRow[a][col] = row;
	//		digitCol[a][row] = col;
	//	}
	//}
	//for(row = 0; row < 9; row++) {
	//	rowband = band[row];
	//	for(col = 0; col < 9; col++) {
	//		a = (*m)[row][col] - 1;
	//		b = (*m)[row][col2 = adjacent[col]] - 1;
	//		row2 = digitRow[a][col2];
	//		col3 = digitCol[b][row2];
	//		if(band[col3] != band[col]) {
	//			row3 = digitRow[a][col3];
	//			if(row3 == digitRow[b][col]) {
	//				if(band[row2] == band[row3]) {
	//					n4++;
	//				}
	//			}
	//		}
	//	}
	//}
}

//// Return whether first element is greater than the second
//bool isBm128Smaller(const bm128 elem1, const bm128 elem2)
//{
//   return elem1.popcount_128() < elem2.popcount_128();
//}

struct bm96 {
	bm128 bytes[6];
	void load(const char *p) {
		bytes[0].loadUnaligned(p + 0 * 16);
		bytes[1].loadUnaligned(p + 1 * 16);
		bytes[2].loadUnaligned(p + 2 * 16);
		bytes[3].loadUnaligned(p + 3 * 16);
		bytes[4].loadUnaligned(p + 4 * 16);
		bytes[5].bitmap128.m128i_u8[0] = *(p + 5 * 16); //81th byte, don't read after the possible buffer end
	}
	void diff(const char *p, bm128& dif) const {
		bm128 pp;
		dif.clear();
		pp.loadUnaligned(p + 0 * 16);
		dif.bitmap128.m128i_u32[0] = bytes[0].diffOctets(pp);
		pp.loadUnaligned(p + 1 * 16);
		dif.bitmap128.m128i_u32[0] |= (bytes[1].diffOctets(pp) << 16);
		pp.loadUnaligned(p + 2 * 16);
		dif.bitmap128.m128i_u32[1] = bytes[2].diffOctets(pp);
		pp.loadUnaligned(p + 3 * 16);
		dif.bitmap128.m128i_u32[1] |= (bytes[3].diffOctets(pp) << 16);
		pp.loadUnaligned(p + 4 * 16);
		dif.bitmap128.m128i_u32[2] = bytes[4].diffOctets(pp);
		//pp.loadUnaligned(p + 5 * 16);
		//dif.bitmap128.m128i_u32[2] |= (bytes[5].diffOctets(pp) << 16);
		if(*(p + 5 * 16) != bytes[5].bitmap128.m128i_u8[0])
			dif.bitmap128.m128i_u32[2] |= (1 << 16);
	}
};

void findUaBySolutions(const char *solList, const int nSol) {
	int uaMaxSize = opt.uaOpt->maxuasize;
	int uaMinSize = opt.uaOpt->minuasize;
	bool uaLimitedBySize = (uaMaxSize < 81);

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
	for(int s = 0; s < nSol; s++) {
		//bm128* uaArray = bm128::allocate(nSol);
		sizedUset* uaArray = (sizedUset*)bm128::allocate(nSol);
		const char *sp = &solList[s * 81];
		bm96 sp96;
		sp96.load(sp);
	//for(sp = solList; sp < send; sp += 81) {
		int nUa = 0;
		int numInvalidUas = 0;
		int firstInvalidUa = nSol;
		const char *ssp, *send = solList + 81 * nSol;
		for(ssp = solList; ssp < send; ssp += 81) {
			if(ssp == sp)
				continue;	//don't compare to itself
			sizedUset &us = uaArray[nUa];
			us.clear();
			if(uaLimitedBySize) {
				int uaSize = 0;
				for(int i = 0; i < 81; i++) {
					if(ssp[i] != sp[i]) {
						us.setBit(i);
						if(++uaSize > uaMaxSize) {
							goto nextSol; //skip large UA
						}
					}
				}
			}
			else {
				//compare in parallel;
				sp96.diff(ssp, us);

				//for(int i = 0; i < 81; i++) {
				//	if(ssp[i] != sp[i]) {
				//		us.setBit(i);
				//	}
				//}
			}
			//do some tests
			for(int y = 0; y < nUa; y++) {
				if(uaArray[y].isSubsetOf(us)) {
					goto nextSol; //skip supersets & duplicates
				}
				if(us.isSubsetOf(uaArray[y])) {
					if(!uaArray[y].isInvalid()) {
						uaArray[y].invalidate(); //mark the superset (many can exist)
						if(firstInvalidUa > y)
							firstInvalidUa = y;
						numInvalidUas++;
					}
				}
			}
			//passed the tests
			us.setSize();
			nUa++;
			// compress uaArray if necessary
			//if(numInvalidUas > 120) { //37"
			//if(numInvalidUas > 5) { //slow"
			if(numInvalidUas > 15) { //23"
			//if(numInvalidUas > 20) { //23"
				//attempt compressing the uaArray by removing the invalid elements
				int compressedSize = firstInvalidUa;
				for(int i = firstInvalidUa + 1; i < nUa; i++) {
					if(!uaArray[i].isInvalid()) { //skip marked
						uaArray[compressedSize++] = uaArray[i];
					}
				}
				nUa = compressedSize;
				firstInvalidUa = nSol;
				numInvalidUas = 0;
				//std::sort(uaArray, uaArray + nUa, isBm128Smaller);
				std::sort(uaArray, uaArray + nUa, sizedUset::isSmaller);
			}
			//if(0 == nUa % 60) { //163"(10000, 60/60), 140"(10000, 15)
			//	std::sort(uaArray, uaArray + nUa, isBm128Smaller);
			//}
nextSol:
			;
		}
		//at this point we have uaArray populated, with possible invalid UA
//#ifdef _OPENMP
//#pragma omp critical
//#endif
		for(int y = 0; y < nUa; y++) {
			sizedUset &u = uaArray[y];
			if(!u.isInvalid()) {
				//int uaSize = 0;
				//int uaSize = u.popcount_128();
				int uaSize = u.getSize();
#if 1
				//hunting large UA sets at this point we have a stream of many many minimal UA
				//let hunt for high-valency UA too
				const int min_valency = 16;
				const int min_valency_size = 20; //check large UA only
				char ip[81]; //inverted multiple-solution puzzle
				int valency = 0;
				if(uaSize >= min_valency_size) {
					for(int i = 0; i < 81; i++) {
						if(u.isBitSet(i)) {
							ip[i] = 0;
						}
						else {
							ip[i] = sp[i];
						}
					}
					valency = (int)solve(ip, INT_MAX);
				}
				if(uaSize >= uaMinSize || valency >= min_valency) {
#else
				if(uaSize >= uaMinSize) {
#endif
					//output the UA
					char p[81];
					for(int i = 0; i < 81; i++) {
						if(u.isBitSet(i)) {
							p[i] = sp[i] + '0';
						}
						else {
							p[i] = '.';
						}
					}
#ifdef _OPENMP
#pragma omp critical
#endif
					{
						if(uaSize >= uaMinSize) {
							printf("%81.81s\n", p);
						}
#if 1
						if(valency >= min_valency) {
							printf("%81.81s@%d\n", p, valency);
						}
#endif
						fflush(NULL);
					}
				}
			}
		}
		bm128::deallocate(uaArray);
	}
}

unsigned long long grid::findUaBySolving(const unsigned char *unknowns, const int unknownsSize) {
	unsigned long long solutions;
	uaCollector c(this, unknowns, unknownsSize);
	solutions = c.runSolver();
	//if(c.uniqueSize == uaCollector::maxSolutions || solutions == INT_MAX) {
	if(c.uniqueSize == uaCollector::maxSolutions || solutions == ULLONG_MAX) {
		//partial processing of solutions leads to non-minimal UA sets
		return 0;
	}
	c.addUaToGrid();
	return solutions;
}

//void grid::unused_findComplementaryUA() {
//	int s1 = static_cast <int>(usetsBySize.size());
//	for(usetListBySize::iterator s = usetsBySize.begin(); s != usetsBySize.end(); s++) {
//		s->digitMask = getDigitMask(*s);
//		uset us;
//		us -= *s; //complementary
//		us &= digitSets[s->digitMask];
//		us.positionsByBitmap();
//		if(us.nbits < 53)
//			findUaBySolving(us.positions, us.nbits);
//		if(usetsBySize.size() - s1 > 50000)
//			break;
//	}
//	//usetsBySize.removeSupersets();
//}
	
//void grid::findUA6digits() {
//	unsigned char hideThem[81];
//	int nOldUA = static_cast<int>(usetsBySize.size());
//	printf("Finding UA by removing occurences of 6 digits.\n");
//	printf("Digits\t\tSol\tNew UA\tTotal UA\n");
//	for(int d1 = 1; d1 <= 4; d1++) {
//		for(int d2 = d1 + 1; d2 <= 5; d2++) {
//			for(int d3 = d2 + 1; d3 <= 6; d3++) {
//				for(int d4 = d3 + 1; d4 <= 7; d4++) {
//					for(int d5 = d4 + 1; d5 <= 8; d5++) {
//						for(int d6 = d5 + 1; d6 <= 9; d6++) {
//							for(int j = 0, k = 0; j < 81; j++) {
//								if(digits[j] == d1 || digits[j] == d2 || digits[j] == d3 || digits[j] == d4 || digits[j] == d5 || digits[j] == d6)
//									hideThem[k++] = j;
//							}
//							int nSol = findUaBySolving(hideThem, 54);
//							int nUA = static_cast<int>(usetsBySize.size());
//							printf("(%d,%d,%d,%d,%d,%d)\t%d\t%d\t%d\n", d1, d2, d3, d4, d5, d6, nSol, nUA - nOldUA, nUA);
//							nOldUA = nUA;
//						}
//					}
//				}
//			}
//		}
//	}
//}
void grid::findUA4digits() {
	unsigned char hideThem[81];
	//int nOldUA = static_cast<int>(usetsBySize.size());
	//printf("Finding UA by removing occurences of 4 digits...");
	//printf("Digits\t\tSol\tNew UA\tTotal UA\n");
	for(int d1 = 1; d1 <= 6; d1++) {
		for(int d2 = d1 + 1; d2 <= 7; d2++) {
			for(int d3 = d2 + 1; d3 <= 8; d3++) {
				for(int d4 = d3 + 1; d4 <= 9; d4++) {
					for(int j = 0, k = 0; j < 81; j++) {
						if(digits[j] == d1 || digits[j] == d2 || digits[j] == d3 || digits[j] == d4)
							hideThem[k++] = j;
					}
					//unsigned long long nSol =
					findUaBySolving(hideThem, 36);
					//int nUA = static_cast<int>(usetsBySize.size());
					//printf("(%d,%d,%d,%d)\t%d\t%d\t%d\n", d1, d2, d3, d4, nSol, nUA - nOldUA, nUA);
					//nOldUA = nUA;
				}
			}
		}
	}
	//int nUA = static_cast<int>(usetsBySize.size());
	//printf(" %d new UA found.\n", nUA - nOldUA);
}
void grid::findUA5digits() {
	unsigned char hideThem[81];
	//int nOldUA = static_cast<int>(usetsBySize.size());
	//printf("Finding UA by removing occurences of 5 digits.\n");
	//printf("Digits\t\tSol\tNew UA\tTotal UA\n");
	for(int d1 = 1; d1 <= 5; d1++) { //5-rookeries
		for(int d2 = d1 + 1; d2 <= 6; d2++) {
			for(int d3 = d2 + 1; d3 <= 7; d3++) {
				for(int d4 = d3 + 1; d4 <= 8; d4++) {
					for(int d5 = d4 + 1; d5 <= 9; d5++) {
						for(int j = 0, k = 0; j < 81; j++) {
							if(digits[j] == d1 || digits[j] == d2 || digits[j] == d3 || digits[j] == d4 || digits[j] == d5)
								hideThem[k++] = j;
						}
						//unsigned long long nSol =
						findUaBySolving(hideThem, 45);
						//int nUA = static_cast<int>(usetsBySize.size());
						//printf("(%d,%d,%d,%d,%d)\t%d\t%d\t%d\n", d1, d2, d3, d4, d5, nSol, nUA - nOldUA, nUA);
						//nOldUA = nUA;
					}
				}
			}
		}
	}
}
//void grid::findUAbyMorph() {
//	uaCollector c(this);
//	c.byMorphs();
//	uset ua;
//	char p[81];
//	for(int x = 0; x < c.uniqueSize; x++) {
//		if(!c.uaArray[x].isInvalid()) { //skip marked
//			ua = c.uaArray[x];
//			ua.positionsByBitmap();
//			ua.toPseudoPuzzle(digits, p);
//			if(2 == solve(p, 3)) { //assume UA with 3+ solutions are non-minimal
//				usetsBySize.insert(ua);
//			}
//		}
//	}
//}
unsigned long long grid::findUAbyPuzzle(const char *puz) {
	unsigned char hideThem[81];
	int k = 0;
	for(int j = 0; j < 81; j++) {
		if(puz[j] == 0)
			hideThem[k++] = j;
	}
	return findUaBySolving(hideThem, k); //nSolutions, 0=error
}
unsigned long long grid::findUAinPuzzle(const char *puz) {
	unsigned char hideThem[81];
	int k = 0;
	for(int j = 0; j < 81; j++) {
		if(puz[j])
			hideThem[k++] = j;
	}
	return findUaBySolving(hideThem, k); //nSolutions, 0=error
}
void grid::findUAbyPuzzleBox(const char *puz) {
	unsigned char hideThem[81];
	for(int box = 0; box < 9; box++) {
		int k = 0;
		for(int j = 0; j < 81; j++) {
			if(puz[j] == 0 && boxByCellIndex[j] != box)
				hideThem[k++] = j;
		}
		findUaBySolving(hideThem, k); //nSolutions, 0=error
	}
}
void grid::findUArandom(const char *puz, int nCells, int nAttempts) {
	//const int nCells = 55;
	if(nCells == -1) nCells = opt.uaOpt->nCells; //54;
	if(nAttempts == -1) nAttempts = opt.uaOpt->nAttempts; //10000;
	unsigned char hideThem[81];
	int nOldUA;
	if(opt.verbose) {
		nOldUA = static_cast<int>(usetsBySize.size());
		fprintf(stderr, "Finding UA by %d times randomly removing values from %d cells...", nAttempts, nCells);
	}
	//printf("Attempt\tSol\tNew UA\tTotal UA\n");
	srand((unsigned)time(NULL));
	bool mask[81];
	for(int a = 0; a < nAttempts; a++) {
		int n = 0;
		for(int i = 0; i < 81; i++)
			mask[i] = false;
		if(puz)
			do {
				int rnd = rand() % 81;
				if(!mask[rnd] && 0 == puz[rnd]) {
					mask[rnd] = true;
					n++;
				}
			} while(n < nCells);
		else
			do {
				int rnd = rand() % 81;
				if(!mask[rnd]) {
					mask[rnd] = true;
					n++;
				}
			} while(n < nCells);
		n = 0;
		for(int i = 0; i < 81; i++)
			if(mask[i]) hideThem[n++] = i;
		//unsigned long long nSol =
		findUaBySolving(hideThem, nCells);
		//int nUA = static_cast<int>(usetsBySize.size());
		//printf("%d\t%d\t%d\t%d\n", a, nSol, nUA - nOldUA, nUA);
		//nOldUA = nUA;
	}
	if(opt.verbose) {
		int nUA = static_cast<int>(usetsBySize.size());
		fprintf(stderr, " %d new UA found.\n", nUA - nOldUA);
	}
}
void grid::findUA2bands() {
	unsigned char hideThem[81];
	int nOldUA = static_cast<int>(usetsBySize.size());
	printf("Finding UA by removing occurences of digits in 2 bands.\n");
	printf("Bands\t\tSol\tNew UA\tTotal UA\n");
	for(int d1 = 1; d1 <= 2; d1++) { //5-rookeries
		for(int d2 = d1 + 1; d2 <= 3; d2++) {
			for(int j = 0, k = 0; j < 81; j++) {
				if(bandByCellIndex[j] == d1 - 1 || bandByCellIndex[j] == d2 - 1)
					hideThem[k++] = j;
			}
			unsigned long long nSol = findUaBySolving(hideThem, 54);
			int nUA = static_cast<int>(usetsBySize.size());
			printf("(%d,%d)\t%llu\t%d\t%d\n", d1, d2, nSol, nUA - nOldUA, nUA);
			nOldUA = nUA;
		}
	}
	printf("Finding UA by removing occurences of digits in 2 stacks.\n");
	printf("Stacks\t\tSol\tNew UA\tTotal UA\n");
	for(int d1 = 1; d1 <= 2; d1++) { //5-rookeries
		for(int d2 = d1 + 1; d2 <= 3; d2++) {
			for(int j = 0, k = 0; j < 81; j++) {
				if(stackByCellIndex[j] == d1 - 1 || stackByCellIndex[j] == d2 - 1)
					hideThem[k++] = j;
			}
			unsigned long long nSol = findUaBySolving(hideThem, 54);
			int nUA = static_cast<int>(usetsBySize.size());
			printf("(%d,%d)\t%llu\t%d\t%d\n", d1, d2, nSol, nUA - nOldUA, nUA);
			nOldUA = nUA;
		}
	}
}

void grid::findUA12() {
#if 1
	int A[ 9 ][ 9 ], A_t[ 9 ][ 9 ];
	for ( int i = 0;  i < 81;  i++ ) {
		A[ i / 9 ][ i % 9 ]   = digits[ i ];
		A_t[ i % 9 ][ i / 9 ] = A[ i / 9 ][ i % 9 ];
	}
	lightweightUsetList lwsets;
	FindUnavoidableSets12( A, lwsets );
	FindUnavoidableSets12( A_t, lwsets, true );
	for(lightweightUsetList::const_iterator u = lwsets.begin(); u != lwsets.end(); u++) {
		uset ua;
		ua.bitmap128 = u->bitmap128;
		ua.positionsByBitmap();
		usetsBySize.insert(ua);
	}
#endif
}

//void grid::findUA6boxes() {
//	unsigned char hideThem[81];
//	int nOldUA = static_cast<int>(usetsBySize.size());
//	printf("Finding UA by removing digits from 6 boxes.\n");
//	printf("Boxes\t\tSol\tNew UA\tTotal UA\n");
//	for(int d1 = 1; d1 <= 4; d1++) { //5-boxes
//		for(int d2 = d1 + 1; d2 <= 5; d2++) {
//			for(int d3 = d2 + 1; d3 <= 6; d3++) {
//				for(int d4 = d3 + 1; d4 <= 7; d4++) {
//					for(int d5 = d4 + 1; d5 <= 8; d5++) {
//						for(int d6 = d5 + 1; d6 <= 9; d6++) {
//							for(int j = 0, k = 0; j < 81; j++) {
//								if(boxByCellIndex[j] == d1 || boxByCellIndex[j] == d2 || boxByCellIndex[j] == d3 || boxByCellIndex[j] == d4 || boxByCellIndex[j] == d5 || boxByCellIndex[j] == d6)
//									hideThem[k++] = j;
//							}
//							int nSol = findUaBySolving(hideThem, 54);
//							int nUA = static_cast<int>(usetsBySize.size());
//							printf("(%d,%d,%d,%d,%d,%d)\t%d\t%d\t%d\n", d1, d2, d3, d4, d5, d6, nSol, nUA - nOldUA, nUA);
//							nOldUA = nUA;
//						}
//					}
//				}
//			}
//		}
//	}
//}

void grid::findUA4boxes() {
	unsigned char hideThem[81];
	//int nOldUA = static_cast<int>(usetsBySize.size());
	//printf("Finding UA by removing digits from 5 boxes.\n");
	//printf("Boxes\t\tSol\tNew UA\tTotal UA\n");
	for(int d1 = 1; d1 <= 6; d1++) { //4-boxes
		for(int d2 = d1 + 1; d2 <= 7; d2++) {
			for(int d3 = d2 + 1; d3 <= 8; d3++) {
				for(int d4 = d3 + 1; d4 <= 9; d4++) {
					for(int j = 0, k = 0; j < 81; j++) {
						if(boxByCellIndex[j] == d1 - 1 || boxByCellIndex[j] == d2 - 1 || boxByCellIndex[j] == d3 - 1 || boxByCellIndex[j] == d4 - 1)
							hideThem[k++] = j;
					}
					unsigned long long nSol = findUaBySolving(hideThem, 45);
					//int nUA = static_cast<int>(usetsBySize.size());
					//printf("(%d,%d,%d,%d)\t%llu\t%d\t%d\n", d1, d2, d3, d4, nSol, nUA - nOldUA, nUA);
					//nOldUA = nUA;
				}
			}
		}
	}
}
void grid::findUA5boxes() {
	unsigned char hideThem[81];
	//int nOldUA = static_cast<int>(usetsBySize.size());
	//printf("Finding UA by removing digits from 5 boxes.\n");
	//printf("Boxes\t\tSol\tNew UA\tTotal UA\n");
	for(int d1 = 1; d1 <= 5; d1++) { //5-boxes
		for(int d2 = d1 + 1; d2 <= 6; d2++) {
			for(int d3 = d2 + 1; d3 <= 7; d3++) {
				for(int d4 = d3 + 1; d4 <= 8; d4++) {
					for(int d5 = d4 + 1; d5 <= 9; d5++) {
						for(int j = 0, k = 0; j < 81; j++) {
							if(boxByCellIndex[j] == d1 - 1 || boxByCellIndex[j] == d2 - 1 || boxByCellIndex[j] == d3 - 1 || boxByCellIndex[j] == d4 - 1 || boxByCellIndex[j] == d5 - 1)
								hideThem[k++] = j;
						}
						unsigned long long nSol = findUaBySolving(hideThem, 45);
						//int nUA = static_cast<int>(usetsBySize.size());
						//printf("(%d,%d,%d,%d,%d)\t%llu\t%d\t%d\n", d1, d2, d3, d4, d5, nSol, nUA - nOldUA, nUA);
						//nOldUA = nUA;
					}
				}
			}
		}
	}
}

void grid::findShortUA() {
	//findUA2digits();
	//findUA6cells();
	findUA4cells();
}
void grid::findInitialUA() {
	//findUA2digits();
	//findUA2bands();
	//findUA5boxes();
	if(opt.uaOpt->digit5Search)
		findUA5digits();
	else if(opt.uaOpt->digit4Search)
		findUA4digits();
	if(opt.uaOpt->randomSearch)
		findUArandom();
	if(opt.uaOpt->unav12)
		findUA12();
	//if(opt.uaOpt->unavmorph)
	//	findUAbyMorph();

	////usetsBySize.removeSupersets();

	//findComplementaryUA();
	//printf("Total %d UA found.\n", static_cast<int>(usetsBySize.size()));
}

void grid::findMaximalCliques() {
	findMaximalCliques(maxClique, usetsBySize);
}
void grid::findMaximalCliques(clique &theClique, usetListBySize &usetsBySize) {
	lightweightCliqueList maximalCliques;
	bm128 *ualist;
	int i = 0, uasize = static_cast <int>(usetsBySize.size());
	int maxUaSize = uasize + 100;
	ualist = (bm128*)_mm_malloc(maxUaSize * sizeof(bm128), 16);
	lightweightClique stackIndexes;
	bm128 stackBitmaps[20], bm;
	int stackTop; //also current clique number - 1
	//copy the unavoidable sets to an array for faster access
	//int maxSize = 13;
	int maxSize = opt.uaOpt->mcnUaSizeLimit; //13
	for(usetListBySize::const_iterator ua = usetsBySize.begin(); ua != usetsBySize.end(); ua++) {
		if(ua->nbits > maxSize)
			break;
		//if(ua->isDisjoint(theClique.fixedClues))
		ualist[i++].bitmap128 = ua->bitmap128;
	}
	uasize = i;
	//printf("Finding Maximal Cliques using unavoidable sets of size up to %d.\n", maxSize);
again:
	bm.clear();
	int mcn = 0;
	stackTop = 0; //also current clique number - 1
	//printf("Using %i unavoidable sets.\n", uasize);
	for(i = 0;; i++) {
		if(i >= uasize) { //something like (i + mcn - stackTop >= uasize) is slower
			//no chance to find larger clique so compare & backtrack
			//check whether we found something valuable
			if(stackTop == mcn) {
				//add the current maximum clique to the list of the known maximals
				maximalCliques.push_back(stackIndexes);
			}
			else if(stackTop > mcn) {
				//found a larger clique
				maximalCliques.clear();
				mcn = stackTop;
				maximalCliques.push_back(stackIndexes);
			}
			//backtrack
			if(stackTop) {
				stackTop--;
				i = stackIndexes.memberIndexes[stackTop];
				bm = stackBitmaps[stackTop];
			}
			else {
				//done
				break;
			}
		}
		else {
			if(bm.isDisjoint(ualist[i])) {
				//an independent set found
				//store the context for backtracking
				stackIndexes.memberIndexes[stackTop] = i;
				stackBitmaps[stackTop] = bm;
				bm |= ualist[i];
				stackTop++;
			}
		}
	}
	//now select one of the possible cliques
	//criterion is minimal combinations
	unsigned long long combinations, minComb;
	int minIndex = -1;
	if(opt.verbose) {
		printf("\nMCN=%i, Maximal Cliques found=%i.\n", mcn, (int)maximalCliques.size());
	}
	char buf[1000];
	for(i = 0; i < (int)maximalCliques.size(); i++) {
		uset ns;
		if(opt.verbose) {
			printf("Clique %3.3d\n", i);
		}
		bm.clear();
		combinations = 1;
		for(int j = 0; j < mcn; j++) {
			ns.bitmap128.m128i_m128i = ualist[maximalCliques[i].memberIndexes[j]].bitmap128.m128i_m128i;
			ns.positionsByBitmap();
			combinations *= ns.nbits;
			if(opt.verbose) {
				ns.toString(buf);
				printf("%s (%d)\n", buf, ns.nbits);
				//printf(" %2.2d", ns.nbits);
			}
			bm |= ns;
		}
		if(minIndex == -1 || minComb > combinations) {
			minIndex = i;
			minComb = combinations;
		}
		if(opt.verbose) {
			printf("%llu combinations.\n\n", combinations);
		}
		if(opt.uaOpt->mcnNoAutoUA)
			continue;
		ns -= bm; //complementary
		ns.clearBits(theClique.fixedClues); //clear the fixed non-clues prior to search for UA there
		ns.positionsByBitmap();
		if(isUA(ns)) {
			MinimizeUA(ns);
			if(theClique.fixedNonClues.nbits) {
				ns.clearBits(theClique.fixedNonClues); //clear the fixed non-clues from the minimal UA found
				ns.positionsByBitmap();
			}
			if(0 == usetsBySize.count(ns)) {
				usetsBySize.insert(ns);
			}
			if(opt.verbose) {
				printf("\nThere is an UA of size %d within the rest of the clues!\n", ns.nbits);
			}
			if(uasize < maxUaSize) {
				ualist[uasize++].bitmap128 = ns.bitmap128;
				if(opt.verbose) {
					printf("\nReprocessing the cliques...\n");
				}
				goto again;
			}
			else {
				if(opt.verbose) {
					printf("\nMore than 100 new UA found, ignoring the rest...\n");
				}
			}
		}
	}
	//minIndex = 2; //debug
	//printf("Choosing Clique %i.\n", minIndex);
	theClique.clear();
	for(i = 0; i < mcn; i++) {
		cliqueMember ns;
		ns = ualist[maximalCliques[minIndex].memberIndexes[i]];
		ns.positionsByBitmap();
		//ns.setNumDisjoints(usetsBySize);
		theClique.insert(ns);
	}
	_mm_free(ualist);
}

void grid::saveUA() {
	FILE *dest;
	char buf[3000];
	strcpy(buf, fname);
	strcat(buf, fileUASuffix);
	dest = fopen(buf, "wt");
	if(opt.verbose) {
		printf("Saving %d UA sets to file %s\n", (int)usetsBySize.size(), buf);
	}
	for(usetListBySize::const_iterator ua = usetsBySize.begin(); ua != usetsBySize.end(); ua++) {
		ua->toString(buf);
		fprintf(dest, "%s\n", buf);
	}
	fclose(dest);
}

void grid::readUAFromFile() {
	readUAFromFileOnly();
	if(usetsBySize.empty()) {
		findInitialUA();
		saveUA();
	}
}
void grid::readUAFromFileOnly() {
	char buf[3000];
	strcpy(buf, fname);
	strcat(buf, fileUASuffix);
	if(opt.verbose) {
		printf("Loading UA sets from file %s\n", buf);
	}
	usetsBySize.ReadFromFile(buf);
}
int grid::readFromFile() {
	if(fname == NULL)
		return -1;
	char buf[3000];
	strcpy(buf, fname);
	strcat(buf, ".txt");
	ifstream file(buf);
	//printf("Reading grid from file %s\n", buf);
	if(! file)
		return -1;
	if(! file.getline(buf, sizeof(buf)))
		return -1;
	return fromString(buf);
}

int grid::fromString(const char* buf) {
	//printf("%s\n", buf);
	for(int i = 0; i < 81; i++) {
		digits[i] = buf[i] - '0';
		if(digits[i] < 0 || digits[i] > 9)
			return -1;
	}
	setBM();
	return 0;
}

void grid::MinimizeUA(uset &ua) const {
	int minPositionIndexToRemove = 0;
	while(minPositionIndexToRemove < ua.nbits) {
		uset oldUA = ua;
		ua.clearBit(ua.positions[minPositionIndexToRemove]);
		//this should work a bit faster than ua.positinsByBitmap()
		for(int j = minPositionIndexToRemove + 1; j < ua.nbits; j++)
			ua.positions[j - 1] = ua.positions[j];
		ua.nbits--;
		if(isUA(ua)) {
			continue;
		}
		//backtrack
		ua = oldUA;
		minPositionIndexToRemove++;
	}
}

//void grid::unused_printStatistics() {
//	printf("\nMCN: %d\n", mcn);
//	printf("Total sets found: %d\n", (int)usetsBySize.size());
//	int cp[81]; // by cell population
//	double cp1[81]; //weighted population
//	int usize[81]; // by size
//	int weight[81]; //number of solutions when this UA not hit
//	int digitPopulation[512];
//	char buf[1000];
//	for(int i = 0; i < 512; i++) {
//		digitPopulation[i] = 0;
//	}
//	for(int i = 0; i < 81; i++) {
//		cp[i] = 0;
//		cp1[i] = 0;
//		usize[i] = 0;
//		//weight[i] = 0;
//	}
//	for(usetListBySize::const_iterator s = usetsBySize.begin(); s != usetsBySize.end(); s++) {
//		int n;
//		n = getDigitMask(*s);
//		digitPopulation[n]++;
//		for(int i = 0; i < 81; i++) {
//			if(s->isBitSet(i)) {
//				cp[i]++;
//				cp1[i] += 1.0 / ((double)s->nbits);
//			}
//		}
//		//uset inverse;
//		//inverse.bitmap128.m128i_m128i = _mm_andnot_si128(s->bitmap128.m128i_m128i, maskffff.m128i_m128i);
//		//inverse.positionsByBitmap();
//		usize[s->nbits]++;
//		//n = getSolutionCount(*s);
//		//if(n > 2) {
//		//	s->toString(buf);
//		//	printf("%d=%s\n", n, buf);
//		//}
//	}
//	printf("\nCell population\nCell\tdigit\t#Unav\t??\n");
//	for(int i = 0; i < 81; i++) {
//		printf("%d\t%d\t%d\t%f\n", i, (int)digits[i], cp[i], cp1[i] / cp[i]);
//	}

//	printf("\nSize distribution\n");
//	for(int i = 0; i < 81; i++) {
//		printf("%d\t%d\n", i, usize[i]);
//	}

//	printf("\nDigit combinations population\nComb\t#Unav\n");
//	for(int i = 0; i < 512; i++) {
//		if(digitPopulation[i]) { //skip zeroes
//			for(int j = 0; j < 9; j++) {
//				if(i & (1 << j)) printf("%d", j + 1);
//			}
//			printf("\t%d\n", digitPopulation[i]);
//		}
//	}
//}

//int grid::unused_getDigitMask(const uset &s) const {
//	int digitMask = 0;
//	for(int i = 0; i < s.nbits; i++) {
//		digitMask |= Digit2Bitmap[digits[s.positions[i]]];
//	}
//	return digitMask;
//}

//void grid::unused_setDigitMasks() {
//	for(usetListBySize::iterator s = usetsBySize.begin(); s != usetsBySize.end(); s++) {
//		s->digitMask = getDigitMask(*s);
//	}
//}

//void grid::unused_calculateDigitSets () {
//	uset s1[9]; //pseudo-sets for each of the digits 1..9
//	for(int i = 0; i < 9; i++) {
//		s1[i].clear();
//	}
//	for(int i = 0; i < 81; i++) {
//		s1[digits[i] - 1].setBit(i);
//	}
//	for(int i = 1; i < 512; i++) {
//		digitSets[i].clear(); //clear
//		digitSets[i].digitMask = i;
//		for(int j = 0; j < 9; j++) {
//			if(i & Digit2Bitmap[j + 1]) { //i has bit j set
//				digitSets[i] |= s1[j];
//			}
//		}
//		digitSets[i].positionsByBitmap(); //also sets nbits
//	}
//}

void grid::ua2puzzle(const uset &s, char *puzzle) const {
	for(int i = 0; i < 81; i++) {
		puzzle[i] = digits[i];
	}
	for(int i = 0; i < s.nbits; i++) {
		puzzle[s.positions[i]] = 0;
	}
};

void grid::ua2InvariantPuzzle(const uset &s, char *puzzle) const {
	for(int i = 0; i < 81; i++) {
		puzzle[i] = 0;
	}
	for(int i = 0; i < s.nbits; i++) {
		puzzle[s.positions[i]] = digits[s.positions[i]];
	}
};

unsigned long long grid::getSolutionCount(const uset &s) const {
	return getSolutionCount(s, INT_MAX);
}

unsigned long long grid::getSolutionCount(const uset &s, const int maxSolutions) const {
	char g[81];
	ua2puzzle(s, g);
	return solve(gridBM, g, maxSolutions);
}

bool grid::isUA(const uset &s) const {
	return getSolutionCount(s, 2) > 1;
}

int grid::checkPuzSetValidity(const bm128 &commonClues, const bm128 *p, const int nPuz, char *res) const {
	//checks each of the puzzles for unique solution
	//stores valid puzzles to buf
	//returns the number of valid puzzles
	int n = 0; //number of valid (unique solution) puzzles
	int nUsets = (int)usetsBySize.size();
	bm128 *uu = bm128::allocate(nUsets);
	nUsets = 0;
	for(usetListBySize::const_iterator u = usetsBySize.begin(); u != usetsBySize.end(); u++) {
		if(u->isDisjoint(commonClues)) {
			uu[nUsets++] = u->bitmap128;
		}
	}
	for(int i = 0; i < nPuz; i++) {
		for(int j = 0; j < nUsets; j++) {
			if(uu[j].isDisjoint(p[i])) {
				goto nextPuz;
			}
		}
		//all known UA are hit
		//check for uniqueness by solving
		char puz[81];
		{
			//bm128 pBM = commonClues | p[i];
			for(int c = 0; c < 81; c++) {
				puz[c] = p[i].isBitSet(c) ? digits[c] : 0;
			}
		}
		//if(1 == solve(gridBM, puz, 2)) {
		if(1 == solve(puz, 2)) {
			memcpy(res + 81 * n, puz, 81);
			n++;
		}
nextPuz:
		;
	}
	bm128::deallocate(uu);
	return n;
}
int grid::checkPuzSetValidity(const bm128 *p, const int nPuz, char *res) const {
	//checks each of the puzzles for unique solution
	//stores valid puzzles to buf
	//returns the number of valid puzzles
	int n = 0; //number of valid (unique solution) puzzles
	int nUsets = (int)usetsBySize.size();
	bm128 *uu = bm128::allocate(nUsets);
	for(usetListBySize::const_iterator u = usetsBySize.begin(); u != usetsBySize.end(); u++) {
		uu[n++] = u->bitmap128;
	}
	n = 0;
	for(int i = 0; i < nPuz; i++) {
		for(int j = 0; j < nUsets; j++) {
			if(uu[j].isDisjoint(p[i])) {
				goto nextPuz;
			}
		}
		//all known UA are hit
		//check for uniqueness by solving
		char puz[81];
		for(int c = 0; c < 81; c++) {
			puz[c] = p[i].isBitSet(c) ? digits[c] : 0;
		}
		//if(1 == solve(gridBM, puz, 2)) {
		if(1 == solve(puz, 2)) {
			memcpy(res + 81 * n, puz, 81);
			n++;
		}
nextPuz:
		;
	}
	bm128::deallocate(uu);
	return n;
}
//__declspec(noinline) bool grid::isMinimalUA(const uset &s) const {
NOINLINE bool grid::isMinimalUA(const uset &s) const {
	//since there are minimal UA with > 2 solutions this is the (only?) way to check for minimality
	for(int i = 0; i < s.nbits; i++) {
		uset u = s;
		u.clearBit(s.positions[i]);
		u.positionsByBitmap();
		if(isUA(u)) {
			char buf[1000];
			s.toString(buf);
			printf("Non-minimal UA (pos %d) %s\n", i, buf);
			return false;
		}
	}
	return true;
}

//void grid::unused_checkUA() const {
//	printf("Checking UA sets");
//	//char buf[1000];
//	//for(usetListBySize::const_iterator s2 = usetsBySize.begin(); s2 != usetsBySize.end(); s2++) {
//	//	printf(".");
//	//	for(usetListBySize::iterator s = usetsBySize.lower_bound(*s2); s != usetsBySize.end(); s++) {
//	//		if(s2 != s && s2->isSubsetOf(*s)) {
//	//			//s is a superset of s2, remove it
//	//			s->toString(buf);
//	//			printf("\nRemoving %s as superset of ", buf);
//	//			s2->toString(buf);
//	//			printf("%s\n", buf);
//	//			usets.erase(*s);
//	//		}
//	//	}
//	//}
//	printf("\n");
//	for(usetListBySize::const_iterator s = usetsBySize.begin(); s != usetsBySize.end(); s++) {
//		if(!isMinimalUA(*s)) {
//			//printf("Non-minimal UA found!\n");
//		}
//	}
//}

void grid::generatePuzzle(char *p) const {
	int n = 0;
	for(int i = 0; i < 81; i++)
		p[i] = 0;
	//put 16 clues in random
	do {
		int rnd = rand() % 81;
		if(p[rnd] == 0) {
			p[rnd] = digits[rnd];
			n++;
		}
	} while(n < 16);
	//add clues until unique solution is found
	while(solve(gridBM, p, 2) != 1) {
		for(;;) {
			int rnd = rand() % 81;
			if(p[rnd] == 0) {
				p[rnd] = digits[rnd];
				n++;
				break;
			}
		}
	}
	//remove redundant clues if any
again:
	int redundants[80];
	int r = 0;
	int pos[80];
	int nPos = 0;
	for(int i = 0; i < 81; i++) {
		if(p[i]) {
			pos[nPos++] = i;
		}
	}
	for(int i = 0; i < nPos; i++) {
		char old = p[pos[i]];
		p[pos[i]] = 0;
		if(solve(gridBM, p, 2) == 1) {
			//pos[i] is redundant
			redundants[r++] = i;
		}
		p[pos[i]] = old;
	}
	if(r) {
		//remove a random clue from the redundant list
		int rnd = rand() % r; //redundant position to remove
		p[pos[redundants[rnd]]] = 0; //clear it from the puzzle
		goto again; //check for more redundants
	}
}
void grid::getBackBone() {
	char p[81];
	int occurences[81];
	for(int i = 0; i < 81; i++) {
		occurences[i] = 0;
	}
	for(int i = 0; i < 5000; i++) {
		generatePuzzle(p);
		for(int j = 0; j < 81; j++) {
			if(p[j]) {
				occurences[j]++;
			}
		}
	}
	for(int i = 0; i < 81; i++) {
		printf("%2d %d\n", i, occurences[i]);
	}
}

void grid::toString(const char *dig, char *buf) {
	for(int i = 0; i < 81; i++) {
		buf[i] = dig[i] ? dig[i] + '0' : '.';
	}
	buf[81] = 0;
}
void grid::toString(char *buf) const {
	toString(digits, buf);
}
void grid::setBM() {
	digit2bitmap(digits, gridBM);
}


extern void subcanon(const char* puz, char* can);
void singleClueSpecialUA(char *puz, const char *sol, const int nClues) {
	bool maxValency = opt.uaOpt->findvalency;
	grid g;
	char buf[100];
	ch81 u;
	memcpy(g.digits, sol, 81);
	digit2bitmap(g.digits, g.gridBM);
	int clNumber = 0;
	//unsigned long long nPerm = 0;
	int numSpecialUA = 0;
	int numUA = 0;
	int maxPerm = 2;
	int maxUaSize = 0;
	int sumSolved = 0;
	//int clueSolved[81];
	int clueMaxUa[81];
	int uaSizeTotal = 0;
	uset givens;
	givens.fromPuzzle(puz);
	for(int pos = 0; pos < 81; pos++) {
		int t = puz[pos];
		if(t) {
			puz[pos] = 0;
			g.usetsBySize.clear();
			g.findUAbyPuzzle(puz);
			int maxUaSizeForClue = g.usetsBySize.rbegin()->nbits;
			if(maxUaSize < maxUaSizeForClue) {
				maxUaSize = maxUaSizeForClue;
			}
			clueMaxUa[clNumber] = maxUaSizeForClue;
			if(opt.verbose) {
				fprintf(stdout, "\n\t%d\t%d\t%d\t%d\t#Clue,pos,numUA,maxUAsize\n", clNumber + 1, pos, (int)g.usetsBySize.size(), maxUaSizeForClue);
			}
			numUA += (int)g.usetsBySize.size();
			uset unsolved; //union of all UA not hit by this clue
			unsolved.clear();
			for(usetListBySize::const_iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end(); ua++) {
				unsolved |= *ua;
				uaSizeTotal += ua->nbits;
				if(maxValency) {
					if(ua->nbits > 9) {
						int nSol;
						nSol = (int)g.getSolutionCount(*ua);
						if(nSol > 2) {
							if(opt.verbose) {
								g.ua2puzzle(*ua, u.chars);
								u.toString(buf);
								fprintf(stdout, "\t\t%d\t%81.81s\t#nPerm,UA\n", nSol, buf);
							}
							numSpecialUA++;
							if(maxPerm < nSol)
								maxPerm = nSol;
						}
						//nPerm += nSol;
					}
					else {
						//nPerm += 2;
					}
				}
			}
			//if there is a clue not included in any of the unhit UA, then it is solvable w/o removed clue at pos
			unsolved |= givens;
			unsolved.positionsByBitmap();
			int nSolved = 81 - unsolved.nbits;
			sumSolved += nSolved;
			if(opt.verbose) {
				if(nSolved) {
					g.ua2puzzle(unsolved, u.chars);
					u.toString(buf);
					fprintf(stdout, "\t\t%d\t%81.81s\t#nSolved,solved\n", nSolved, buf);
				}
			}
			puz[pos] = t;
			clNumber++;
		} //if given
	} //pos loop
	if(opt.verbose) {
		if(maxValency) {
			fprintf(stdout, "\t\t\t%d\t%d\t%d\t%d\t%d\t#maxPerm,numSpecialUA,numUA,maxUAsize,nSolved\n", maxPerm, numSpecialUA, numUA, maxUaSize, sumSolved);
		}
		else {
			fprintf(stdout, "\t\t\t%d\t%d\t%d\t#numUA,maxUAsize,nSolved\n", numUA, maxUaSize, sumSolved);
		}
	}
	else {
		if(maxValency) {
			fprintf(stdout, "\t%d\t%d\t%d\t%d\t%d\t%d\n", maxPerm, numSpecialUA, numUA, maxUaSize, uaSizeTotal, sumSolved);
		}
		else {
			fprintf(stdout, "\t%d\t%d\t%d\t%d\n{", numUA, maxUaSize, uaSizeTotal, sumSolved);
			int sumMaxUa = 0;
			for(int i = 0; i < clNumber; i++) {
				fprintf(stdout, "%s%2d", i ? "," : "", clueMaxUa[i]);
				sumMaxUa += clueMaxUa[i];
			}
			fprintf(stdout, "}\t%d\n", sumMaxUa);
		}
	}
	fflush(NULL);
}
int processUA() {
	grid g;
	int ret;
	g.fname = opt.uaOpt->gridFileName;
	if(g.fname) { //process UA for a fixed grid
		ret = g.readFromFile();
		if(ret)
			return ret;
		if(opt.uaOpt->mspuzzles) { //use only given multiple solution puzzles in generation
			FILE *ms;
			ms = fopen(opt.uaOpt->mspuzzles, "rt");
			if(ms == NULL)
				return 1; //error
			char buf[3000];
			while(fgets(buf, sizeof(buf), ms)) {
				for(int i = 0; i < 81; i++) {
					buf[i] -= '0';
					if(buf[i] <= 0 || buf[i] > 9) {
						buf[i] = 0;
					}
				}
				g.findUAbyPuzzle(buf);
			}
		}
		else {
			g.readUAFromFileOnly(); //try to read existing file
			g.findInitialUA(); //do the job

			////char puz[88] = "00005000000670000300000104020060000000800000007000500000000001008000009000030000"; //mega
			//char puz[88] = "020000780000009000000030000004000000000000200007800005009060000000000040040020300"; //sf
			//for(int i = 0; i < 81; i++)
			//	puz[i] -= '0';
			//g.findUAbyPuzzle(puz);

		}
		g.saveUA(); //create/update the ua file
		if(opt.uaOpt->subcanon) { //dump UA list in canonical form (w/ duplicates)
			char puz[88], puz1[88];
			for(usetListBySize::const_iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end(); ua++) {
				g.ua2InvariantPuzzle(*ua, puz);
				g.toString(puz, puz1);
				printf("%s\t", puz1);
				subcanon(puz, puz1);
				g.toString(puz1, puz);
				printf("%s\n", puz);
			}
		}
	}
	else if(opt.uaOpt->minus1) { //list UA with >2 solutions applying {-1} to a valid puzzle --unav --minus1 < puzzlelist.txt > specialua.txt
		char buf[3000];
		ch81 puz, sol;
		while(fgets(buf, sizeof(buf), stdin)) {
			int nClues = puz.fromString(buf);
			printf("%81.81s\t%d", buf, nClues);
			unsigned long long nSol;
			nSol = solve(puz.chars, 1, sol.chars);
			if(nSol) {
				singleClueSpecialUA(puz.chars, sol.chars, nClues);
			}
			else { //invalid puzzle
				printf("\tInvalid puzzle\n");
			}
		}
	}
	else { //find UA from multi-solution puzzle --unav < puzzlelist.txt > ua.txt
		//const int maxSol = 13100000;
		int maxSol = 13100000;
		if(opt.uaOpt->maxsolutioncount) {
			maxSol = (int)opt.uaOpt->maxsolutioncount;
		}
		//char sol[maxSol][81];
		//char (*sol)[maxSol][81] = (char(*)[maxSol][81])malloc(maxSol * 81);
		char *sol = (char*)malloc(maxSol * 81);
		char buf[3000];
		ch81 buf1;
		char puz[81];
		puzzleSet cache;
		while(fgets(buf, sizeof(buf), stdin)) {
			int nClues = 81;
			if(opt.verbose)
				printf("\n\n%81.81s\t", buf);
			for(int i = 0; i < 81; i++) {
				buf[i] -= '0';
				if(buf[i] <= 0 || buf[i] > 9) {
					buf[i] = 0;
					nClues--;
				}
			}
			memcpy(puz, buf, 81);
			unsigned long long nSol;
			//nSol = solve(puz, maxSol, sol); //this is when we are searching for multi-valency UA
			nSol = solve(puz, 1, sol); //this is when we are searching for UA strictly within the erased cells of the given puzzle
			if(opt.uaOpt->minuasize) {
				if(nSol < maxSol) {
					findUaBySolutions(sol, (int)nSol);
				}
			}
			else {
				if(opt.verbose)
					printf("%d\t%llu\t#pattern,nClues,nSolutions\n", nClues, nSol);
				if(nSol == maxSol) {
					printf("Maximum solutions reached!\n");
				}
				else {
					for(int s = 0; s < nSol; s++) {
						memcpy(g.digits, &sol[s * 81], 81);
						digit2bitmap(g.digits, g.gridBM);
						g.usetsBySize.clear();
						if(g.findUAbyPuzzle(puz)) { //no error (known error is nSol >= INT_MAX which couldn't happen)
							if(opt.verbose) {
								g.toString(g.digits, buf);
								printf("\n%81.81s\t%d\t%d\t#solution,sNumber,nUnavoidables\n", buf, s + 1, (int)g.usetsBySize.size());
							}
							for(usetListBySize::const_iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end(); ua++) {
								//ua->toString(buf);
								g.ua2InvariantPuzzle(*ua, buf);
								if(opt.verbose) {
									unsigned long long n = solve(buf, 2);
									g.toString(buf, buf + 88);
									printf("%81.81s\t%d\t%c\n", buf + 88, ua->nbits, n == 1 ? 'y' : 'n'); //natural
								}
								else if(opt.uaOpt->minvalency) {
									unsigned long long n = g.getSolutionCount(*ua);
									if(n >= opt.uaOpt->minvalency) {
										g.ua2puzzle(*ua, buf);
										subcanon(buf, buf1.chars);
										if(cache.find(buf1) == cache.end()) {
											buf1.toString(buf);
											printf("%81.81s\t%d\t%llu\n", buf, ua->nbits, n); //canonical only
											fflush(NULL);
											cache.insert(buf1);
										}
									}
								}
								else if(ua->nbits >= opt.uaOpt->minuasize) { //default minuasize = 0
									subcanon(buf, buf1.chars);
									buf1.toString(buf);
									printf("%81.81s\t%d\n", buf, ua->nbits); //canonical only
								}
							}
						}
					} //for s
				}
			}
		}
		free(sol);
	}
	return 0;
}
