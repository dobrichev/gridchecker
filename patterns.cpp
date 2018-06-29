#include "t_128.h"
#include "ch81.h"
#include "solver.h"
#include "options.h"
#include "rate.h"
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include "patterns.h"
#include "mm_allocator.h"
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
#include <thread>
#include <inttypes.h>
#include <chrono>

typedef uint32_t rating_t;

void morph::getMorphs(void *context, const char* p, const morph &start, const morph &end, morphCallBack *cb) {
	morph current;
	//int n = 0;
	char tp[9][9]; //transposed
	char (*p99[2])[9][9]; //original & transposed
	//lightweightUsetList ss;
	p99[0] = (char (*)[9][9])p;
	p99[1] = &tp;
	for(int r = 0; r < 9; r++) { //transpose
		for(int c = 0; c < 9; c++) {
			tp[c][r] = (*p99[0])[r][c];
		}
	}
	static const int spi[6][9] = { //perform on all rows on original to permute stacks
		{0,1,2, 3,4,5, 6,7,8}, //123
		{0,1,2, 6,7,8, 3,4,5}, //132
		{3,4,5, 0,1,2, 6,7,8}, //213
		{3,4,5, 6,7,8, 0,1,2}, //231
		{6,7,8, 0,1,2, 3,4,5}, //312
		{6,7,8, 3,4,5, 0,1,2}, //321
	};
	static const int cpi[6][3] = {
		{0,1,2}, //123
		{0,2,1}, //132
		{1,0,2}, //213
		{1,2,0}, //231
		{2,0,1}, //312
		{2,1,0}, //321
	};
	for(current.transpose = start.transpose; current.transpose <= end.transpose; current.transpose++) {
		for(current.stackPerm = start.stackPerm; current.stackPerm <= end.stackPerm; current.stackPerm++) { //permute stacks
			char spp[9][9]; //puzzle with permuted stacks
			for(int r = 0; r < 9; r++) {
				for(int c = 0; c < 9; c++) {
					spp[r][c] = (*p99[current.transpose])[r][spi[current.stackPerm][c]];
				}
			}
			for(current.stack1ColPerm = start.stack1ColPerm; current.stack1ColPerm <= end.stack1ColPerm; current.stack1ColPerm++) { //stack 1 column permutation
				char s1cpp[9][9]; //puzzle with permuted columns in stack 1
				memcpy(s1cpp, spp, 81);
				for(int r = 0; r < 9; r++) {
					for(int c = 0; c < 3; c++) {
						s1cpp[r][c] = spp[r][cpi[current.stack1ColPerm][c]];
					}
				}
				for(current.stack2ColPerm = start.stack2ColPerm; current.stack2ColPerm <= end.stack2ColPerm; current.stack2ColPerm++) { //stack 2 column permutation
					char s2cpp[9][9]; //puzzle with permuted columns in stack 2
					memcpy(s2cpp, s1cpp, 81);
					for(int r = 0; r < 9; r++) {
						for(int c = 0; c < 3; c++) {
							s2cpp[r][c + 3] = s1cpp[r][cpi[current.stack2ColPerm][c] + 3];
						}
					}
					for(current.stack3ColPerm = start.stack3ColPerm; current.stack3ColPerm <= end.stack3ColPerm; current.stack3ColPerm++) { //stack 3 column permutation
						char s3cpp[9][9]; //puzzle with permuted columns in stack 3
						memcpy(s3cpp, s2cpp, 81);
						for(int r = 0; r < 9; r++) {
							for(int c = 0; c < 3; c++) {
								s3cpp[r][c + 6] = s2cpp[r][cpi[current.stack3ColPerm][c] + 6];
							}
						}
						for(current.bandPerm = start.bandPerm; current.bandPerm <= end.bandPerm; current.bandPerm++) { //band permutation
							char bpp[9][9]; //puzzle with permuted bands
							for(int b = 0; b < 3; b++) {
								memcpy(bpp[3 * b], s3cpp[3 * cpi[current.bandPerm][b]], 27);
							}
							char b1rpp[9][9]; //puzzle with permuted rows in band 1
							memcpy(b1rpp[3], bpp[3], 54); //rows 4..9
							for(current.band1RowPerm = start.band1RowPerm; current.band1RowPerm <= end.band1RowPerm; current.band1RowPerm++) { //band 1 row permutation
								for(int r = 0; r < 3; r++) {
									memcpy(b1rpp[r], bpp[cpi[current.band1RowPerm][r]], 9);
									//here the row r is fixed
								}
								char b2rpp[9][9]; //puzzle with permuted rows in band 2
								memcpy(b2rpp, b1rpp, 81);
								for(current.band2RowPerm = start.band2RowPerm; current.band2RowPerm <= end.band2RowPerm; current.band2RowPerm++) { //band 2 row permutation
									for(int r = 0; r < 3; r++) {
										memcpy(b2rpp[r + 3], b1rpp[cpi[current.band2RowPerm][r] + 3], 9);
										//here the row 3 + r is fixed
									}
									char b3rpp[9][9]; //puzzle with permuted rows in band 3
									memcpy(b3rpp, b2rpp, 54); //rows 1..6
									for(current.band3RowPerm = start.band3RowPerm; current.band3RowPerm <= end.band3RowPerm; current.band3RowPerm++) { //band 3 row permutation
										for(int r = 0; r < 3; r++) {
											memcpy(b3rpp[r + 6], b2rpp[cpi[current.band3RowPerm][r] + 6], 9);
											//here row 6 + r is fixed
										}
										int ret = (*cb)(context, p, (char*)b3rpp, current);
										if(ret) { //call back
											return; // ret; //immediately exit when callback returns nonzero
										}
										//n++;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return;
}
void morph::processAllMorphs(void *context, const char* p, morphCallBack *cb) {
	static const morph morph_first = {0,0,0,0,0,0,0,0,0};
	static const morph morph_last = {1,5,5,5,5,5,5,5,5};
	morph::getMorphs(context, p, morph_first, morph_last, cb);
}
void morph::processSingleMorph(void *context, const char* p, const morph &morph, morphCallBack *cb) {
	morph::getMorphs(context, p, morph, morph, cb);
}
void morph::processSingleMorph(void *context, const char* p, const int morphIndex, morphCallBack *cb) {
	morph m;
	m.byIndex(morphIndex);
	morph::getMorphs(context, p, m, m, cb);
}

class bm128IntMap : public std::map<bm128, int, less<bm128>, mm_allocator<pair<bm128, int> > > {};
class bm128IntsMap : public std::map<bm128, std::vector<int>, less<bm128>, mm_allocator<pair<bm128, std::vector<int>> > > {};
const ch81 probe = {
	{
		 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80
	}
};

////class allTranformations implementation
//const int allTranformations::count = 2*6*6*6*6*6*6*6*6;
//int allTranformations::allCallback(void *context, const char *puz, char *m, const morph &theMorph) {
//	(void)puz; //suppress compiler warnings
//	memcpy(&((ch81*)context)[theMorph.index()], m, 81);
//	return 0;
//}
//allTranformations::allTranformations() {
//	indices = (ch81*) malloc(81 * count);
//	getMorphs(indices, probe.chars, morph_first, morph_last, &allTranformations::allCallback);
//}
//allTranformations::~allTranformations() {
//	free(indices);
//}
//void allTranformations::transform(const ch81 &src, ch81 &dest, int transformationIndex) const {
//	ch81 &ind = indices[transformationIndex];
//	for(int i = 0; i < 81; i++) {
//		dest.chars[(int)(ind.chars[i])] = src.chars[i];
//
//		//reverse transfor would look like
//		//dest.chars[i] = src.chars[ind.chars[i]];
//	}
//}

class pat {
private:
	bm128IntMap ss; //pairs of (givens' positions, morph index)
	vector<morph> self_transformations;
public:
	vector<bm128, mm_allocator<bm128> > morphs;
	vector<int> morphIndex;
	vector<ch81> maps;
	int numAutomorphisms;
private:
	static int init_callback (void *context, const char *puz, char *m, const morph &theMorph) {
		pat *pp = static_cast<pat*>(context);
		if(0 == memcmp(puz, m, 81)) {
			pp->numAutomorphisms++;
			pp->self_transformations.push_back(theMorph);
		}
		bm128 rr;
		rr.clear();
		for(int i = 0; i < 81; i++) {
			if(m[i]) {
				rr.setBit(i);
			}
		}
		pp->ss[rr] = theMorph.index();
		return 0;
	}
	static int map_callback (void *context, const char *puz, char *m, const morph &theMorph) {
		(void)puz; (void)theMorph; //suppress compiler warnings
		pat *pp = static_cast<pat*>(context);
		pp->maps.push_back(*(ch81*)m);
		return 1; //exit on first call
	}
	void minRelabel(const char *src, char *res) const {
		int map[10] = {0,-1,-1,-1,-1,-1,-1,-1,-1,-1};
		int n = 1;
		for(int pos = 0; pos < 81; pos++) {
			if(src[pos] == 0) {
				res[pos] = 0;
				continue;
			}
			if(map[(int)(src[pos])] == -1) { //first occurrence of this label
				map[(int)(src[pos])] = n++;
			}
			res[pos] = map[(int)(src[pos])];
		}
	}
	void minlex(const char *src, char *res) const {
		if(numAutomorphisms == 1) {
			minRelabel(src, res);
			return;
		}
		char test[81];
		char m[81];
		minRelabel(src, res); //store the best at res
		for(int i = 1; i < numAutomorphisms; i++) { //skip the non-morphed at 0
			//morph the pussle
			for(int j = 0; j < 81; j++) {
				m[(int)maps[i].chars[j]] = src[j];
			}
			minRelabel(m, test);
			if(memcmp(test, res, 81) < 0) { //store the best at res
				memcpy(res, test, 81);
			}
		}
	}
public:
	void init(const char *p) {
		//copy the string
		ch81 original;
		original.fromString(p);
		//convert to 0/1
		for(int i = 0; i < 81; i++) {
			if(original.chars[i])
				original.chars[i] = 1;
		}
		numAutomorphisms = 0;
		//populate ss, self_transformations, numAutomorphisms
		morph::processAllMorphs(this, original.chars, &pat::init_callback);
		//init morphs using ss
		for(bm128IntMap::const_iterator i = ss.begin(); i != ss.end(); i++) {
			morphs.push_back(i->first);
			morphIndex.push_back(i->second);
		}
		//free some memory
		ss.clear();
		//compose transformation maps
		for(int i = 0; i < (int)self_transformations.size(); i++) {
			morph::processSingleMorph(this, probe.chars, self_transformations[i], &pat::map_callback);
		}
		self_transformations.clear();
	}
	void getPosMorphs(const char *p, char *res) const {
		memset(res, 0, 81 * numAutomorphisms);
		for(int i = 0; i < numAutomorphisms; i++) {
			char *m = res + 81 * i;
			//morph the puzzle
			for(int j = 0; j < 81; j++) {
				m[(int)maps[i].chars[j]] = p[j];
			}
		}
	}
	void minlex(ch81 &p) { //overwrite given puzzle with its minlex representation
		ch81 res;
		minlex(p.chars, res.chars);
		p = res; //structure copy
	}
};

//int mapPuz_callback (void *context, const char *puz, char *m, const morph &theMorph) {
//	memcpy(context, m, 81);
//	return 1; //exit on first call
//}


int statistics() {
	char buf[1000];
	map<int,int> digitDistribution; //givens per digit
	map<int,int> boxDistribution; //givens per box
	//int templ2Distribution[181]; //2-templates count
	//int templ2DigitCount[181]; //digits per 2-template
	//for(int i = 0; i < 181; i++) {
	//	templ2Distribution[i] = templ2DigitCount[i] = 0;
	//}
	////ch81 *perm = (ch81*)malloc(3359232 * sizeof(ch81));
	//puzzleSetTagInt t3Count;
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		int nClues = puz.fromString(buf);

		//getMorphs(puz.chars, perm);
		//continue;

		//int t2index[36]; //index of each of the 36 2-templates
		//ch81 sol;
		//solve(puz.chars, 1, sol.chars);

		//for(int i = 0; i < 181; i++) {
		//	templ2Distribution[i] = templ2DigitCount[i] = 0;
		//}

		//int sumGivens = 0;
		//for(int i = 0; i < 36; i++) {
		//	ch81 t;
		//	int nGivens = 0;
		//	for(int c = 0; c < 81; c++) {
		//		if((sol.chars[c] == (1 + choice2of9[i][0])) || (sol.chars[c] == (1 + choice2of9[i][1]))) {
		//			t.chars[c] = sol.chars[c];
		//			if(puz.chars[c]) nGivens++;
		//		}
		//		else {
		//			t.chars[c] = 0;
		//		}
		//	}
		//	ch81 tCan;
		//	subcanon(t.chars, tCan.chars);
		//	for(int ti = 0; ti < 181; ti++) {
		//		if(0 == memcmp(tCan.chars, templ2[ti], 81)) {
		//			t2index[i] = ti;
		//			templ2Distribution[ti]++;
		//			templ2DigitCount[ti] += nGivens;
		//			break;
		//		}
		//	}
		//	sumGivens += nGivens;
		//}

		//for(int i = 0; i < 84; i++) {
		//	ch81 t;
		//	int nGivens = 0;
		//	for(int c = 0; c < 81; c++) {
		//		if((sol.chars[c] == (1 + choice3of9[i][0])) || (sol.chars[c] == (1 + choice3of9[i][1])) || (sol.chars[c] == (1 + choice3of9[i][2]))) {
		//			t.chars[c] = sol.chars[c];
		//			if(puz.chars[c]) nGivens++;
		//		}
		//		else {
		//			t.chars[c] = 0;
		//		}
		//	}
		//	ch81 tCan;
		//	subcanon(t.chars, tCan.chars);
		//	t3Count[tCan]++;
		//	sumGivens += nGivens;
		//}

		//for(int i = 0; i < 126; i++) {
		//	ch81 t;
		//	int nGivens = 0;
		//	for(int c = 0; c < 81; c++) {
		//		if(!((sol.chars[c] == (1 + choice4of9[i][0])) || (sol.chars[c] == (1 + choice4of9[i][1])) || (sol.chars[c] == (1 + choice4of9[i][2])) || (sol.chars[c] == (1 + choice4of9[i][3])))) {
		//			t.chars[c] = sol.chars[c];
		//			if(puz.chars[c]) nGivens++;
		//		}
		//		else {
		//			t.chars[c] = 0;
		//		}
		//	}
		//	ch81 tCan;
		//	subcanon(t.chars, tCan.chars);
		//	t3Count[tCan]++;
		//	sumGivens += nGivens;
		//}

		//for(int i = 0; i < 181; i++) {
		//	printf("%3d", templ2Distribution[i]);
		//}
		//printf("\t%3d\n", sumGivens);
		//for(int i = 0; i < 181; i++) {
		//	printf("%3d", templ2DigitCount[i]);
		//}
		//printf("\n");

		vector<int> digitsCount(9, 0);
		int digitsCountMask = 0;
		for(int i = 0; i < 81; i++) {
			if(puz.chars[i]) {
				digitsCount[puz.chars[i] - 1]++;
			}
		}
		sort(digitsCount.begin(), digitsCount.end(), greater<int>());
		for(int i = 0; i < 9; i++) {
			digitsCountMask = digitsCountMask * 10 + digitsCount[i];
		}
		digitDistribution[digitsCountMask]++;
		if(opt.verbose) {
			puz.toString(buf);
			fprintf(stdout, "%81.81s\t%d\t", buf, nClues);
			fprintf(stdout, "%d\t", digitsCountMask); //number of givens per digit
		}

		vector<int> boxCount(9, 0);
		int boxCountMask = 0;
		for(int i = 0; i < 81; i++) {
			if(puz.chars[i]) {
				boxCount[boxByCellIndex[i]]++;
			}
		}
		sort(boxCount.begin(), boxCount.end(), greater<int>());
		for(int i = 0; i < 9; i++) {
			boxCountMask = boxCountMask * 10 + boxCount[i];
		}
		boxDistribution[boxCountMask]++;
		if(opt.verbose) {
			fprintf(stdout, "%d\n", boxCountMask); //number of givens per box
		}
	}

	//for(int i = 0; i < 181; i++) {
	//	printf("%d\t%d\t%d\n", i + 1, templ2Distribution[i], templ2DigitCount[i]);
	//}

	fprintf(stdout, "givens per digit\n");
	for(map<int,int>::const_iterator dd = digitDistribution.begin(); dd != digitDistribution.end(); dd++) {
		fprintf(stdout, "%d\t%d\n", dd->first, dd->second);
	}
	fprintf(stdout, "givens per box\n");
	for(map<int,int>::const_iterator dd = boxDistribution.begin(); dd != boxDistribution.end(); dd++) {
		fprintf(stdout, "%d\t%d\n", dd->first, dd->second);
	}

	//t3Count.saveToFile(stdout);

	return 0;
}

int redundancy() {
	char buf[2000];
	bool summary = opt.verbose;
	int min[81], max[81], sum[81], count[81];
	if(summary) {
		for(int i = 0; i < 81; i++) {
			min[i] = INT_MAX;
			max[i] = 0;
			sum[i] = 0;
			count[i] = 0;
		}
	}

	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		int nClues = puz.fromString(buf);
		int redundancyLevel = 0; //sum(visible givens)
		for(int i = 0; i < 81; i++) {
			if(puz.chars[i]) {
				for(int ac = 0; ac < 20; ac++) {
					if(puz.chars[affectedCells[i][ac]]) {
						redundancyLevel++;
					}
				}
			}
		}
		redundancyLevel /= 2;
		if(summary) {
			count[nClues]++;
			if(min[nClues] > redundancyLevel) {
				min[nClues] = redundancyLevel;
			}
			if(max[nClues] < redundancyLevel) {
				max[nClues] = redundancyLevel;
			}
			sum[nClues] += redundancyLevel;
		}
		//puz.toString(buf);
		fprintf(stdout, "%81.81s\t%d\t%d\n", buf, nClues, redundancyLevel);
	}
	if(summary) {
		fprintf(stderr, "nClues\tmin\tmax\tavg\tcount\n");
		for(int i = 0; i < 81; i++) {
			if(count[i] == 0)
				continue;
			fprintf(stderr, "%d\t%d\t%d\t%6.2f\t%d\n", i, min[i], max[i], ((double)sum[i]) / count[i], count[i]);
		}
	}
	return 0;
}

int puzFound(void *context, const char* puz) {
	(void)context; //suppress compiler warnings
	char buf[1000];
	ch81 p;
	for(int i = 0; i < 81; i++)
		p.chars[i] = puz[i];
	p.toString(buf);
	fprintf(stdout, "%81.81s\n", buf);
	fflush(NULL);
	return 0;
}

int enumerate(const char* pattern, const char* fixclues) {
//One pattern with Redundancy of 36
//..............1..2..3....4.............34..5..1..6...7...........4..5.3..7.8.2..1
//
//... ... ...
//... ..1 ..2
//..3 ... .4.
//
//... ... ...
//... 34. .5.
//.1. .6. ..7
//
//... ... ...
//..4 ..5 .3.
//.7. 8.2 ..1
//
//
//... ... ...
//... ..8 ..9
//..h ... .c.
//
//... ... ...
//... dg. .b.
//.f. .e. ..a
//
//... ... ...
//..7 ..5 .6.
//.1. 2.3 ..4
//
//max givens per group (r8), then max visible outside the group, then ? (1,2,3,4)
//max visible fixed(r8c6=2), then max visible non-fixed, then ? (5=5..9)
//max visible fixed(r8c3,r8c8,r2c6=2), then max visible non-fixed(2,3,1), then ? (6)
//max visible fixed(r8c3=3), then max visible non-fixed, then ? (7)
//max visible fixed(r2c6=2), then max visible non-fixed, then ? (8)
//max visible fixed(r2c9=2), then max visible non-fixed, then ? (9)
//max visible fixed(r3c8,r6c9=2), then max visible non-fixed(2,3), then ? (a)
//max visible fixed(r3c8,r5c8,r6c2=2), then max visible non-fixed(2,3,1), then ? (b)
//max visible fixed(r3c8=3), then max visible non-fixed, then ? (c)
//max visible fixed(r3c3,r5c4,r6c2=2), then max visible non-fixed(0,2,1), then ? (d)
//max visible fixed(r3c3,r5c5,r6c2,r6c5=2), then max visible non-fixed(0,1,1,2), then ? (e)
//max visible fixed(r5c5,r6c2=3), then max visible non-fixed(0,0), then max non-fixed in chutes (1,2) (f)
//max visible fixed(r5c5=3), then max visible non-fixed, then max non-fixed in chutes (g)
//last one (h)
	//char buf[1000] = "..............1..2..3....4.............34..5..1..6...7...........4..5.3..7.8.2..1";
	ch81 puz;
	int nClues;
	nClues = puz.fromString(pattern);
	//rate the groups by
	//1) num givens, then
	//2) prefer box, then
	//3) max givens per minirow/minicol, then
	//max visible outside the group, then
	//?
	bool isOrdered[81];
	int nFixedClues = 0;
	int cluePositions[81];
	ch81 p;
	p.clear();
	if(fixclues) {
		//fix the clues from the given parameter
		ch81 fc;
		fc.fromString(fixclues);
		for(int i = 0; i < 81; i++) {
			isOrdered[i] = false;
			if(puz.chars[i] && fc.chars[i]) { //ignore if not given in the pattern too
				isOrdered[i] = true;
				cluePositions[nFixedClues] = i;
				p.chars[i] = fc.chars[i];
				nFixedClues++;
			}
		}
	}
	else {
		int givensPerGroup[27];
		int maxGivensPerGroup = 0; 
		int groupWithMaxGivens = 0; 
		for(int i = 0; i < 27; i++) {
			givensPerGroup[i] = 0;
		}
		for(int i = 0; i < 81; i++) {
			isOrdered[i] = false;
			if(puz.chars[i]) {
				for(int gr = 0; gr < 3; gr++) {
					givensPerGroup[affectedGroups[i][gr]]++;
				}
			}
		}
		for(int i = 0; i < 27; i++) {
			if(maxGivensPerGroup <= givensPerGroup[i]) { //<= to find last maximal group, possible box
				maxGivensPerGroup = givensPerGroup[groupWithMaxGivens = i];
			}
		}
		//fix the clues from the chosen group
		for(int i = 0; i < 9; i++) {
			int ci = cellsInGroup[groupWithMaxGivens][i];
			if(puz.chars[ci]) {
				isOrdered[ci] = true;
				cluePositions[nFixedClues] = ci;
				p.chars[ci] = ++nFixedClues;
			}
		}
	}
	//order the rest clues
	for(int nOrderedClues = nFixedClues; nOrderedClues < nClues; nOrderedClues++) {
		int maxRating = 0;
		int best = -1;
		for(int i = 0; i < 81; i++) {
			if(puz.chars[i] && !isOrdered[i]) {
				int rating = 0;
				for(int j = 0; j < 81; j++) {
					if(puz.chars[j] && j != i) {
						if(rowByCellIndex[i] == rowByCellIndex[j] || colByCellIndex[i] == colByCellIndex[j] || boxByCellIndex[i] == boxByCellIndex[j]) { 
							if(isOrdered[j]) {
								rating += 100*100; //fixed visible
							}
							else {
								rating += 100; //non-fixed visible
							}
						}
						if((!isOrdered[j]) && (bandByCellIndex[i] == bandByCellIndex[j] || stackByCellIndex[i] == stackByCellIndex[j]) && (boxByCellIndex[i] != boxByCellIndex[j])) { 
							rating += 1; //non-fixed in chutes
						}
					}
				}
				if(maxRating < rating) {
					maxRating = rating;
					best = i;
				}
			}
		}
		isOrdered[best] = true;
		cluePositions[nOrderedClues] = best;
	}
	//at this point we have:
	//p - a puzzle populated with fixed clues
	//cluePositions - an array of nClues with ordered clue positions, including fixed
	//nFixedClues - the number of clues in cluePositions not to touch
	//nClues - the size of cluePositions array
	int err = solverPattern(p.chars, nClues - nFixedClues, cluePositions + nFixedClues, puzFound);

	return err;
}
struct puzzleRecord {
	uint8_t key[16]; //32 half-bytes, first given is always "1", these are the values for givens at positions 2..33
	rating_t rateFast; // bits 0..3=depth; bits 4..5=minimal; 6..7=reserved; 8..15=ER; 16..23=EP; 24..31=ED //also updated directly by fskfr::skfrCommit()
	rating_t rateFinal; // bits 0..3=nosingles depth; bit 4..7=reserved; 8..15=ER; 16..23=EP; 24..31=ED
	//minimal bit flags: 00=unknown; 01=non-minimal; 10=minimal
	static const rating_t rateMask = 0xFFFFFF00;
	static const rating_t depthMask = 0x07;
	static const rating_t minimalityMask = 0x18;
	static const rating_t ERmask = 0xFF00;
	static const rating_t EPmask = 0xFF0000;
	static const rating_t EDmask = 0xFF000000;
	bool operator< (const puzzleRecord & other) const {
		return(memcmp(this, &other, 16) < 0);
	}
	bool operator== (const puzzleRecord & other) const {
		return(memcmp(this, &other, 16) == 0);
	}
	bool isMinimal () const {
		return((rateFast & minimalityMask) == (2 << 3));
	}
	puzzleRecord& merge(const puzzleRecord& other) { //combine fields from 2 input puzzles
		if((other.rateFast & rateMask) > (this->rateFast & rateMask)) this->rateFast = ((this->rateFast & (~rateMask)) | (other.rateFast & rateMask)); //the greater rating
		if((other.rateFast & depthMask) > (this->rateFast & depthMask)) this->rateFast = ((this->rateFast & (~depthMask)) | (other.rateFast & depthMask)); //the greater depth
		if((other.rateFast & minimalityMask) > (this->rateFast & minimalityMask)) this->rateFast = ((this->rateFast & (~minimalityMask)) | (other.rateFast & minimalityMask)); //the greater minimality (just following convention)
		if((other.rateFinal & rateMask) > (this->rateFinal & rateMask)) this->rateFinal = ((this->rateFinal & (~rateMask)) | (other.rateFinal & rateMask)); //the greater rating
		if((other.rateFinal & depthMask) > (this->rateFinal & depthMask)) this->rateFinal = ((this->rateFinal & (~depthMask)) | (other.rateFinal & depthMask)); //the greater nosingles depth
		return *this;
	}
	rating_t getRateFinalER() const {
		return (this->rateFinal & ERmask) >> 8;
	}
	rating_t getRateFinalEP() const {
		return (this->rateFinal & EPmask) >> 16;
	}
	rating_t getRateFinalED() const {
		return (this->rateFinal & EDmask) >> 24;
	}
};
typedef std::set<puzzleRecord*> puzzleRecordset;

struct inputFilter {
	rating_t minus;
	rating_t minED;
	rating_t maxED;
	rating_t minEP;
	rating_t maxEP;
	rating_t minER;
	rating_t maxER;
	bool noSingles;
	rating_t minMinimality;
	rating_t maxMinimality;
	inputFilter(rating_t n, rating_t minED_, rating_t maxED_, rating_t minEP_, rating_t maxEP_, rating_t minER_, rating_t maxER_,
			rating_t noSingles_, rating_t minMinimality_ = 0, rating_t maxMinimality_ = 3) :
		minus(n & puzzleRecord::depthMask),
		minED((minED_ << 24) & puzzleRecord::EDmask), maxED((maxED_ << 24) & puzzleRecord::EDmask),
		minEP((minEP_ << 16) & puzzleRecord::EPmask), maxEP((maxEP_ << 16) & puzzleRecord::EPmask),
		minER((minER_ << 8) & puzzleRecord::ERmask), maxER((maxER_ << 8) & puzzleRecord::ERmask),
		noSingles(noSingles_), minMinimality((minMinimality_ << 3) & puzzleRecord::minimalityMask), maxMinimality((maxMinimality_ << 3) & puzzleRecord::minimalityMask) {}
	bool matches(const puzzleRecord& rec) const {
		if(noSingles) {
			if(((rec.rateFast & puzzleRecord::depthMask) < minus) //not relabeled to this depth unconditionally
				&& ((rec.rateFinal & puzzleRecord::depthMask) < minus) //not relabeled to this depth ignoring puzzles solved by singles
				&& ((rec.rateFast & puzzleRecord::EDmask) >= minED)
				&& ((rec.rateFast & puzzleRecord::EDmask) <= maxED)
				&& ((rec.rateFast & puzzleRecord::EPmask) >= minEP)
				&& ((rec.rateFast & puzzleRecord::EPmask) <= maxEP)
				&& ((rec.rateFast & puzzleRecord::ERmask) >= minER)
				&& ((rec.rateFast & puzzleRecord::ERmask) <= maxER)
				&& ((rec.rateFast & puzzleRecord::minimalityMask) >= minMinimality)
				&& ((rec.rateFast & puzzleRecord::minimalityMask) <= maxMinimality))
			{
				return true;
			}
		}
		else {
			if(((rec.rateFast & puzzleRecord::depthMask) < minus) //not relabeled to this depth
				&& ((rec.rateFast & puzzleRecord::EDmask) >= minED)
				&& ((rec.rateFast & puzzleRecord::EDmask) <= maxED)
				&& ((rec.rateFast & puzzleRecord::EPmask) >= minEP)
				&& ((rec.rateFast & puzzleRecord::EPmask) <= maxEP)
				&& ((rec.rateFast & puzzleRecord::ERmask) >= minER)
				&& ((rec.rateFast & puzzleRecord::ERmask) <= maxER)
				&& ((rec.rateFast & puzzleRecord::minimalityMask) >= minMinimality)
				&& ((rec.rateFast & puzzleRecord::minimalityMask) <= maxMinimality))
			{
				return true;
			}
		}
		return false;
	}
	bool matchesRateFinal(const puzzleRecord& rec) const {
		//here we don't care about depth but care about minimality
		if(
				   ((rec.rateFinal & puzzleRecord::EDmask) >= minED)
				&& ((rec.rateFinal & puzzleRecord::EDmask) <= maxED)
				&& ((rec.rateFinal & puzzleRecord::EPmask) >= minEP)
				&& ((rec.rateFinal & puzzleRecord::EPmask) <= maxEP)
				&& ((rec.rateFinal & puzzleRecord::ERmask) >= minER)
				&& ((rec.rateFinal & puzzleRecord::ERmask) <= maxER)
				&& ((rec.rateFast & puzzleRecord::minimalityMask) >= minMinimality)
				&& ((rec.rateFast & puzzleRecord::minimalityMask) <= maxMinimality)
			)
		{
			return true;
		}
		return false;
	}
};
class pgGotchi {
	//for 16+4+4 there is 3550103402/83=42772330 puzzles capacity
public:
	struct uncomprPuz {
		ch81 p;
		uint32_t rateFastED;
		uint32_t rateFastEP;
		uint32_t rateFastER;
		uint32_t rateFinalED;
		uint32_t rateFinalEP;
		uint32_t rateFinalER;
		uint32_t specTransformedUpTo;
		uint32_t transformedUpTo;
		uint32_t minimality;
		void toString(char *s) const {
			p.toString(s);
			sprintf(s + 81, " ED=%u.%1.1u/%u.%1.1u/%u.%1.1u %u %u %u ED=%u.%1.1d/%u.%1.1u/%u.%1.1u\n",
				rateFastER / 10, rateFastER % 10, rateFastEP / 10, rateFastEP % 10, rateFastED / 10, rateFastED % 10,
				specTransformedUpTo, transformedUpTo, minimality,
				rateFinalER / 10, rateFinalER % 10, rateFinalEP / 10, rateFinalEP % 10, rateFinalED / 10, rateFinalED % 10);
		}
		int fromString(const char *s) {
			unsigned int rateFastERh = 0, rateFastERl = 0, rateFastEPh = 0, rateFastEPl = 0, rateFastEDh = 0, rateFastEDl = 0,
				rateFinalERh = 0, rateFinalERl = 0, rateFinalEPh = 0, rateFinalEPl = 0, rateFinalEDh = 0, rateFinalEDl = 0;

			int ret = p.fromString(s);
			specTransformedUpTo = 0;
			transformedUpTo = 0;
			minimality = 0;
			sscanf(s + 81, " ED=%u.%u/%u.%u/%u.%u %u %u %u ED=%u.%u/%u.%u/%u.%u",
				&rateFastERh, &rateFastERl, &rateFastEPh, &rateFastEPl, &rateFastEDh, &rateFastEDl,
				&specTransformedUpTo, &transformedUpTo, &minimality,
				&rateFinalERh, &rateFinalERl, &rateFinalEPh, &rateFinalEPl, &rateFinalEDh, &rateFinalEDl);
			rateFastER = 10 * rateFastERh + rateFastERl;
			rateFastEP = 10 * rateFastEPh + rateFastEPl;
			rateFastED = 10 * rateFastEDh + rateFastEDl;
			rateFinalER = 10 * rateFinalERh + rateFinalERl;
			rateFinalEP = 10 * rateFinalEPh + rateFinalEPl;
			rateFinalED = 10 * rateFinalEDh + rateFinalEDl;
			return ret;
		}
	};
	class pgContainer : public set<puzzleRecord> {};
	pgContainer theList;
	int size; //max 33
	pat ppp;
private:
	fskfr fastRater;
	int map[34]; //compressed to real position
	mutable std::shared_mutex mutex_;
	void updateRelabelDepthNoLock(puzzleRecord* hh, bool noSingles, rating_t depth) {
		rating_t newMask = depth & puzzleRecord::depthMask;
		if((hh->rateFinal & puzzleRecord::depthMask) < newMask) { //LSB = n = relabeled up to depth n (unconditionally)
			hh->rateFinal = (hh->rateFinal & (~puzzleRecord::depthMask)) | newMask;
		}
		if(!noSingles) {
			if((hh->rateFast & puzzleRecord::depthMask) < newMask) { //LSB = n = relabeled up to depth n (only when not ignoring the singles)
				hh->rateFast = (hh->rateFast & (~puzzleRecord::depthMask)) | newMask;
			}
		}
	}
	void updateRelabelDepth(puzzleRecord* hh, bool noSingles, rating_t depth) {
		std::unique_lock<std::shared_mutex> lock(mutex_);
		updateRelabelDepthNoLock(hh, noSingles, depth);
	}
	void updateRelabelDepth(puzzleRecordset& src, bool noSingles, rating_t depth) {
		std::unique_lock<std::shared_mutex> lock(mutex_);
		for(puzzleRecordset::iterator i = src.begin(); i != src.end(); i++) {
			puzzleRecord* hh = *i;
			updateRelabelDepthNoLock(hh, noSingles, depth);
		}
	}
	void compress(const uncomprPuz &u, puzzleRecord &c) const {
		for(int i = 0; i < 16; i++)
			c.key[i] = 0;
		if(0 == (size & 1)) { //half-byte
			c.key[size / 2 - 1] = u.p.chars[map[size - 1]]; //store the last given
		}
		for(int i = 1, j = 0; i < size - 1; j++, i += 2) { //start from the second given, the first is always "1"
			c.key[j] = u.p.chars[map[i]] | (u.p.chars[map[i + 1]] << 4);
		}
		c.rateFast = (((((u.rateFastED << 8) | u.rateFastEP) << 8) | u.rateFastER) << 8) | (u.specTransformedUpTo & puzzleRecord::depthMask) | ((u.minimality << 3) & puzzleRecord::minimalityMask);
		c.rateFinal = (((((u.rateFinalED << 8) | u.rateFinalEP) << 8) | u.rateFinalER) << 8) | (u.transformedUpTo & puzzleRecord::depthMask);
	}
	void uncompress(const puzzleRecord &c, uncomprPuz &u) const {
		u.p.clear();
		u.p.chars[map[0]] = 1;
		if(0 == (size & 1)) { //half-byte
			u.p.chars[map[size - 1]] = c.key[size / 2 - 1]; //store the last given
		}
		for(int i = 1, j = 0; i < size - 1; j++, i += 2) { //start from the second given, the first is always "1"
			u.p.chars[map[i]] = c.key[j] & 0x0F;
			u.p.chars[map[i + 1]] = c.key[j] >> 4;
		}
		u.specTransformedUpTo = c.rateFast & puzzleRecord::depthMask;
		u.minimality = (c.rateFast & puzzleRecord::minimalityMask) >> 3;
		rating_t t = c.rateFast >> 8;
		u.rateFastER = t & 0xFF;
		t >>= 8;
		u.rateFastEP = t & 0xFF;
		u.rateFastED = t >> 8;
		u.transformedUpTo = c.rateFinal & 0xFF;
		t = c.rateFinal >> 8;
		u.rateFinalER = t & 0xFF;
		t >>= 8;
		u.rateFinalEP = t & 0xFF;
		u.rateFinalED = t >> 8;
	}
	void init(const char *exemplar) {
		uncomprPuz u;
		size = u.fromString(exemplar);
		if(size < 17 || size > 33)
			return; //error, invalid pattern size
		for(int i = 0, j = 0; i < size; j++) {
			if(u.p.chars[j]) {
				map[i++] = j;
			}
		}
		ppp.init(exemplar);
		ppp.morphs.clear();
		ppp.morphIndex.clear();
	}
public:
	void load() {
		char buf[1000];
		int first = 1;
		while(fgets(buf, sizeof(buf), stdin)) {
			if(first) {
				init(buf);
				first = 0;
			}
			add(buf, 1);
		}
		if(opt.verbose) {
			fprintf(stderr, "Number of symmetries %d\n", ppp.numAutomorphisms);
			fprintf(stderr, "%d puzzles loaded\n", (int)theList.size());
		}
	}
	void fastRateAll() {
		for(pgContainer::iterator h = theList.begin(); h != theList.end(); h++) {
			if((h->rateFast & 0xFF00) == 0) { //ER was not set
				puzzleRecord* hh = const_cast<puzzleRecord*>(&(*h));
				uncomprPuz u;
				uncompress(*hh, u);
				fastRater.push(u.p.chars, &(hh->rateFast));
			}
		}
		fastRater.commit();
	}
	void add(const char* p, const int allowUpdates = 0) { //
		uncomprPuz u;
		puzzleRecord c;
		u.fromString(p);
		//test for pattern validity
		for(int i = 0; i < size; i++) {
			if(u.p.chars[map[i]] < 1 || u.p.chars[map[i]] > 9)
				return;
		}
		//minlex the source
		ppp.minlex(u.p);
		compress(u, c);
		pgContainer::iterator h = theList.lower_bound(c);
		//if(h != theList.end()) { //update
		if(h != theList.end() && c == *h) { //update
			if(allowUpdates) {
//				for(int i = 0; i < 8; i++) { //store the higher of the values
//					if(((unsigned char *)(&c.rateFast))[i] < ((unsigned char *)(&h->rateFast))[i])
//						((unsigned char *)(&c.rateFast))[i] = ((unsigned char *)(&h->rateFast))[i];
//				}
				puzzleRecord* hh = const_cast<puzzleRecord*>(&(*h));
				hh->merge(c);
				if((hh->rateFast & 0xFF00) == 0) { //calculate ER
					fastRater.push(u.p.chars, &(hh->rateFast));
				}
			}
		}
		else { //insert
			//make additional tests here
			if(0 == u.minimality) {
				//test for redundancy
				if(0 == solverIsIrreducibleByProbing(u.p.chars)) {
					c.rateFast |= ((rating_t)1 << 3);
					//return; //redundant clue(s)
				}
				else {
					c.rateFast |= ((rating_t)2 << 3);
				}
			}
			{
				//redundancy test passed
				//insert the puzzle, use h as a hint
				std::unique_lock<std::shared_mutex> lock(mutex_);
				pgContainer::iterator hhh = theList.insert(h, c);
				//rate the puzzle
				if((c.rateFast & puzzleRecord::rateMask) == 0) { //calculate ER
					puzzleRecord* hh = const_cast<puzzleRecord*>(&(*hhh));
					fastRater.push(u.p.chars, &(hh->rateFast));
				}
			} //critical
		}
	}
	void relN(unsigned int n, unsigned int minED, unsigned int maxED, unsigned int minEP, unsigned int maxEP, unsigned int minER, unsigned int maxER, unsigned int maxPasses, unsigned int noSingles, unsigned int onlyMinimals) {
		//static const pgContainer::size_type max_batch_sizes[] = {0,20000,2000,200,20,20,20,20,20,20,20}; //limit to some reasonable batch size
		inputFilter iFilter(n, minED, maxED, minEP, maxEP, minER, maxER, noSingles, onlyMinimals ? 2 : 0, 3);
		int resCount;
		//const pgContainer::size_type max_batch_size = max_batch_sizes[n];
		if(maxPasses == 0) maxPasses = 1;
		do {
			//get puzzles passing filter and not relabeled up to n (if any)
			puzzleRecordset src;
			resCount = 0;
			int redundantCount = 0;
			for(pgContainer::iterator h = theList.begin(); h != theList.end(); h++) {
				puzzleRecord* hh = const_cast<puzzleRecord*>(&(*h));
				if(iFilter.matches(*hh)) {
					src.insert(hh);
					resCount++;
					//if(resCount >= max_batch_size) break; //limit to some reasonable batch size
					if(! onlyMinimals) {
						if(!hh->isMinimal()) {
							redundantCount++;
						}
					}
				}
			}
			if(opt.verbose) {
				fprintf(stderr, "Relabel %d, %d passes left, processing %d + %d = %d items ", n, maxPasses, resCount - redundantCount, redundantCount, resCount);
			}
			//relabel & insert
			size_t percentage = 0;
			size_t resNum = 0;
			for(puzzleRecordset::iterator i = src.begin(); i != src.end(); i++) {
				if(gExiting) continue;
				puzzleRecord* hh = *i;
				uncomprPuz u;
				uncompress(*hh, u);
//				puzzleSet found;
//				solverRelabel(u.p.chars, n, false, true, noSingles, relCallBackLocal_unused, &found); //checking for minimality here is slower
//				for(puzzleSet::const_iterator lc = found.begin(); lc != found.end(); lc++) {
//					//add(lc->chars, 0, 1);
//					add(lc->chars, 0);
//				}
				solverRelabel(u.p.chars, n, false, true, noSingles, relCallBackLocal, this); //checking for minimality here is slower
				updateRelabelDepth(hh, noSingles, n);
				if(opt.verbose) {
					resNum++;
					size_t new_percentage = resNum * 10 / resCount;
					if(percentage < new_percentage) {
						percentage = new_percentage;
						fprintf(stderr, "%c", new_percentage > 5 ? '\\' : '/');
					}
				}
			}
			fastRater.commit();
//			//This pass is done. Mark the seed as used at appropriate depth.
//			if(!gExiting) {
//				updateRelabelDepth(src, noSingles, n);
//			}
			if(opt.verbose) {
				fprintf(stderr, "\n");
			}
		} while((!gExiting) && resCount && --maxPasses > 0);
	}

	//this is called by solverRelabel on any puzzle candidate
	//which a) has single completion, and b) conforms the noSingles filter
	//but minimality is still to be checked
//	static int relCallBackLocal_unused(const char *puz, void *context) {
//		puzzleSet *pFound = static_cast< puzzleSet * > (context);
//		ch81 p;
//		p.toString(puz, p.chars);
//		pFound->insert(p);
//		return 0;
//	}
	static int relCallBackLocal(const char *puz, void *context) {
		pgGotchi *this_ = static_cast< pgGotchi * > (context);
		ch81 p;
		p.toString(puz, p.chars);
		this_->add(p.chars, 0);
		return 0;
	}

//	//this is called by solverRelabel on any puzzle candidate
//	//which a) has single completion, and b) conforms the noSingles filter
//	//but minimality is still to be checked
//	static int relCallBack(const char *puz, const void *context) {
//		ch81 pText;
//		pgGotchi *pg = static_cast< pgGotchi* > (const_cast<void*>(context));
//		pText.toString(puz, pText.chars);
//		char buf[88];
//		memcpy(buf, pText.chars, 81);
//		buf[81] = 0;
//		//sprintf(buf, "%81.81s\n", pText.chars);
////#ifdef _OPENMP
////		//TODO: move the critical section after the minimality check loop if possible
////#pragma omp critical
////#endif
//		pg->add(buf, 0, 1);
//		return 0;
//	}
	void dump() {
		for(pgContainer::iterator h = theList.begin(); h != theList.end(); h++) {
			uncomprPuz u;
			uncompress(*h, u);
			char buf[256];
			u.toString(buf);
			printf("%s", buf);
			//fflush(NULL);
		}
		fflush(NULL);
	}
	static inline unsigned int popCount32(unsigned int v) { // count bits set in this (32-bit value)
		v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
		v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
		return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
	}
	int distance(const puzzleRecord &p1, const puzzleRecord &p2) const { //get minimal hamming distance, print p2 unchanged
		if(size > 32)
			return -1;
		if(ppp.numAutomorphisms > 8)
			return -1;
		uncomprPuz u1, u2;
		uncompress(p1, u1);
		uncompress(p2, u2);
		int b1[8][9], b2[9]; //32-bit bitmaps corresponding to each given's positions in the mapped coordinate system
		char m1[8][81];
		//get all positional morphs of the first puzzle
		ppp.getPosMorphs(u1.p.chars, m1[0]);
		//accumulate the givens per digit for positional morphs of the first puzzle into bitmaps
		for(int i = 0; i < 8; i++) {
			for(int j = 0; j < 9; j++) {
				b1[i][j] = 0;
			}
		}
		for(int j = 0; j < ppp.numAutomorphisms; j++) {
			for(int i = 0; i < size; i++) {
				b1[j][m1[j][map[i]] - 1] |= Digit2Bitmap[i + 1]; //b1 is 0-based, bitmaps are 1-based
			}
		}
		//for(int i = 0; i < 9; i++) {
		//	int x = b1[0][i];
		//	for(int j = 0; j < size; j++) {
		//		fprintf(stderr, "%d", (x >> j) & 1);
		//	}
		//	fprintf(stderr, "\t%d\n", i + 1);
		//}
		//accumulate the givens per digit of the second puzzle into bitmaps
		for(int i = 0; i < 9; i++) {
			b2[i] = 0;
		}
		for(int i = 0; i < size; i++) {
			b2[u2.p.chars[map[i]] - 1] |= Digit2Bitmap[i + 1];
		}
		int minBC = INT_MAX; //minimal bitcount = 2 * minimal hamming distance
		int r[9]; //p1 givens relabeled to
		for(int i = 0; i < ppp.numAutomorphisms; i++) { //iterate all positional morphs of p1
			//do 9 nested loops (all possible relabelings of p1) finding the least distance
			for(r[0] = 0; r[0] < 9; r[0]++) { //digit to relabel "1" in p1 to
				int d1set = Digit2Bitmap[r[0] + 1];
				int bc1 = popCount32(b1[i][0] ^ b2[r[0]]); //number of unmatching positions of digit (r[0] + 1) in p1 and digit 1 in p2
				if(bc1 > minBC)
					continue; //a better match already has been found in previous iterations
				for(r[1] = 0; r[1] < 9; r[1]++) {
					if(Digit2Bitmap[r[1] + 1] & d1set) // don't relabel >1 digits to the same one
						continue;
					int d2set = d1set | Digit2Bitmap[r[1] + 1];
					int bc2 = bc1 + popCount32(b1[i][1] ^ b2[r[1]]);
					if(bc2 > minBC)
						continue;
					for(r[2] = 0; r[2] < 9; r[2]++) {
						if(Digit2Bitmap[r[2] + 1] & d2set)
							continue;
						int d3set = d2set | Digit2Bitmap[r[2] + 1];
						int bc3 = bc2 + popCount32(b1[i][2] ^ b2[r[2]]);
						if(bc3 > minBC)
							continue;
						for(r[3] = 0; r[3] < 9; r[3]++) {
							if(Digit2Bitmap[r[3] + 1] & d3set)
								continue;
							int d4set = d3set | Digit2Bitmap[r[3] + 1];
							int bc4 = bc3 + popCount32(b1[i][3] ^ b2[r[3]]);
							if(bc4 > minBC)
								continue;
							for(r[4] = 0; r[4] < 9; r[4]++) {
								if(Digit2Bitmap[r[4] + 1] & d4set)
									continue;
								int d5set = d4set | Digit2Bitmap[r[4] + 1];
								int bc5 = bc4 + popCount32(b1[i][4] ^ b2[r[4]]);
								if(bc5 > minBC)
									continue;
								for(r[5] = 0; r[5] < 9; r[5]++) {
									if(Digit2Bitmap[r[5] + 1] & d5set)
										continue;
									int d6set = d5set | Digit2Bitmap[r[5] + 1];
									int bc6 = bc5 + popCount32(b1[i][5] ^ b2[r[5]]);
									if(bc6 > minBC)
										continue;
									for(r[6] = 0; r[6] < 9; r[6]++) {
										if(Digit2Bitmap[r[6] + 1] & d6set)
											continue;
										int d7set = d6set | Digit2Bitmap[r[6] + 1];
										int bc7 = bc6 + popCount32(b1[i][6] ^ b2[r[6]]);
										if(bc7 > minBC)
											continue;
										for(r[7] = 0; r[7] < 9; r[7]++) {
											if(Digit2Bitmap[r[7] + 1] & d7set)
												continue;
											int d8set = d7set | Digit2Bitmap[r[7] + 1];
											int bc8 = bc7 + popCount32(b1[i][7] ^ b2[r[7]]);
											if(bc8 > minBC)
												continue;
											for(r[8] = 0; r[8] < 9; r[8]++) {
												if(Digit2Bitmap[r[8] + 1] & d8set)
													continue;
												int bc9 = bc8 + popCount32(b1[i][8] ^ b2[r[8]]);
												if(bc9 <= minBC) { //a relabeling closer than all previous is found
													minBC = bc9;
													if(opt.verbose) {
														ch81 x, z;
														x.clear();
														z.clear();
														int rr[9]; //reverse relabelling
														for(int j = 0; j < 9; j++) rr[r[j]] = j;
														for(int j = 0; j < 9; j++) fprintf(stderr, "%d", rr[j] + 1);
														for(int j = 0; j < size; j++) {
															z.chars[map[j]] = m1[i][map[j]]; //p1
															x.chars[map[j]] = 1 + rr[u2.p.chars[map[j]] - 1]; //p2
														}
														ch81 y;
														z.toString(y.chars);
														fprintf(stderr, "\n%81.81s\n", y.chars);
														x.toString(y.chars);
														fprintf(stderr, "%81.81s\t%d\n", y.chars, minBC / 2);
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return minBC / 2;
	}
	void getPlayablePuzzles(std::vector<std::string>& puzzletList) {
		inputFilter iFilter(0, 12, 120, 12, 120, 12, 120, 0, 2, 2); //ER>0 && EP>0 && ED>0 && minimal
		//compose array of best puzzles for each final ER
		uncomprPuz best_records[256];
		for(pgContainer::iterator h = theList.begin(); h != theList.end(); h++) {
			puzzleRecord* hh = const_cast<puzzleRecord*>(&(*h));
			if(iFilter.matchesRateFinal(*hh)) {
				rating_t test_ER = hh->getRateFinalER();
				if(test_ER > 120) continue; //bug???
				if(
						(best_records[test_ER].rateFinalEP < hh->getRateFinalEP()) //better due to EP
						||
						(best_records[test_ER].rateFinalEP == hh->getRateFinalEP() && best_records[test_ER].rateFinalED < hh->getRateFinalED()) //better due to ED
					) {
					uncomprPuz best_puzzle;
					uncompress(*hh, best_puzzle);
					best_records[test_ER] = best_puzzle;
				}
			}
		}
		// copy the best puzzles to the output
		char buf[1000];
		for(int i = 120; i >= 0; i--) {
			if(0 == best_records[i].rateFastEP) continue; //no puzzles for this ER
			best_records[i].toString(buf);
			puzzletList.push_back(buf);
		}
	}
};

int probe_map_callback (void *context, const char *puz, char *m, const morph &theMorph) {
	(void)puz; (void)theMorph; //suppress compiler warnings
	char *p = static_cast<char*>(context);
	memcpy(p, m, 81);
	return 1; //exit on first call
}

void scanCollection(const char *ppuz){
	ch81 exemplar;
	int size = exemplar.fromString(ppuz);
	pat ppp;
	ppp.init(ppuz);
	int transfCount = ppp.morphs.size();
//	bm128 patBm;
//	patBm.clear();
//	for(int i = 0; i < 81; i++) {if(exemplar.chars[i]) patBm.setBit(i);}
	int givenPositions[88];
	for(int i = 0, n = 0; i < 81; i++) {
		if(exemplar.chars[i]) givenPositions[n++] = i;
	}
	fprintf(stdout, "%81.81s original\n", ppuz);

	//sort morphs for later binary search
	bm128IntMap allMorphs;
	bm128IntsMap allMorphsMinus1;
	for(int i = 0; i < transfCount; i++) {
		allMorphs[ppp.morphs[i]] = i;
		for(int p = 0; p < size; p++) {
			bm128 tmp(ppp.morphs[i]);
			tmp.clearBit(givenPositions[p]);
			allMorphsMinus1[tmp].push_back(i);
		}
	}

//	//how the distribution looks like?
//	int counts[100] = {0};
//	for(bm128IntsMap::const_iterator m = allMorphsMinus1.begin(); m != allMorphsMinus1.end(); m++) {
//		if(m->second.size() < 100) counts[m->second.size()]++;
//	}
//	for(int i = 0; i < 100; i++) {
//		fprintf(stderr, "%d\t%d\n", i, counts[i]);
//	}

	//scan the collection
	char buf[1000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 hard;
		ch81 found;
		ch81 foundText;
		int hSize = hard.fromString(buf);
		bm128 hardBm;
		switch(size - hSize) {
		case 0:		//exact size match
			hardBm.clear();
			for(int i = 0; i < 81; i++) {if(hard.chars[i]) hardBm.setBit(i);}
			{
				bm128IntMap::const_iterator foundAt = allMorphs.find(hardBm);
				if(foundAt != allMorphs.end()) {
					//compose transformation map
					ch81 map;
					morph::processSingleMorph(map.chars, probe.chars, ppp.morphIndex[foundAt->second], &probe_map_callback);
					for(int j = 0; j < 81; j++) {found.chars[(int)(map.chars[j])] = hard.chars[j];}
					found.toString(foundText.chars);
					fprintf(stdout, "%81.81s = %s\n", foundText.chars, buf + 81);
					fflush(NULL);
				}
			}
			break;
		case 1:		//one less clue
			hardBm.clear();
			for(int i = 0; i < 81; i++) {if(hard.chars[i]) hardBm.setBit(i);}
			{
				bm128IntsMap::const_iterator foundAt = allMorphsMinus1.find(hardBm);
				if(foundAt != allMorphsMinus1.end()) {
					for(bm128IntsMap::value_type::second_type::size_type n = 0; n < foundAt->second.size(); n++) {
						//compose transformation map
						ch81 map;
						morph::processSingleMorph(map.chars, probe.chars, ppp.morphIndex[foundAt->second[n]], &probe_map_callback);
						for(int j = 0; j < 81; j++) {found.chars[(int)(map.chars[j])] = hard.chars[j];}
						int redundantGivenPosition;
						solve(found.chars, 1, foundText.chars); //find missing value
						for(int j = 0; j < 81; j++) {
							if(exemplar.chars[j] && (0 == found.chars[j])) {
								redundantGivenPosition = j;
								found.chars[redundantGivenPosition] = foundText.chars[redundantGivenPosition];	//this is the hole
								break;
							}
						}
						found.toString(foundText.chars);
						fprintf(stdout, "%81.81s -%d %s\n", foundText.chars, redundantGivenPosition, buf + 81);
						fflush(NULL);
					}
				}
			}
			break;
		default:
			;
		}
	}
}
int gotchiPass(const char *cmd) {
	//parse command
	if(*cmd == 's') {
		scanCollection(cmd + 1); // --patterns --pg s<puzzle> < collection.txt > found.txt
		return 0;
	}
	unsigned int maxRel = 1;
	unsigned int minED = 0;
	unsigned int maxED = 255;
	unsigned int minEP = 0;
	unsigned int maxEP = 255;
	unsigned int minER = 0;
	unsigned int maxER = 255;
	unsigned int maxPasses = 1;
	unsigned int noSingles = 0;
	unsigned int onlyMinimals = 0; // 01=non-minimal; 10=minimal
	sscanf(cmd, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", &maxRel, &minER, &maxER, &minEP, &maxEP, &minED, &maxED, &maxPasses, &noSingles, &onlyMinimals);
	pgGotchi g;
	g.load();
	if(*cmd == 'd') {
		const puzzleRecord *p2 = &(*g.theList.begin());
		for(pgGotchi::pgContainer::iterator h = std::next(g.theList.begin()); h != g.theList.end(); h++) {
			const puzzleRecord* p1 = const_cast<puzzleRecord*>(&(*h));
			g.distance(*p1, *p2);
		}
		return 0;
	}
	g.fastRateAll();
	if(maxRel) { //for relabel = 0 rate only
		g.relN(maxRel, minED, maxED, minEP, maxEP, minER, maxER, maxPasses, noSingles, onlyMinimals);
	}
	//g.distance((*g.theList.begin()), (*g.theList.rbegin()));
	if(opt.verbose) {
		fprintf(stderr, "%d puzzles done\n", (int)g.theList.size());
	}
	g.dump();
	return 0;
}
//class task {
//public:
//	enum taskType {
//		taskRelabel,
//		taskApproxRate,
//		taskRate
//	} type;
//	string inputParameters;
//	string outputParameters;
//	std::chrono::steady_clock::time_point sentTime;
//	//sentTime = std::chrono::steady_clock::now();
//};
int apiListen(const char *host, int port);
class pgDb {
	char username[32];
	char password[32];
	char listeningAddress[100];
	int listeningPort;
public:
	pgGotchi g;
public:
	void load(const char *cmd) {
		//parse the command-line parameters
		sscanf(cmd, "%31[^:]:%31[^@]@%100[^:]:%d", username, password, listeningAddress, &listeningPort);
		if(opt.verbose) {
			fprintf(stderr, "Starting server with\nUsername=%s\nPassword=%s\nAddress=%s\nPort=%u\n", username, password, listeningAddress, listeningPort);
		}
		//load the puzzles (part of) file
		g.load();
		if(opt.verbose) {
			fprintf(stderr, "Running the listener. Press Ctrl-C to shutdown...\n");
		}
	}
	void dump() {
		g.dump();
	}
	void serve() {
		while (!gExiting) { //this should be unnecessary
			apiListen(listeningAddress, listeningPort);
			//std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
};

pgDb* Db = NULL; //use dirty global var (to postpone separation of the above class declarations from implementations)
int pgServer(const char *cmd) {
	Db = new pgDb();
	Db->load(cmd);
	Db->serve(); //blocks until Ctrl-C
	Db->dump();
	return 0;
}

/////////////////////
///
/// API functions
void getPlayablePuzzles(std::vector<std::string>& puzzletList) {
	Db->g.getPlayablePuzzles(puzzletList);
}

/////////////////////
extern int processPatterns() {
	if(opt.patternOpt->redundancy) {// --pattern --redundancy < in.txt > out.txt
		return redundancy();
	}
	else if(opt.patternOpt->statistics) {// --pattern --statistics < in.txt > out.txt
		return statistics();
	}
	else if(opt.patternOpt->enumerate) { // --pattern --enumerate [--subcanon] > out.txt
		return enumerate(opt.patternOpt->enumerate, opt.patternOpt->fixclues);
	}
	else if(opt.patternOpt->pg) { // --pattern --pg <commands> < in.txt > out.txt
		return gotchiPass(opt.patternOpt->pg);
	}
	else if(opt.patternOpt->pgserver) { // --pattern --pgserver user:password@address:port < in.txt > out.txt
		return pgServer(opt.patternOpt->pgserver);
	}
	return -1;
}
