#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "options.h"
#include "tables.h"
#include "t_128.h"
#include "ch81.h"
#include "solver.h"
#include "grid.h"
#include "rowminlex.h"  //subcanon()
#include "patminlex.h"
//#include "BitMask768.h"

#include <map>
#include <algorithm>
using namespace std;

//struct int3to8 {
//	int c[6];
//	ch81 r;
//	int3to8() {
//		for(int i = 0; i < 6; i++)
//			c[i] = 0;
//	}
//};
//
//struct mapR4: public map<ch81,int3to8> {};

//void test() { //read 4-templates from stdin and write (un)sorted 4-rookeries to stdout along with some statictics
//	mapR4 r4;
//	int n = 0;
//	char buf[1000];
//	while(fgets(buf, sizeof(buf), stdin)) {
//		n++;
//		if(*buf == 0) {
//			fprintf(stderr, "Empty line %d\n", n);
//			continue; //silently ignore empty lines
//		}
//		int v = 0;
//		int err = sscanf(buf + 82, "%d", &v); //completion count
//		if(v < 3 || v > 8) { //line 22717580
//			fprintf(stderr, "Ignoring %d at line %d\t(%s)\t%d\n", v, n, buf, err);
//			continue;
//		}
//		ch81 puz;
//		int puzSize = puz.fromString(buf);
//		ch81 p1 = puz; //structure copy
//		for(int i = 0; i < 81; i++) {
//			if(p1.chars[i]) {
//				p1.chars[i] = 1;
//			}
//		}
//		ch81 pcan;
//		subcanon(p1.chars, pcan.chars); //pattern
//		int3to8 &x = r4[pcan];
//		x.c[v - 3]++; //completion count
//		x.r = puz; //representative
//	}
//	for(mapR4::const_iterator p = r4.begin(); p != r4.end(); p++) {
//		const int3to8 &x = p->second;
//		p->first.toString(buf);
//		x.r.toString(buf + 88);
//		int s = x.c[0] + x.c[1] + x.c[2] + x.c[3] + x.c[4] + x.c[5];
//		printf("%81.81s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%81.81s\n", buf, s, x.c[0], x.c[1], x.c[2], x.c[3], x.c[4], x.c[5], buf + 88);
//	}
//	fprintf(stderr, "\nDone: %d templates form %d rookeries\n", n, r4.size());
//}
//
void getGrid23Templates() {
	puzzleSetTagInt t2;
	puzzleSetTagInt t3;
	t2.loadFromFile("t2clues.txt", false);
	t3.loadFromFile("t3clues.txt", false);

	bm128 dTemplates[9];
	bm128 templates2[36];
	bm128 templates3[84];
	ch81 tCan2[36];
	ch81 tCan3[84];
	int compl2[36];
	int compl3[84];
	ch81 puz;

	char buf[2000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 sol;
		//int puzSize = sol.fromString(buf);
		sol.fromString(buf);

		//find all 1-templates
		for(int i = 0; i < 9; i++) {
			dTemplates[i].clear();
		}
		for(int i = 0; i < 81; i++) {
			dTemplates[sol.chars[i] - 1].setBit(i);
		}

		//find all 36 2-templates
		for(int i = 0; i < 36; i++) {
			templates2[i] = dTemplates[choice2of9[i][0]] | dTemplates[choice2of9[i][1]];
			puz.clear();
			for(int p = 0; p < 81; p++) {
				if(templates2[i].isBitSet(p))
					puz.chars[p] = sol.chars[p];
			}
			subcanon(puz.chars, tCan2[i].chars);
			compl2[i] = t2.find(tCan2[i])->second;
		}
		//find all 84 3-templates
		for(int i = 0; i < 84; i++) {
			templates3[i] = dTemplates[choice3of9[i][0]] | dTemplates[choice3of9[i][1]];
			templates3[i] |= dTemplates[choice3of9[i][2]];
			puz.clear();
			for(int p = 0; p < 81; p++) {
				if(templates3[i].isBitSet(p))
					puz.chars[p] = sol.chars[p];
			}
			subcanon(puz.chars, tCan3[i].chars);
			compl3[i] = t3.find(tCan3[i])->second;
		}

		int min2223 = 100;
		int max2223 = 0;
		int sum2223 = 0;
		int min333 = 100;
		int max333 = 0;
		int sum333 = 0;

		for(int i = 0; i < 1260; i++) {
			int c = compl2[choice2223of9[i][0]] + compl2[choice2223of9[i][1]] + compl2[choice2223of9[i][2]] + compl3[choice2223of9[i][3]];
			sum2223 += c;
			if(min2223 > c)
				min2223 = c;
			if(max2223 < c)
				max2223 = c;
		}
		for(int i = 0; i < 280; i++) {
			int c = compl3[choice333of9[i][0]] + compl3[choice333of9[i][1]] + compl3[choice333of9[i][2]];
			sum333 += c;
			if(min333 > c)
				min333 = c;
			if(max333 < c)
				max333 = c;
		}
		printf("%81.81s\t%d\t%d\t%5.3f\t%d\t%d\t%5.3f\n", buf, min2223, max2223, sum2223 / 1260.0, min333, max333, sum333 / 280.0);

		//extern const int choice2223of9[1260][4];
		//extern const int choice333of9[280][3];
		//typedef pair <ch81, int> Ch81_Int_Pair;
		//struct puzzleSetTagInt : public map<ch81, int> {

		//find all disjoint a(2)+b(2)+c(2)+t(3) templates
	}
}

//struct rookeryWithTemplates : public map<bm128, puzzleSet, less<bm128>, mm_allocator<bm128>> {};
struct templateWithExemplar {
	ch81 can;
	ch81 exemplar;
	bool operator < (const templateWithExemplar& other) const {
		return can < other.can;
	}
};
struct exemplarSet : public set<templateWithExemplar> {};
//typedef bit_mask<5184> colTemplates_index_type;

struct templates {
	bm128 allTemplates[46656];
	bm128 colTemplates[9][5184];
//	colTemplates_index_type colTemplatesIndexes[9][81];
//	colTemplates_index_type colTemplatesIndexAll[9];
	void init();
	unsigned long long nSol(const char *p);
	templates();
	//void getComplementaryTemplates() const;
	void get2templates(puzzleSet& t2all) const;
	void get2rookeries(lightweightUsetList& r2all) const;
	void get3templates(puzzleSet& t3all) const;
	void get3rookeries(lightweightUsetList& r3all) const;
	void get4rookeries(lightweightUsetList& r4all) const;
	void rookeryPlus1(const bm128& src, lightweightUsetList& res) const;
	void rookeriesPlus1(lightweightUsetList& src, lightweightUsetList& res) const;
	void templatePlus1();
	void templatePlus1(const ch81& src, puzzleSet& res) const;
	void templatesPlus1(const puzzleSet& src, puzzleSet& res) const;
	//void templates2rookeries() const; //uses subcanon instead of patcanon
	void rookery2templates(const bm128& r, puzzleSet& res, bool first = false) const;
	int rookery2templates4(const bm128& r, exemplarSet& res) const;
	int rookery2templates5(const bm128& r, exemplarSet& res) const;
	void template2rookery(const ch81& src, bm128& r) const;
	void count6templates() const;
	void count5templates() const;
	void countGrids() const;
	void generateGrids() const;
	void generateXX110() const;
	int getIndexes(const ch81& src, unsigned short* res) const;
	void get999911110() const; //from 5-templates get those completable by 4 clues and print the puzzles
};

templates::templates() {
	init();
	//get2templates();
	//get3templates();
	//get3rookeries();
	//templatePlus1();
	//getComplementaryTemplates();
	//templates2rookeries();
}

int templates::getIndexes(const ch81& src, unsigned short* res) const {
	int numFound = 0;
	bm128 t[10];
	for(int i = 0; i < 10; i++) t[i].clear();
	for(int c = 0; c < 81; c++) {
		t[(int)src.chars[c]].setBit(c);
	}
	for(int d = 1; d < 10; d++) {
		if(t[d].popcount_128() != 9) continue;
		int col = -1;
		for(int c = 0; c < 9; c++) {
			if(t[d].isBitSet(c)) {
				col = c;
				break;
			}
		}
		if(col != -1) {
			for(int i = 0; i < 5184; i++) {
				if(t[d].isSubsetOf(colTemplates[col][i])) {
					res[numFound] = col * 5184 + i;
					numFound++;
					break;
				}
			}
		}
	}
	std::sort(&res[0], &res[numFound]);
	return numFound;
}

void templates::template2rookery(const ch81& src, bm128& r) const {
	r.clear();
	ch81 can;

//	ch81 tmp;
//	for(int i = 0; i < 81; i++) {
//		tmp.chars[i] = src.chars[i] ? 1 : 0;
//	}
//	patminlex rml(tmp.chars, can.chars);
	patminlex rml(src.chars, can.chars, patminlex::action::findMinPattern);

	for(int i = 0; i < 81; i++) {
		if(can.chars[i]) {
			r.setBit(80 - i);
		}
	}
}

//void templates::getComplementaryTemplates() const { //read k-rookeries from stdin and count complementary templates. Works well for 5-templates from 4-rookeries.
//	char buf[2000];
//	//unsigned long long nTempl = 0;
//	unsigned long long progress = 0;
//	const unsigned long long maxSol = 10000000;
//	//ch81 p1[maxSol];
//	ch81 *p1 = (ch81*)malloc(maxSol * sizeof(ch81));
//	while(fgets(buf, sizeof(buf), stdin)) {
//		ch81 puz;
//		int puzSize = puz.fromString(buf);
//		unsigned long long nSol;
//		ch81 solCan;
//		//fix the non-givens in the first row
//		int n = puzSize / 9; //max given label
//		for(int i = 0; i < 9; i++) {
//			if(puz.chars[i] == 0) {
//				puz.chars[i] = ++n;
//			}
//		}
//		n = puzSize / 9;
//		nSol = solve(puz.chars, maxSol, p1[0].chars);
//		if(nSol == maxSol) {
//			fprintf(stderr, "Maximum of 100000 solutions reached: %81.81s\n", buf);
//			continue;
//		}
//		puzzleSet templ;
//		for(unsigned int s = 0; s < nSol; s++) {
//			for(int i = 0; i < 81; i++) {
//				if(p1[s].chars[i] <= n) {
//					p1[s].chars[i] = 0;
//				}
//			}
//			subcanon(p1[s].chars, solCan.chars);
//			templ.insert(solCan);
//		}
//		//templ.saveToFile(stdout);
//		fprintf(stdout, "%81.81s\t%d\n", buf, (int)templ.size());
//		fflush(stdout);
//		//nTempl += templ.size();
//		templ.clear();
//		if(++progress % 300 == 0) {
//			fprintf(stderr, "%llu\n", progress);
//		}
//	}
//	free(p1);
//	//printf("%llu\n", nTempl);
//}

void templates::templatePlus1() { //read k-templates from stdin, store k+1 templates to files 000.txt, 001.txt, etc.
	char buf[2000];
	puzzleSet kp1t;
	int fileNumber = 0;
	char fileName[16];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 p;
		p.fromString(buf);
		bm128 kp;
		kp.clear();
		for(int i = 0; i < 81; i++) {
			if(p.chars[i]) {
				kp.setBit(i);
			}
		}
		for(int tn = 0; tn < 46656; tn++) {
			if(kp.isDisjoint(allTemplates[tn])) {
				ch81 pp1 = p; //structure copy
				for(int i = 0; i < 81; i++) {
					if(allTemplates[tn].isBitSet(i)) {
						pp1.chars[i] = 9;
					}
				}
				ch81 can;
				subcanon(pp1.chars, can.chars);
				kp1t.insert(can);
				if(kp1t.size() > 15000000) {
					//store the chunk to disk
					sprintf(fileName, "%3.3d.txt", fileNumber++);
					kp1t.saveToFile(fileName);
					kp1t.clear();
				}
			}
		}
	}
	sprintf(fileName, "%3.3d.txt", fileNumber++);
	kp1t.saveToFile(fileName);
}
void templates::rookeryPlus1(const bm128& src, lightweightUsetList& res) const {
	ch81 tmp;
	for(int i = 0; i < 81; i++) {
		tmp.chars[i] = src.isBitSet(i) ? 1 : 0;
	}
	for(int col = 0; col < 9; col++) {
		if(tmp.chars[col]) continue; //this doesn't help much
		for(int tn = 0; tn < 5184; tn++) {
			if(src.isDisjoint(colTemplates[col][tn])) {
				ch81 p = tmp; //structure copy
				for(int i = 0; i < 81; i++) {
					if(colTemplates[col][tn].isBitSet(i)) {
						p.chars[i] = 1;
					}
				}
				bm128 r;
				template2rookery(p, r);
				res.insert(r);
			}
		}
	}
}
void templates::templatesPlus1(const puzzleSet& src, puzzleSet& res) const {
	for(puzzleSet::const_iterator s = src.begin(); s != src.end(); s++) {
		templatePlus1(*s, res);
	}
}
void templates::rookeriesPlus1(lightweightUsetList& src, lightweightUsetList& res) const {
	for(lightweightUsetList::const_iterator r = src.begin(); r != src.end(); r++) {
		rookeryPlus1(*r, res);
	}
}

void templates::templatePlus1(const ch81& src, puzzleSet& res) const {
	bm128 kp;
	kp.clear();
	for(int i = 0; i < 81; i++) {
		if(src.chars[i]) {
			kp.setBit(i);
		}
	}
	for(int col = 0; col < 9; col++) {
		if(src.chars[col]) continue; //this doesn't help much
		for(int tn = 0; tn < 5184; tn++) {
			if(kp.isDisjoint(colTemplates[col][tn])) {
				ch81 p = src; //structure copy
				for(int i = 0; i < 81; i++) {
					if(colTemplates[col][tn].isBitSet(i)) {
						p.chars[i] = 9;
					}
				}
				ch81 can;
				patminlex rml(p.chars, can.chars);
				res.insert(can);
			}
		}
	}
}
//void templates::rookery2templates1(const bm128& r, puzzleSet& res, bool first) const {
//	int rsize = 0;
//	int r1row[9];
//	for(int i = 0; i < 9; i++) {
//		if(r.isBitSet(i)) {
//			r1row[rsize] = i;
//			rsize++;
//		}
//	}
//	switch(rsize) {
//		case 2:
//			for(int c0 = 0; c0 < 5184; c0++) {
//				bm128 r0(colTemplates[r1row[0]][c0]);
//				if(!r0.isSubsetOf(r)) continue;
//				ch81 p1;
//				p1.clear();
//				for(int i = 0; i < 81; i++) {
//					if(r0.isBitSet(i)) {
//						p1.chars[i] = 1;
//					}
//				}
//				for(int c1 = 0; c1 < 5184; c1++) {
//					bm128 r1(colTemplates[r1row[1]][c1]);
//					if(!r1.isSubsetOf(r)) continue;
//					if(!r1.isDisjoint(r0)) continue;
//					ch81 p2 = p1; //structure copy
//					for(int i = 0; i < 81; i++) {
//						if(r1.isBitSet(i)) {
//							p2.chars[i] = 2;
//						}
//					}
//					ch81 can;
//					patminlex pml(p2.chars, can.chars);
//					res.insert(can);
//					if(first) return;
//				}
//			}
//			break;
//		case 3:
//			for(int c0 = 0; c0 < 5184; c0++) {
//				bm128 r0(colTemplates[r1row[0]][c0]);
//				if(!r0.isSubsetOf(r)) continue;
//				ch81 p1;
//				p1.clear();
//				for(int i = 0; i < 81; i++) {
//					if(r0.isBitSet(i)) {
//						p1.chars[i] = 1;
//					}
//				}
//				for(int c1 = 0; c1 < 5184; c1++) {
//					bm128 r1(colTemplates[r1row[1]][c1]);
//					if(!r1.isSubsetOf(r)) continue;
//					if(!r1.isDisjoint(r0)) continue;
//					ch81 p2 = p1; //structure copy
//					for(int i = 0; i < 81; i++) {
//						if(r1.isBitSet(i)) {
//							p2.chars[i] = 2;
//						}
//					}
//					r1 |= r0;
//					for(int c2 = 0; c2 < 5184; c2++) {
//						bm128 r2(colTemplates[r1row[2]][c2]);
//						if(!r2.isSubsetOf(r)) continue;
//						if(!r2.isDisjoint(r1)) continue;
//						ch81 p3 = p2; //structure copy
//						for(int i = 0; i < 81; i++) {
//							if(colTemplates[r1row[2]][c2].isBitSet(i)) {
//								p3.chars[i] = 3;
//							}
//						}
//						ch81 can;
//						patminlex pml(p3.chars, can.chars);
//						res.insert(can);
//						if(first) return;
//					}
//				}
//			}
//			break;
//		case 4:
//			for(int c0 = 0; c0 < 5184; c0++) {
//				bm128 r0(colTemplates[r1row[0]][c0]);
//				if(!r0.isSubsetOf(r)) continue;
//				ch81 p1;
//				p1.clear();
//				for(int i = 0; i < 81; i++) {
//					if(r0.isBitSet(i)) {
//						p1.chars[i] = 1;
//					}
//				}
//				for(int c1 = 0; c1 < 5184; c1++) {
//					bm128 r1(colTemplates[r1row[1]][c1]);
//					if(!r1.isSubsetOf(r)) continue;
//					if(!r1.isDisjoint(r0)) continue;
//					ch81 p2 = p1; //structure copy
//					for(int i = 0; i < 81; i++) {
//						if(r1.isBitSet(i)) {
//							p2.chars[i] = 2;
//						}
//					}
//					r1 |= r0;
//					for(int c2 = 0; c2 < 5184; c2++) {
//						bm128 r2(colTemplates[r1row[2]][c2]);
//						if(!r2.isSubsetOf(r)) continue;
//						if(!r2.isDisjoint(r1)) continue;
//						ch81 p3 = p2; //structure copy
//						for(int i = 0; i < 81; i++) {
//							if(colTemplates[r1row[2]][c2].isBitSet(i)) {
//								p3.chars[i] = 3;
//							}
//						}
//						r2 |= r1;
//						for(int c3 = 0; c3 < 5184; c3++) {
//							bm128 r3(colTemplates[r1row[3]][c3]);
//							if(!r3.isSubsetOf(r)) continue;
//							if(!r3.isDisjoint(r2)) continue;
//							ch81 p4 = p3; //structure copy
//							for(int i = 0; i < 81; i++) {
//								if(colTemplates[r1row[3]][c3].isBitSet(i)) {
//									p4.chars[i] = 4;
//								}
//							}
//							ch81 can;
//							patminlex pml(p4.chars, can.chars);
//							res.insert(can);
//							if(first) return;
//						}
//					}
//				}
//			}
//			break;
//		case 5:
//			for(int c0 = 0; c0 < 5184; c0++) {
//				bm128 r0(colTemplates[r1row[0]][c0]);
//				if(!r0.isSubsetOf(r)) continue;
//				ch81 p1;
//				p1.clear();
//				for(int i = 0; i < 81; i++) {
//					if(r0.isBitSet(i)) {
//						p1.chars[i] = 1;
//					}
//				}
//				for(int c1 = 0; c1 < 5184; c1++) {
//					bm128 r1(colTemplates[r1row[1]][c1]);
//					if(!r1.isSubsetOf(r)) continue;
//					if(!r1.isDisjoint(r0)) continue;
//					ch81 p2 = p1; //structure copy
//					for(int i = 0; i < 81; i++) {
//						if(r1.isBitSet(i)) {
//							p2.chars[i] = 2;
//						}
//					}
//					r1 |= r0;
//					for(int c2 = 0; c2 < 5184; c2++) {
//						bm128 r2(colTemplates[r1row[2]][c2]);
//						if(!r2.isSubsetOf(r)) continue;
//						if(!r2.isDisjoint(r1)) continue;
//						ch81 p3 = p2; //structure copy
//						for(int i = 0; i < 81; i++) {
//							if(r2.isBitSet(i)) {
//								p3.chars[i] = 3;
//							}
//						}
//						r2 |= r1;
//						for(int c3 = 0; c3 < 5184; c3++) {
//							bm128 r3(colTemplates[r1row[3]][c3]);
//							if(!r3.isSubsetOf(r)) continue;
//							if(!r3.isDisjoint(r2)) continue;
//							ch81 p4 = p3; //structure copy
//							for(int i = 0; i < 81; i++) {
//								if(r3.isBitSet(i)) {
//									p4.chars[i] = 4;
//								}
//							}
//							r3 |= r2;
//							for(int c4 = 0; c4 < 5184; c4++) {
//								bm128 r4(colTemplates[r1row[4]][c4]);
//								if(!r4.isSubsetOf(r)) continue;
//								if(!r4.isDisjoint(r3)) continue;
//								ch81 p5 = p4; //structure copy
//								for(int i = 0; i < 81; i++) {
//									if(r4.isBitSet(i)) {
//										p5.chars[i] = 5;
//									}
//								}
//								ch81 can;
//								patminlex pml(p5.chars, can.chars);
//								res.insert(can);
//								if(first) return;
//							}
//						}
//					}
//				}
//			}
//			break;
//		default:
//			break;
//	}
//}
void templates::rookery2templates(const bm128& r, puzzleSet& res, bool first) const {
	int rsize = 0;
	int r1row[9];
	for(int i = 0; i < 9; i++) {
		if(r.isBitSet(i)) {
			r1row[rsize] = i;
			rsize++;
		}
	}
	switch(rsize) {
		case 2:
			for(int c0 = 0; c0 < 5184; c0++) {
				bm128 r0(colTemplates[r1row[0]][c0]);
				if(!r0.isSubsetOf(r)) continue;
				ch81 p1;
				p1.clear();
				for(int i = 0; i < 81; i++) {
					if(r0.isBitSet(i)) {
						p1.chars[i] = 1;
					}
				}
				bm128 rr0(r);
				rr0.clearBits(r0);
				ch81 p2 = p1; //structure copy
				for(int i = 0; i < 81; i++) {
					if(rr0.isBitSet(i)) {
						p2.chars[i] = 2;
					}
				}
				ch81 can;
				patminlex pml(p2.chars, can.chars);
				res.insert(can);
				if(first) return;
			}
			break;
		case 3:
			{
				bm128 rt[2][5184];
				int rtsize[2] = {0,0};
				for(int c = 0; c < 2; c++) {
					for(int i = 0; i < 5184; i++) {
						if(colTemplates[r1row[c]][i].isSubsetOf(r)) {
							rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
							rtsize[c]++;
						}
					}
				}
				//printf("%d\t%d\n", rtsize[0], rtsize[1]);
				for(int c0 = 0; c0 < rtsize[0]; c0++) {
					bm128 r0(rt[0][c0]);
					ch81 p1;
					p1.clear();
					for(int i = 0; i < 81; i++) {
						if(r0.isBitSet(i)) {
							p1.chars[i] = 1;
						}
					}
					bm128 rr0(r);
					rr0.clearBits(r0);
					for(int c1 = 0; c1 < rtsize[1]; c1++) {
						bm128 r1(rt[1][c1]);
						if(!r1.isSubsetOf(rr0)) continue;
						ch81 p2 = p1; //structure copy
						for(int i = 0; i < 81; i++) {
							if(r1.isBitSet(i)) {
								p2.chars[i] = 2;
							}
						}
						bm128 rr1(rr0);
						rr1.clearBits(r1);
						ch81 p3 = p2; //structure copy
						for(int i = 0; i < 81; i++) {
							if(rr1.isBitSet(i)) {
								p3.chars[i] = 3;
							}
						}
						ch81 can;
						patminlex pml(p3.chars, can.chars);
						res.insert(can);
						if(first) return;
					}
				}
			}
			break;
		case 4:
			{
				bm128 rt[3][5184];
				int rtsize[3] = {0,0,0};
				for(int c = 0; c < 3; c++) {
					for(int i = 0; i < 5184; i++) {
						if(colTemplates[r1row[c]][i].isSubsetOf(r)) {
							rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
							rtsize[c]++;
						}
					}
				}
				for(int c0 = 0; c0 < rtsize[0]; c0++) {
					bm128 r0(rt[0][c0]);
					ch81 p1;
					p1.clear();
					for(int i = 0; i < 81; i++) {
						if(r0.isBitSet(i)) {
							p1.chars[i] = 1;
						}
					}
					bm128 rr0(r);
					rr0.clearBits(r0);
					for(int c1 = 0; c1 < rtsize[1]; c1++) {
						bm128 r1(rt[1][c1]);
						if(!r1.isSubsetOf(rr0)) continue;
						ch81 p2 = p1; //structure copy
						for(int i = 0; i < 81; i++) {
							if(r1.isBitSet(i)) {
								p2.chars[i] = 2;
							}
						}
						bm128 rr1(rr0);
						rr1.clearBits(r1);
						for(int c2 = 0; c2 < rtsize[2]; c2++) {
							bm128 r2(rt[2][c2]);
							if(!r2.isSubsetOf(rr1)) continue;
							ch81 p3 = p2; //structure copy
							for(int i = 0; i < 81; i++) {
								if(r2.isBitSet(i)) {
									p3.chars[i] = 3;
								}
							}
							bm128 rr2(rr1);
							rr2.clearBits(r2);
							ch81 p4 = p3; //structure copy
							for(int i = 0; i < 81; i++) {
								if(rr2.isBitSet(i)) {
									p4.chars[i] = 4;
								}
							}
							ch81 can;
							patminlex pml(p4.chars, can.chars);
							res.insert(can);
							if(first) return;
						}
					}
				}
			}
			break;
		case 5:
			{
				//reduce the templates to play with, by removing those that aren't subsets of the given rookery
				bm128 rt[4][5184];
				int rtsize[4] = {0,0,0,0};
				for(int c = 0; c < 4; c++) {
					for(int i = 0; i < 5184; i++) {
						if(colTemplates[r1row[c]][i].isSubsetOf(r)) {
							rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
							rtsize[c]++;
						}
					}
				}
				//printf("%d\t%d\t%d\t%d\n", rtsize[0], rtsize[1], rtsize[2], rtsize[3]);
				for(int c0 = 0; c0 < 5184; c0++) {
					bm128 r0(colTemplates[r1row[0]][c0]);
					if(!r0.isSubsetOf(r)) continue;
					ch81 p1;
					p1.clear();
					for(int i = 0; i < 81; i++) {
						if(r0.isBitSet(i)) {
							p1.chars[i] = 1;
						}
					}
					bm128 rr0(r);
					rr0.clearBits(r0);
					int rtsize[4] = {0,0,0,0};
					for(int c = 1; c < 4; c++) {
						for(int i = 0; i < 5184; i++) {
							if(colTemplates[r1row[c]][i].isSubsetOf(rr0)) {
								rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
								rtsize[c]++;
							}
						}
					}
					for(int c1 = 0; c1 < rtsize[1]; c1++) {
						bm128 r1(rt[1][c1]);
						if(!r1.isSubsetOf(rr0)) continue;
						ch81 p2 = p1; //structure copy
						for(int i = 0; i < 81; i++) {
							if(r1.isBitSet(i)) {
								p2.chars[i] = 2;
							}
						}
						bm128 rr1(rr0);
						rr1.clearBits(r1);
						for(int c2 = 0; c2 < rtsize[2]; c2++) {
							bm128 r2(rt[2][c2]);
							if(!r2.isSubsetOf(rr1)) continue;
							ch81 p3 = p2; //structure copy
							for(int i = 0; i < 81; i++) {
								if(r2.isBitSet(i)) {
									p3.chars[i] = 3;
								}
							}
							bm128 rr2(rr1);
							rr2.clearBits(r2);
							for(int c3 = 0; c3 < rtsize[3]; c3++) {
								bm128 r3(rt[3][c3]);
								if(!r3.isSubsetOf(rr2)) continue;
								ch81 p4 = p3; //structure copy
								for(int i = 0; i < 81; i++) {
									if(r3.isBitSet(i)) {
										p4.chars[i] = 4;
									}
								}
								bm128 rr3(rr2);
								rr3.clearBits(r3);
								ch81 p5 = p4; //structure copy
								for(int i = 0; i < 81; i++) {
									if(rr3.isBitSet(i)) {
										p5.chars[i] = 5;
									}
								}
								ch81 can;
								patminlex pml(p5.chars, can.chars);
								res.insert(can);
								if(first) return;
							}
						}
					}
				}
			}
			break;
		case 6:
			{
				//reduce the templates to play with, by removing those that aren't subsets of the given rookery
				bm128 rt[5][5184];
				int rtsize[5] = {0,0,0,0,0};
				for(int c = 0; c < 5; c++) {
					for(int i = 0; i < 5184; i++) {
						if(colTemplates[r1row[c]][i].isSubsetOf(r)) {
							rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
							rtsize[c]++;
						}
					}
				}
				for(int c0 = 0; c0 < rtsize[0]; c0++) {
					bm128 r0(rt[0][c0]);

//				for(int c0 = 0; c0 < 5184; c0++) {
//					bm128 r0(colTemplates[r1row[0]][c0]);
//					if(!r0.isSubsetOf(r)) continue;

					ch81 p1;
					p1.clear();
					for(int i = 0; i < 81; i++) {
						if(r0.isBitSet(i)) {
							p1.chars[i] = 1;
						}
					}
					bm128 rr0(r);
					rr0.clearBits(r0);

					int rrtsize[4] = {0,0,0,0};
					bm128 rrt[4][5184];
					for(int c = 0; c < 4; c++) {
						for(int i = 0; i < rtsize[c + 1]; i++) {
							if(rt[c + 1][i].isSubsetOf(rr0)) {
								rrt[c][rrtsize[c]] = rt[c + 1][i];
								rrtsize[c]++;
							}
						}
					}

					for(int c1 = 0; c1 < rrtsize[0]; c1++) {
						bm128 r1(rrt[0][c1]);
						//if(!r1.isSubsetOf(rr0)) continue;
						ch81 p2 = p1; //structure copy
						for(int i = 0; i < 81; i++) {
							if(r1.isBitSet(i)) {
								p2.chars[i] = 2;
							}
						}
						bm128 rr1(rr0);
						rr1.clearBits(r1);
						for(int c2 = 0; c2 < rrtsize[1]; c2++) {
							bm128 r2(rrt[1][c2]);
							if(!r2.isSubsetOf(rr1)) continue;
							ch81 p3 = p2; //structure copy
							for(int i = 0; i < 81; i++) {
								if(r2.isBitSet(i)) {
									p3.chars[i] = 3;
								}
							}
							bm128 rr2(rr1);
							rr2.clearBits(r2);
							for(int c3 = 0; c3 < rrtsize[2]; c3++) {
								bm128 r3(rrt[2][c3]);
								if(!r3.isSubsetOf(rr2)) continue;
								ch81 p4 = p3; //structure copy
								for(int i = 0; i < 81; i++) {
									if(r3.isBitSet(i)) {
										p4.chars[i] = 4;
									}
								}
								bm128 rr3(rr2);
								rr3.clearBits(r3);
								for(int c4 = 0; c4 < rrtsize[3]; c4++) {
									bm128 r4(rrt[3][c4]);
									if(!r4.isSubsetOf(rr3)) continue;
									ch81 p5 = p4; //structure copy
									for(int i = 0; i < 81; i++) {
										if(r4.isBitSet(i)) {
											p5.chars[i] = 5;
										}
									}
									bm128 rr4(rr3);
									rr4.clearBits(r4);
									ch81 p6 = p5; //structure copy
									for(int i = 0; i < 81; i++) {
										if(rr4.isBitSet(i)) {
											p6.chars[i] = 6;
										}
									}
									ch81 can;
									patminlex pml(p6.chars, can.chars);
									res.insert(can);
									if(first) return;
								}
							}
						}
					}
				}
			}
			break;
		default:
			break;
	}
}
int templates::rookery2templates4(const bm128& r, exemplarSet& res) const {
	int count = 0;
	int rsize = 0;
	int r1row[9];
	for (int i = 0; i < 9; i++) {
		if (r.isBitSet(i)) {
			r1row[rsize] = i;
			rsize++;
		}
	}
	bm128 rt[3][5184];
	int rtsize[3] = { 0, 0, 0 };
	for (int c = 0; c < 3; c++) {
		for (int i = 0; i < 5184; i++) {
			if (colTemplates[r1row[c]][i].isSubsetOf(r)) {
				rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
				rtsize[c]++;
			}
		}
	}
	for (int c0 = 0; c0 < rtsize[0]; c0++) {
		bm128 r0(rt[0][c0]);
		ch81 p1;
		p1.clear();
		for (int i = 0; i < 81; i++) {
			if (r0.isBitSet(i)) {
				p1.chars[80 - i] = 1;
			}
		}
		bm128 rr0(r);
		rr0.clearBits(r0);
		for (int c1 = 0; c1 < rtsize[1]; c1++) {
			bm128 r1(rt[1][c1]);
			if (!r1.isSubsetOf(rr0))
				continue;
			ch81 p2 = p1; //structure copy
			for (int i = 0; i < 81; i++) {
				if (r1.isBitSet(i)) {
					p2.chars[80 - i] = 2;
				}
			}
			bm128 rr1(rr0);
			rr1.clearBits(r1);
			for (int c2 = 0; c2 < rtsize[2]; c2++) {
				bm128 r2(rt[2][c2]);
				if (!r2.isSubsetOf(rr1))
					continue;
				ch81 p3 = p2; //structure copy
				for (int i = 0; i < 81; i++) {
					if (r2.isBitSet(i)) {
						p3.chars[80 - i] = 3;
					}
				}
				bm128 rr2(rr1);
				rr2.clearBits(r2);
				//ch81 p4 = p3; //structure copy
				templateWithExemplar rr;
				rr.exemplar = p3;
				for (int i = 0; i < 81; i++) {
					if (rr2.isBitSet(i)) {
						rr.exemplar.chars[80 - i] = 4;
					}
				}
				patminlex pml(rr.exemplar.chars, rr.can.chars);
				res.insert(rr);
				count++;
			}
		}
	}
	return count;
}
int templates::rookery2templates5(const bm128& r, exemplarSet& res) const {
	int count = 0;
	int rsize = 0;
	int r1row[9];
	for (int i = 0; i < 9; i++) {
		if (r.isBitSet(i)) {
			r1row[rsize] = i;
			rsize++;
		}
	}
	//reduce the templates to play with, by removing those that aren't subsets of the given rookery
	bm128 rt[4][5184];
	int rtsize[4] = { 0, 0, 0, 0 };
	for (int c = 0; c < 4; c++) {
		for (int i = 0; i < 5184; i++) {
			if (colTemplates[r1row[c]][i].isSubsetOf(r)) {
				rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
				rtsize[c]++;
			}
		}
	}
	//printf("%d\t%d\t%d\t%d\n", rtsize[0], rtsize[1], rtsize[2], rtsize[3]);
	for (int c0 = 0; c0 < 5184; c0++) {
		bm128 r0(colTemplates[r1row[0]][c0]);
		if (!r0.isSubsetOf(r))
			continue;
		ch81 p1;
		p1.clear();
		for (int i = 0; i < 81; i++) {
			if (r0.isBitSet(i)) {
				p1.chars[80 - i] = 5;
			}
		}
		bm128 rr0(r);
		rr0.clearBits(r0);
		int rtsize[4] = { 0, 0, 0, 0 };
		for (int c = 1; c < 4; c++) {
			for (int i = 0; i < 5184; i++) {
				if (colTemplates[r1row[c]][i].isSubsetOf(rr0)) {
					rt[c][rtsize[c]] = colTemplates[r1row[c]][i];
					rtsize[c]++;
				}
			}
		}
		for (int c1 = 0; c1 < rtsize[1]; c1++) {
			bm128 r1(rt[1][c1]);
			if (!r1.isSubsetOf(rr0))
				continue;
			ch81 p2 = p1; //structure copy
			for (int i = 0; i < 81; i++) {
				if (r1.isBitSet(i)) {
					p2.chars[80 - i] = 6;
				}
			}
			bm128 rr1(rr0);
			rr1.clearBits(r1);
			for (int c2 = 0; c2 < rtsize[2]; c2++) {
				bm128 r2(rt[2][c2]);
				if (!r2.isSubsetOf(rr1))
					continue;
				ch81 p3 = p2; //structure copy
				for (int i = 0; i < 81; i++) {
					if (r2.isBitSet(i)) {
						p3.chars[80 - i] = 7;
					}
				}
				bm128 rr2(rr1);
				rr2.clearBits(r2);
				for (int c3 = 0; c3 < rtsize[3]; c3++) {
					bm128 r3(rt[3][c3]);
					if (!r3.isSubsetOf(rr2))
						continue;
					ch81 p4 = p3; //structure copy
					for (int i = 0; i < 81; i++) {
						if (r3.isBitSet(i)) {
							p4.chars[80 - i] = 8;
						}
					}
					bm128 rr3(rr2);
					rr3.clearBits(r3);
					//ch81 p5 = p4; //structure copy
					templateWithExemplar rr;
					rr.exemplar = p4;
					for (int i = 0; i < 81; i++) {
						if (rr3.isBitSet(i)) {
							rr.exemplar.chars[80 - i] = 9;
						}
					}
					patminlex pml(rr.exemplar.chars, rr.can.chars);
					res.insert(rr);
					count++;
				}
			}
		}
	}
	return count;
}
//void templates::templates2rookeries() const { //read k-templates from stdin and write (un)sorted k-rookeries to stdout
//	puzzleSet r3tall;
//	char buf[2000];
//	while(fgets(buf, sizeof(buf), stdin)) {
//		ch81 puz;
//		//int puzSize = puz.fromString(buf);
//		puz.fromString(buf);
//		ch81 p1 = puz; //structure copy
//		for(int i = 0; i < 81; i++) {
//			if(p1.chars[i]) {
//				p1.chars[i] = 1;
//			}
//		}
//		ch81 pcan;
//		subcanon(p1.chars, pcan.chars);
//		if(r3tall.find(pcan) == r3tall.end()) {
//			r3tall.insert(pcan);
//			pcan.toString(p1.chars);
//			printf("%81.81s %81.81s\n", p1.chars, buf);
//		}
//	}
//	//r3tall.saveToFile(stdout);
//}
void templates::get3templates(puzzleSet& t3all) const { //3.2 seconds
	puzzleSet t2all;
	get2templates(t2all);
	templatesPlus1(t2all, t3all);
}
void templates::get3rookeries(lightweightUsetList& r3all) const { //3.629" templates+1 / 2.741" rookery+1
	lightweightUsetList r2all;
	get2rookeries(r2all);
	rookeriesPlus1(r2all, r3all);
}
void templates::get4rookeries(lightweightUsetList& r4all) const { //498.676 seconds
	lightweightUsetList r3all;
	get3rookeries(r3all);

//	bm128 u = *r3all.begin();
//	r3all.clear();
//	r3all.insert(u);

	rookeriesPlus1(r3all, r4all);
}

void templates::get2templates(puzzleSet& t2all) const { //0.1 seconds
	//fix template in r1c1 to first one
	//prepare a pseudo-puzzle
	ch81 r1p;
	r1p.clear();
	for(int i = 0; i < 81; i++) {
		if(colTemplates[0][0].isBitSet(i)) {
			r1p.chars[i] = 1;
		}
	}
	//find all compatible 2-templates
	for(int r2 = 0 + 1; r2 < 9; r2++) {
		for(int r2tn = 0; r2tn < 5184; r2tn++) {
			if(colTemplates[0][0].isDisjoint(colTemplates[r2][r2tn])) {
				//a valid 2-template found
				ch81 r2p = r1p; //structure copy
				for(int i = 0; i < 81; i++) {
					if(colTemplates[r2][r2tn].isBitSet(i)) {
						r2p.chars[i] = 2;
					}
				}
				ch81 can;
				patminlex ml(r2p.chars, can.chars);
				t2all.insert(can);
			}
		}
	}
}
void templates::get2rookeries(lightweightUsetList& r2all) const {
	puzzleSet t2all;
	get2templates(t2all);
	for(puzzleSet::const_iterator p = t2all.begin(); p != t2all.end(); p++) {
		bm128 r;
		template2rookery(*p, r);
		r2all.insert(r);
	}
}

void templates::count6templates() const {
	puzzleSet p3;
	lightweightUsetList r3all;
	get3rookeries(r3all);
	fprintf(stderr, "Number of 3-rookeries\t%d\n", (int)r3all.size());
	unsigned long long numT3 = 0;
	unsigned long long numT6 = 0;
#ifdef _OPENMP
#pragma omp parallel
#endif //_OPENMP
	{
	for(lightweightUsetList::const_iterator r3 = r3all.begin(); r3 != r3all.end(); r3++) {
#ifdef _OPENMP
#pragma omp single nowait
#endif //_OPENMP
		{
		puzzleSet t3;
		rookery2templates(*r3, t3);
		bm128 r6 = maskLSB[81];
		r6.clearBits(*r3);
		puzzleSet t6;
		rookery2templates(r6, t6);
		ch81 txt;
		r3->toMask81(txt.chars);
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
		{
			printf("%81.81s\t%d\t%d\n", txt.chars, (int)t3.size(), (int)t6.size());
			numT3 += t3.size();
			numT6 += t6.size();
			fflush(NULL);
		}
//		ch81 txt;
//		r3->toMask81(txt.chars);
//		printf("%81.81s\n", txt.chars);
//		for(puzzleSet::const_iterator p3 = t3.begin(); p3 != t3.end(); p3++) {
//
//		}
		}
	}
	}
	fprintf(stderr, "Total number of 3-templates\t%llu\n", numT3);
	fprintf(stderr, "Total number of 6-templates\t%llu\n", numT6);
}

void templates::count5templates() const {
	puzzleSet p3;
	lightweightUsetList r4all;
	get4rookeries(r4all);
	fprintf(stderr, "Number of 4-rookeries\t%d\n", (int)r4all.size());
	unsigned long long numT4 = 0;
	unsigned long long numT5 = 0;
#ifdef _OPENMP
#pragma omp parallel
#endif //_OPENMP
	{
	for(lightweightUsetList::const_iterator r4 = r4all.begin(); r4 != r4all.end(); r4++) {
#ifdef _OPENMP
#pragma omp single nowait
#endif //_OPENMP
		{
		puzzleSet t4;
		rookery2templates(*r4, t4);
		bm128 r5 = maskLSB[81];
		r5.clearBits(*r4);
		puzzleSet t5;
		rookery2templates(r5, t5);
		ch81 txt;
		r4->toMask81(txt.chars);
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
		{
			printf("%81.81s\t%d\t%d\n", txt.chars, (int)t4.size(), (int)t5.size());
			numT4 += t4.size();
			numT5 += t5.size();
			fflush(NULL);
		}
//		ch81 txt;
//		r3->toMask81(txt.chars);
//		printf("%81.81s\n", txt.chars);
//		for(puzzleSet::const_iterator p3 = t3.begin(); p3 != t3.end(); p3++) {
//
//		}
		}
	}
	}
	fprintf(stderr, "Total number of 4-templates\t%llu\n", numT4);
	fprintf(stderr, "Total number of 5-templates\t%llu\n", numT5);
}

void templates::get999911110() const {
	puzzleSet p3;
	lightweightUsetList r4all;
	get4rookeries(r4all);
	fprintf(stderr, "Number of 4-rookeries\t%d\n", (int)r4all.size());
	for(lightweightUsetList::const_iterator r4 = r4all.begin(); r4 != r4all.end(); r4++) {
		bm128 r5 = maskLSB[81];
		r5.clearBits(*r4);
		puzzleSet t5;
		rookery2templates(r5, t5);
		for(puzzleSet::const_iterator t = t5.begin(); t != t5.end(); t++) {
			ch81 txt;
			t->toString(txt.chars);
			printf("%81.81s\n", txt.chars);
		}
		//fflush(NULL);
	}
}

void templates::countGrids() const {
	//ch81 r9 = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	puzzleSet p3;
	lightweightUsetList r4all;

	//get4rookeries(r4all);
	char buf[1000];
	while(fgets(buf, sizeof(buf), stdin)) {
		bm128 r4;
		ch81 p4;
		p4.fromString(buf);
		template2rookery(p4, r4);
		r4all.insert(r4);
		//if(0 == memcmp(buf, ".....1111.11.1...11.111.......11.11.11...1.1.11.1..1....11.1..1.11.11...1.....111", 81)) break; //bug in eclipse ide
		//break;
	}

	//fprintf(stderr, "Number of 4-rookeries\t%d\n", (int)r4all.size());
	unsigned long long numGridsTotal = 0;
#ifdef _OPENMP
#pragma omp parallel
#endif //_OPENMP
	{
		for (lightweightUsetList::const_iterator r4 = r4all.begin(); r4 != r4all.end(); r4++) {
#ifdef _OPENMP
#pragma omp single nowait
#endif //_OPENMP
			{
				int foundLess = 0;
				int foundGrearter = 0;
				int foundEqual = 0;
				bool t4generated = false;
				bm128 r5 = maskLSB[81];
				r5.clearBits(*r4);
				exemplarSet t5;
				rookery2templates5(r5, t5);

				unsigned long long numGridsForRookery = 0;
				ch81 r4can; //canonical textual representation of the rookery
				for (int i = 0; i < 81; i++) {
					if (r4->isBitSet(80 - i)) {
						r4can.chars[i] = 1;
					}
					else {
						r4can.chars[i] = 0;
					}
				}
				exemplarSet t4;
				int initialNumT5 = t5.size();
				for (exemplarSet::const_iterator p5 = t5.begin(); p5 != t5.end(); p5++) {
					//find whether any independent of t4 sub-rookery has canonical form < r4
					bool hasEqualRookery = false;
					templateWithExemplar min4of5;
					for (int v = 1; v < 6; v++) { //for each 5 choose 4, not depending of p4
						ch81 rr4of5 = p5->can;
						for (int c = 0; c < 81; c++) {
							if (p5->can.chars[c] == v) {
								rr4of5.chars[c] = 0;
							}
						}
						ch81 rr4of5can;
						patminlex::res res;
						{
							patminlex pml(rr4of5.chars, rr4of5can.chars, patminlex::action::findMinPatternLE, &res, r4can.chars);
						}

						if(res == patminlex::res::resGT) {
							//this pair will be exported later by a larger r4. Skip it.

//							//but firstly debug it
//							ch81 txt;
//							for(int i = 0; i < 81; i++) {
//								if(r4can.chars[i])
//									txt.chars[i] = '0' + r4can.chars[i];
//								else
//									txt.chars[i] = '.';
//							}
//							printf("\n%81.81s\trookery 4\n", txt.chars);
//							for(int i = 0; i < 81; i++) {
//								if(rr4of5.chars[i])
//									txt.chars[i] = '0' + rr4of5.chars[i];
//								else
//									txt.chars[i] = '.';
//							}
//							printf("%81.81s\tt 4 of 5\n", txt.chars);
//							{
//								patminlex pml(rr4of5.chars, rr4of5can.chars);
//							}
//							for(int i = 0; i < 81; i++) {
//								if(rr4of5can.chars[i])
//									txt.chars[i] = '0' + rr4of5can.chars[i];
//								else
//									txt.chars[i] = '.';
//							}
//							printf("%81.81s\tt 4 of 5 can\n", txt.chars);
//							ch81 rrrr;
//							for(int i = 0; i < 81; i++) {
//								if(rr4of5.chars[i])
//									rrrr.chars[i] = 1;
//								else
//									rrrr.chars[i] = 0;
//							}
//							{
//								patminlex pml(rrrr.chars, rr4of5can.chars);
//							}
//							for(int i = 0; i < 81; i++) {
//								if(rr4of5can.chars[i])
//									txt.chars[i] = '0' + rr4of5can.chars[i];
//								else
//									txt.chars[i] = '.';
//							}
//							printf("%81.81s\tr 4 of 5 can\n", txt.chars);
//
//							// end debug

							foundGrearter++;
							goto nextp5;
						}
						if(res == patminlex::res::resEQ) {
							//this pair has to be checked at p4 level
							foundEqual++;
							goto nextp5;
						}
//						//if (rr4of5can < r4can) {
//						if(res == patminlex::res::resLT) {
//							//this pair will be exported by a smaller r4. Skip it.
//							foundLess++;
//							goto nextp5;
//						}
//						if(res == patminlex::res::resEQ) {
//						//if (rr4of5can == r4can) {
//							foundEqual++;
////							//this pair is sufficient to be checked to a smaller subset of t4, up to the canonical representation of the rr4of5can's template.
////							patminlex pml(rr4of5.chars, rr4of5can.chars);
////							if(hasEqualRookery) {
////								if(rr4of5can < min4of5.can) {
////									min4of5.can = rr4of5can; //copy
////								}
////							}
////							else {
////								min4of5.can = rr4of5can; //copy
////								hasEqualRookery = true;
////							}
//						}
					} //clear value 1..5
//					//now check against each t4
//					if(! t4generated) {
//						//at this point we need t4
//						rookery2templates4(*r4, t4);
//						t4generated = true;
//					}
//					{
//						exemplarSet::const_iterator t4end = hasEqualRookery ? t4.upper_bound(min4of5) : t4.end();
//						for (exemplarSet::const_iterator p4 = t4.begin(); p4 != t4end; p4++) {
//						//for (exemplarSet::const_iterator p4 = t4.begin(); p4 != t4.end(); p4++) {
//							ch81 res = p5->exemplar;
//							for (int c = 0; c < 81; c++) {
//								res.chars[c] += p4->exemplar.chars[c];
//							}
//							//res will be exported if the following tests are passed
//							for (int i4of9 = 1; i4of9 < 121; i4of9++) { //for each 9 choose 4, except first that depend only of p4 and latest 5 that depend only of p5
//							//for (int i4of9 = 1; i4of9 < 126; i4of9++) { //for each 9 choose 4, except first that depend only of p4
//								int v1 = choice4of9[i4of9][0] + 1;
//								int v2 = choice4of9[i4of9][1] + 1;
//								int v3 = choice4of9[i4of9][2] + 1;
//								int v4 = choice4of9[i4of9][3] + 1;
//								ch81 ss4of9 = res;
////								for (int c = 0; c < 81; c++) {
////									if (ss4of9.chars[c] != v1 && ss4of9.chars[c] != v2 && ss4of9.chars[c] != v3 && ss4of9.chars[c] != v4) {
////										ss4of9.chars[c] = 0;
////									}
////								}
//								for (int c = 0; c < 81; c++) {
//									if (ss4of9.chars[c] == v1) continue;
//									if (ss4of9.chars[c] == v2) continue;
//									if (ss4of9.chars[c] == v3) continue;
//									if (ss4of9.chars[c] == v4) continue;
//									ss4of9.chars[c] = 0;
//								}
//								ch81 rr4of9can;
//								patminlex::res pr;
//								{
//									//patminlex pml(ss4of9.chars, rr4of9can.chars, patminlex::action::findMinPattern);
//									patminlex pml(ss4of9.chars, rr4of9can.chars, patminlex::action::findMinPatternLE, &pr, r4can.chars);
//								}
//								//if (rr4of9can < r4can) {
//								if(pr == patminlex::res::resLT) {
//									//this pair will be exported by a smaller r4. Skip it.
//									goto nextp4;
//								}
//								//if (rr4of9can == r4can) {
//								if(pr == patminlex::res::resEQ) {
//									ch81 ss4of9can;
//									patminlex pml(ss4of9.chars, ss4of9can.chars); //maybe it is worth to search only for existence of canonical representation smaller than p4->can
//									if (ss4of9can < p4->can) {
//										//the essentially same union p4+p5 was exported on earlier iteration of p4. Skip it.
//										goto nextp4;
//									}
//								}
//							} //each 4 of 9
//							//all subsets of p5 are >= p4. Export the (p4+p5) grid.
//							numGridsForRookery++;
//							nextp4: ;
//						} //for p4
//					}
					nextp5: ;
				} //for p5
				//nextr4:
				for(int i = 0; i < 81; i++) {
					r4can.chars[i] = r4can.chars[i] ? r4can.chars[i] + '0' : '.';
				}
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
				{
					numGridsTotal += numGridsForRookery;
					//r4, t4, t5, t5*t4, reduced t4*t5
					//printf("%81.81s\t%d\t%d\t%llu\t%llu\n", r4can.chars, (int) t4.size(), initialNumT5, (unsigned long long) (initialNumT5 * t4.size()), numGridsForRookery); //attempts 1..8
					//printf("%81.81s\t%d\t%d\t%d\t%d\n", r4can.chars, initialNumT5, foundLess, foundEqual, initialNumT5 - foundLess - foundEqual); //attempt9
					printf("%81.81s\t%d\t%d\t%d\t%d\n", r4can.chars, initialNumT5, foundGrearter, foundEqual, initialNumT5 - foundGrearter - foundEqual); //attempt10
					//fflush(NULL);
				} //omp critical
			} //omp single nowait
		} //for r4
	} //omp parallel
	fprintf(stderr, "Total number of grids\t%llu\n", numGridsTotal);
}

void templates::generateXX110() const { //this way it doesn't work!
	puzzleSet t2all; //all 2-templates in canonical form
	puzzleSet t2entwined; //2-templates in canonical form that resolvable by puzzle with 999999910 distribution
	get2templates(t2all);
	typedef map <unsigned short, set <unsigned short> > tIndexes;
	tIndexes t2Indexes;
	for(puzzleSet::const_iterator t2 = t2all.begin(); t2 != t2all.end(); t2++) {
		ch81 sol;
		ch81 puz;
		solve(t2->chars, 1, sol.chars);
		for(int i = 0; i < 81; i++) {
			if(sol.chars[i] <= 2) {
				if(sol.chars[i] == 1 && i < 9) {
					puz.chars[i] = 1; //leave a single given from the 2-template
				}
				else {
					puz.chars[i] = 0;
				}
			}
			else {
				puz.chars[i] = sol.chars[i];
			}
		}
		if(1 == solve(puz.chars, 2)) {
			t2entwined.insert(*t2); //105 2-templates
			t2->toString(puz.chars);
			unsigned short indexes[10];
			int n = getIndexes(*t2, indexes);
			t2Indexes[indexes[0]].insert(indexes[1]);
			t2Indexes[indexes[1]].insert(indexes[0]);
			//printf("%81.81s\t%d\t%d\t%d\n", puz.chars, n, indexes[0], indexes[1]);
		}
	}
	printf("all t2=%d\tfep=%d\tdistinct=%d\n", (int)t2all.size(), (int)t2entwined.size(), (int)t2Indexes.size());
//	for(tIndexes::const_iterator ii = t2Indexes.begin(); ii != t2Indexes.end(); ii++) {
//		printf("%d\t%d\n", ii->first, (int)ii->second.size());
//	}
	std::vector<std::vector<bool>> isFEP(46656, std::vector<bool>(46656,false));
	for(int i = 0; i < 264072960; i++) {
		//store fe pairs (a,b)
		unsigned long a, b;
		fscanf(stdin, "%lu\t%lu", &a, &b);
		isFEP[a][b] = true;
		isFEP[b][a] = true;
	}
	puzzleSet t3entwined;
	for(tIndexes::const_iterator a2 = t2Indexes.begin(); a2 != t2Indexes.end(); a2++) {
		for(int i = 0; i < 46656; i++) {
			if(isFEP[a2->first][i]) {
				for(set <unsigned short>::const_iterator b2 = a2->second.begin(); b2 != a2->second.end(); b2++) {
					if(isFEP[*b2][i]) {
						//(a2->first, *b2, i) is a fully entwined triplet
						bm128 tt1(allTemplates[a2->first]);
						bm128 tt2(allTemplates[*b2]);
						bm128 tt3(allTemplates[i]);
						ch81 ttt;
						ttt.clear();
						for(int c = 0; c < 81; c++) {
							bm128 mask = bitSet[c];
							if(mask.isSubsetOf(tt1)) ttt.chars[c] = 1;
							else if (mask.isSubsetOf(tt2)) ttt.chars[c] = 2;
							else if (mask.isSubsetOf(tt3)) ttt.chars[c] = 3;
						}
						ch81 can;
						patminlex rml(ttt.chars, can.chars);
						if(t3entwined.insert(can).second) {

							//debug
							tt1 |= tt2;
							tt1 |= tt3;
							ch81 sol;
							if(tt1.popcount_128() != 27) printf("@");
							if(solve(can.chars, 1, sol.chars) != 1) printf("!");
							ch81 inverse(sol);
							for(int c = 0; c < 81; c++) {
								if(can.chars[c]) inverse.chars[c] = 0;
							}
							int n;
							char ss[40][81];
							if((n = solve(inverse.chars, 40, ss[0])) != 6) {
								can.toString(ttt.chars);
								printf("\n%81.81s\n", ttt.chars);
								inverse.toString(ttt.chars);
								printf("%81.81s\t%d\n", ttt.chars, n);
								for(int s = 0; s < n; s++) {
									ch81::toString(ss[s], ttt.chars);
									printf("%81.81s\t(%d)\n", ttt.chars, s);
								}
							}
						}
					}
				}
			}
		}
	}
	printf("\nt3=%d\n", (int)t3entwined.size());
//	printf("map size=%d\n", (int)fepMap.size());
	//at this point we have a canonicalized list t2entwined of all fully entwined pairs, i.e. solvable by a single clue.
	//adding a third template, if it assembles (1,3) and (2,3) canonicalized 2-templates from t2entwined, then it is solvable by 2 clues
	//bool fep[46656][46656];
//	for(int i = 0; i < 46656 - 1; i++) {
//		ch81 p1;
//		for(int c = 0; c < 81; c++) {
//			p1.chars[c] = allTemplates[i].isBitSet(c) ? 1 : 0;
//		}
//		for(int j = i + 1; j < 46656; j++) {
//			//fep[i][j] = false;
//			if(allTemplates[i].isDisjoint(allTemplates[j])) {
//				ch81 p2 = p1;
//				ch81 can;
//				for(int c = 0; c < 81; c++) {
//					if(allTemplates[j].isBitSet(c)) p2.chars[c] = 2;
//				}
//				patminlex rml(p2.chars, can.chars);
//				if(t2entwined.find(can) != t2entwined.end()) {
//					//fep[i][j] = true;
//					printf("%d\t%d\n", i, j);
//				}
//			}
//		}
//	}

//	for(puzzleSet::const_iterator t2 = t2entwined.begin(); t2 != t2all.end(); t2++) {
//		ch81 puzA;
//		ch81 puzB;
//		bm128 bmA;
//		bm128 bmB;
//		bmA.clear();
//		bmB.clear();
//		for(int i = 0; i < 81; i++) {
//			switch(t2->chars[i]) {
//				case 1:
//					puzA.chars[i] = 1;
//					bmA.setBit(i);
//					puzB.chars[i] = 0;
//					break;
//				case 2:
//					puzA.chars[i] = 0;
//					puzB.chars[i] = 2;
//					bmB.setBit(i);
//					break;
//				default:
//					puzA.chars[i] = 0;
//					puzB.chars[i] = 0;
//			}
//		}

//		for(int col = 0; col < 9; col++) {
//			for(int tn = 0; tn < 5184; tn++) {
//				if(bmA.isDisjoint(colTemplates[col][tn])) {
//					ch81 p = src; //structure copy
//					for(int i = 0; i < 81; i++) {
//						if(colTemplates[col][tn].isBitSet(i)) {
//							p.chars[i] = 9;
//						}
//					}
//					ch81 can;
//					patminlex rml(p.chars, can.chars);
//					res.insert(can);
//				}
//			}
//		}
//	}
}

void templates::generateGrids() const {
	unsigned long long numGrids = 0;
	for(int ti0 = 0; ti0 < 1; ti0++) {
		bm128 t0 = colTemplates[0][ti0];
		//extract the compatible to t0 templates for columns 1..8. "Compatible" == disjoint and with >= index
		int colTemplates0[8][2396]; //reduced cache of templates compatible indexes, for the inner iterations
		int ctSizes0[8] = {0,0,0,0,0,0,0,0}; //2097,2097,2097,2396,2396,2097,2396,2396
		for(int ti1 = ti0; ti1 < 5184; ti1++) {
			for(int col = 0; col < 8; col++) {
				if(t0.isDisjoint(colTemplates[1 + col][ti1])) {
					colTemplates0[col][ctSizes0[col]] = ti1;
					ctSizes0[col]++;
				}
			}
		}
		//printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", ctSizes0[0], ctSizes0[1], ctSizes0[2], ctSizes0[3], ctSizes0[4], ctSizes0[5], ctSizes0[6], ctSizes0[7]);
		//iterate column 1 compatible templates
		for(int i1 = 0; i1 < ctSizes0[0]; i1++) { //index within cache
			int ti1 = colTemplates0[0][i1]; //global index
			bm128 t1 = colTemplates[1][ti1]; //the template bitmap
			//extract the compatible to t1 templates for columns 2..8.
			int colTemplates1[7][2396]; //reduced cache for columns 2..8
			int ctSizes1[7] = {0,0,0,0,0,0,0};
			for(int col = 0; col < 7; col++) {
				for(int i = 0; i < ctSizes0[1 + col]; i++) {
					int ti = colTemplates0[1 + col][i];
					//if(ti < ti1) continue; //once a+b is done, skip doing b+a
					if(t1.isDisjoint(colTemplates[2 + col][ti])) {
						colTemplates1[col][ctSizes1[col]] = ti;
						ctSizes1[col]++;
					}
				}
			}
			//iterate column 2 compatible templates
			for(int i2 = 0; i2 < ctSizes1[0]; i2++) { //index within cache
				int ti2 = colTemplates1[0][i2]; //global index
				bm128 t2 = colTemplates[2][ti2]; //the template bitmap
				//extract the compatible to t2 templates for columns 3..8.
				int colTemplates2[6][2396]; //todo: find the max and (statically) reduce size to it
				int ctSizes2[6] = {0,0,0,0,0,0};
				for(int col = 0; col < 6; col++) {
					for(int i = 0; i < ctSizes1[1 + col]; i++) {
						int ti = colTemplates1[1 + col][i];
						//if(ti < ti2) continue; //once a+b is done, skip doing b+a
						if(t2.isDisjoint(colTemplates[3 + col][ti])) {
							colTemplates2[col][ctSizes2[col]] = ti;
							ctSizes2[col]++;
						}
					}
				}
				//iterate column 3 compatible templates
				for(int i3 = 0; i3 < ctSizes2[0]; i3++) { //index within cache
					int ti3 = colTemplates2[0][i3]; //global index
					bm128 t3 = colTemplates[3][ti3]; //the template bitmap
					//extract the compatible to t3 templates for columns 4..8.
					int colTemplates3[5][2396]; //todo: find the max and (statically) reduce size to it
					int ctSizes3[5] = {0,0,0,0,0};
					for(int col = 0; col < 5; col++) {
						for(int i = 0; i < ctSizes2[1 + col]; i++) {
							int ti = colTemplates2[1 + col][i];
							//if(ti < ti3) continue; //once a+b is done, skip doing b+a
							if(t3.isDisjoint(colTemplates[4 + col][ti])) {
								colTemplates3[col][ctSizes3[col]] = ti;
								ctSizes3[col]++;
							}
						}
					}
					//iterate column 4 compatible templates
					for(int i4 = 0; i4 < ctSizes3[0]; i4++) { //index within cache
						int ti4 = colTemplates3[0][i4]; //global index
						bm128 t4 = colTemplates[4][ti4]; //the template bitmap
						//extract the compatible to t4 templates for columns 5..8.
						int colTemplates4[4][2396]; //todo: find the max and (statically) reduce size to it
						int ctSizes4[4] = {0,0,0,0};
						for(int col = 0; col < 4; col++) {
							for(int i = 0; i < ctSizes3[1 + col]; i++) {
								int ti = colTemplates3[1 + col][i];
								//if(ti < ti4) continue; //once a+b is done, skip doing b+a
								if(t4.isDisjoint(colTemplates[5 + col][ti])) {
									colTemplates4[col][ctSizes4[col]] = ti;
									ctSizes4[col]++;
								}
							}
						}
						//iterate column 5 compatible templates
						for(int i5 = 0; i5 < ctSizes4[0]; i5++) { //index within cache
							int ti5 = colTemplates4[0][i5]; //global index
							bm128 t5 = colTemplates[5][ti5]; //the template bitmap
							//extract the compatible to t5 templates for columns 6..8.
							int colTemplates5[3][2396]; //todo: find the max and (statically) reduce size to it
							int ctSizes5[3] = {0,0,0};
							for(int col = 0; col < 3; col++) {
								for(int i = 0; i < ctSizes4[1 + col]; i++) {
									int ti = colTemplates4[1 + col][i];
									//if(ti < ti5) continue; //once a+b is done, skip doing b+a
									if(t5.isDisjoint(colTemplates[6 + col][ti])) {
										colTemplates5[col][ctSizes5[col]] = ti;
										ctSizes5[col]++;
									}
								}
							}
							//iterate column 6 compatible templates
							for(int i6 = 0; i6 < ctSizes5[0]; i6++) { //index within cache
								int ti6 = colTemplates5[0][i6]; //global index
								bm128 t6 = colTemplates[6][ti6]; //the template bitmap
								//extract the compatible to t6 templates for columns 7..8.
								int colTemplates6[2][2396]; //todo: find the max and (statically) reduce size to it
								int ctSizes6[2] = {0,0};
								for(int col = 0; col < 2; col++) {
									for(int i = 0; i < ctSizes5[1 + col]; i++) {
										int ti = colTemplates5[1 + col][i];
										//if(ti < ti6) continue; //once a+b is done, skip doing b+a
										if(t6.isDisjoint(colTemplates[7 + col][ti])) {
											colTemplates6[col][ctSizes6[col]] = ti;
											ctSizes6[col]++;
										}
									}
								}
								//iterate column 7 compatible templates
								for(int i7 = 0; i7 < ctSizes6[0]; i7++) { //index within cache
									int ti7 = colTemplates6[0][i7]; //global index
									bm128 t7 = colTemplates[7][ti7]; //the template bitmap
									//iterate column 8 compatible templates, there should be 0 or 1
									for(int i8 = 0; i8 < ctSizes6[1]; i8++) {
										int ti8 = colTemplates6[1][i8];
										//if(ti8 < ti7) continue; //once a+b is done, skip doing b+a
										if(t7.isDisjoint(colTemplates[8][ti8])) {
											//we have a grid for exporting
											bm128 t8 = colTemplates[8][ti8]; //the template bitmap

											numGrids++;
											//printf(".");
										}
									}
								}
							}
						}
					}
				}
			}
			printf("%d\t%llu\n", i1, numGrids);
		}
	}
}
//void templates::get2rookeries() {
//	puzzleSet r2all;
//	//fix template in r1c1 to first one
//	//prepare a pseudo=puzzle
//	ch81 r1p;
//	r1p.clear();
//	for(int i = 0; i < 81; i++) {
//		if(colTemplates[0][0].isBitSet(i)) {
//			r1p.chars[i] = 1;
//		}
//	}
//	//find all compatible 2-templates
//	int n = 0;
//	for(int r2 = 0 + 1; r2 < 9; r2++) {
//		for(int r2tn = 0; r2tn < 5184; r2tn++) {
//			if(colTemplates[0][0].isDisjoint(colTemplates[r2][r2tn])) {
//				//a valid 2-template found
//				ch81 r2p = r1p; //structure copy
//				for(int i = 0; i < 81; i++) {
//					if(colTemplates[r2][r2tn].isBitSet(i)) {
//						r2p.chars[i] = 2;
//					}
//				}
//				ch81 can, canRookery;
//				subcanon(r2p.chars, can.chars);
//				ch81 canMask = can; //structure copy
//				for(int i = 0; i < 81; i++) {
//					if(canMask.chars[i])
//						canMask.chars[i] = 1;
//				}
//				subcanon(canMask.chars, canRookery.chars);
//				if(r2all.find(canRookery) == r2all.end()) {
//					r2all.insert(canRookery);
//					canRookery.toString(canMask.chars);
//					can.toString(canRookery.chars);
//					printf("%81.81s\t%81.81s\n", canMask.chars, canRookery.chars);
//				}
//			}
//		}
//	}
//	//printf("%d\n", r2all.size());
//	//r2all.saveToFile(stdout);
//}
unsigned long long templates::nSol(const char *p) {
	//~80 puzzles/second for Gordon Royle's list of 17s (500/s initialization only), 400/s w/o guessing
	bm128 clues[9]; //positions of the givens for each digit
	bm128 invalid[9]; //forbidden positions for each digit

//Take a puzzle. Example from Patterns Game.
//... ..1 ..2
//..3 .4. .5.
//.6. 7. .8..
//
//..6 ... .7.
//.4. .2. ..3
//1.. ..4 9..
//
//..8 ..9 5..
//.7. 8.. .6.
//6.. .5. ... ER=6.6/6.6/6.6 - joel64, patterns game 146
//
//Step A. Prepare 9 81-bit masks, one per digit. Example for digit 7.
//.*. *** .*2
//.*3 *** .*.
//*** 7** ***
//
//*** *** *7*
//.*. *2. ***
//1*. *.4 ***
//
//*** *.9 5*.
//*7* *** ***
//*** *5. .*. Digit 7 step A1: set all visible cells
//
//
//.*. *** .**
//.** *** .*.
//*** 7** ***
//
//*** *** *7*
//.*. **. ***
//**. *.* ***
//
//*** *.* **.
//*7* *** ***
//*** **. .*. Digit 7 step A2: set all givens <> 7
//
//
//.*. *** .**
//.** *** .*.
//*** .** ***
//
//*** *** *.*
//.*. **. ***
//**. *.* ***
//
//*** *.* **.
//*.* *** ***
//*** **. .*. Digit 7 mask: only 6 from the 46656 possible templates are disjoint to this mask
//
//Step B. Apply the 9 masks over all 46656 templates. Store all disjoint to the mask templates as candidates for the respective digit.
//
//Template distribution 36 28 54 28 14  4  6 14 42 	product=301112598528 sum=226
//
//Step C. Perform some basic eliminations (none of them is required for this example puzzle).
//
//Step D. Remove all templates that have no at least one disjoint template for each of the other digits.
//Repeat until solution found or no more eliminations exist.
//
//Template distribution  1  1  1  1  1  1  1  1  1 	product=1            sum=9 (example puzzle solved)
//
//Steps E-Y. Still unknown.
//
//Step Z. Last resort: Reorder digits (relabel) in accending order by number of templates, perform 9 nested loops, find all disjoint templates.
//
//No-solution condition: A digit with 0 template candidates.
//Single-solution condition: 9 disjoint templates remain, one for each digit.
//Multiple-solution condition: More than one possibility to form 9 disjoint templates, one for each digit.
//
//The basic methods include:
//- Solve a cell if only templates for one of the digits hit it. Remove the templates for the same digit that not hit the solved cell.
//- Solve a digit if there is a cell covered by only one template. Remove rest of the templates for this digit. Remove the templates for other digits that hit the solved digit.
//- Other methods, suggested (not yet) by experts.
//
//Step B rate is ~500 puzzles/sec.
//Generating large amount of solutions for multiple-solution puzzle is expected to be fairly fast.

	bm128 allClues; //all givens
	unsigned short invalids[46656]; //bit (d - 1) is set if the respective template is forbidden for digit d.
	int kTemplCount[9];
	int validCount[9]; //how many valid templates remain for each digit
	unsigned long long ret = 0;
	//ch81 prBuf; //print buffer for debugging/explaining
#ifdef __INTEL_COMPILER
#pragma noparallel
#endif //__INTEL_COMPILER
	for(int t = 0; t < 46656; t++)
		invalids[t] = 0; //initially there are no invalid templates
	for(int d = 0; d < 9; d++) {
		clues[d].clear(); //initially there are no clues
		validCount[d] = 46656; //initially every digit has 46656 valid templates
		invalid[d].clear(); //initially all digits have no invalid positions
		kTemplCount[d] = 0;
	}
	allClues.clear();
	for(int c = 0; c < 81; c++) {
		int d = p[c];
		if(d--) { //work with zero based digits 0..8
			clues[d].setBit(c); //cell c is given for digit d
			allClues.setBit(c); //cell c is given
			for(int r = 0; r < 20; r++) { //all templates visible from the given are invalid
				invalid[d].setBit(affectedCells[c][r]); //related cell r can't contain the same digit d
			}
		}
	}
	for(int d = 0; d < 9; d++) {
		bm128 rest; //all givens but d
		rest = allClues;
		rest ^= clues[d];
		invalid[d] |= rest; //d can't share cells with other givens
	}
	//the only one missing bit in the houses of the givens in invalid[] guaranees also the existence of the givens
	for(int t = 0; t < 46656; t++) {
		for(int d = 0; d < 9; d++) {
			bm128 tt = allTemplates[t];
			tt &= invalid[d]; //find forbidden positions for digit d in template t
			if(!tt.isZero()) { //invalid template for this digit
				invalids[t] |= Digit2Bitmap[d + 1]; //mark the respective bit
				validCount[d]--; //one less template candidate
			}
		}
	}

	//for(int c = 0; c < 81; c++) {
	//	printf("%c", p[c] + '0');
	//}
	//printf("\n");

	int tCount = 0; //all template candidates
	int tCounts[9]; //template candidates per digit
	int solvedDigits = 0;
	unsigned long long rating = 1;
	for(int d = 0; d < 9; d++) {
		tCount += validCount[d];
		tCounts[d] = 0;
		printf("%d ", validCount[d]);
		rating *= validCount[d];
		if(validCount[d] == 1)
			solvedDigits |= Digit2Bitmap[d + 1];
	}
	printf("\t%llu (pass 1) tCount = %d\n", rating, tCount);

	bm128 **digitTemplates[9]; //9 pointers to arrays of the valid templates
	digitTemplates[0] = (bm128**)malloc(tCount * sizeof(bm128*));
	for(int d = 0; d < 9 - 1; d++) {
		digitTemplates[d + 1] = digitTemplates[d] + validCount[d];
	}

	//obtain pointers to the valid templates
	for(int t = 0; t < 46656; t++) {	
		if(511 == (invalids[t] & 511))
			continue; //nothing to do with entirely invalid template
		for(int d = 0; d < 9; d++) {
			if(0 == (invalids[t] & Digit2Bitmap[d + 1])) {
				digitTemplates[d][tCounts[d]++] = &allTemplates[t];
			}
		}
	}


	bool repeat;
	do {
restart:

		//printf("\nSurvived templates\n");
		//for(int d = 0; d < 9; d++) {
		//	for(int t = 0; t < validCount[d]; t++) {
		//		digitTemplates[d][t]->toMask81('1' + d, prBuf.chars);
		//		printf("%81.81s\t%d\n", prBuf.chars, t + 1);
		//	}
		//}

		repeat = false;
		//fix the templates which are the only candidate for a cell
		bm128 allPoss;
		allPoss.clear();
		bm128 duplicates;
		duplicates.clear();
		for(int d = 0; d < 9; d++) {
			bm128 newSolvedCells;
			newSolvedCells -= clues[d]; //all cells except givens are candidates for digit d
			for(int t = 0; t < validCount[d]; t++) {
				duplicates |= (allPoss & *digitTemplates[d][t]);
				allPoss |= *digitTemplates[d][t];
				newSolvedCells &= *digitTemplates[d][t]; //d is not in this cell for at least one template, leave cell unsolved.
			}
			//add to newly solved cells these having this digit as only candidate
			bm128 allDigitPoss = maskLSB[81].m128i_m128i; //all cells are candidates for digit 2
			for(int d2 = 0; d2 < 9; d2++) {
				if(d2 == d)
					continue;
				for(int t2 = 0; t2 < validCount[d2]; t2++) {
					allDigitPoss.clearBits(*digitTemplates[d2][t2]); //exclude cells where at least one <> d candidate exists
				}
			}
			allDigitPoss.clearBits(clues[d]); //remove also previously known givens
			if(!allDigitPoss.isZero()) {
				int firstRemoved = -1;
				for(int t = 0; t < validCount[d]; t++) {
					bm128 notSolvedCell = allDigitPoss;
					notSolvedCell.clearBits(*digitTemplates[d][t]);
					if(!notSolvedCell.isZero()) {
						//a candidate template with missing solved cell is invalid
						digitTemplates[d][t] = NULL;
						if(firstRemoved == -1) {
							firstRemoved = t;
						}
					}
				}
				if(firstRemoved != -1) {
					//cleanup the NULL template pointers
					int t2 = firstRemoved;
					for(int t1 = firstRemoved + 1; t1 < validCount[d]; t1++) {
						if(digitTemplates[d][t1])
							digitTemplates[d][t2++] = digitTemplates[d][t1];
					}
					validCount[d] = t2;
					repeat = true; //perform next pass
					if(t2 == 0) {
						printf("No valid template for digit %d.\n", d + 1); //no solution
						goto exit;
					}
					if(t2 == 1) {
						clues[d] = *digitTemplates[d][0];; //solved digit
						solvedDigits |= Digit2Bitmap[d + 1];
					}
					goto restart; //???
				}
				newSolvedCells |= allDigitPoss;
			}
			//eliminate contradicting templates from other digits for newly solved cells
			if(!newSolvedCells.isZero()) {
				clues[d] |= newSolvedCells;
				for(int d2 = 0; d2 < 9; d2++) {
					if(d2 == d)
						continue;
					int firstRemoved = -1;
					for(int t2 = 0; t2 < validCount[d2]; t2++) {
						if(!newSolvedCells.isDisjoint(*digitTemplates[d2][t2])) {
							//remove this template from the list
							digitTemplates[d2][t2] = NULL;
							if(firstRemoved == -1)
								firstRemoved = t2;
						}
					}
					if(firstRemoved != -1) {
						//cleanup the NULL template pointers
						int t2 = firstRemoved;
						for(int t1 = firstRemoved + 1; t1 < validCount[d2]; t1++) {
							if(digitTemplates[d2][t1])
								digitTemplates[d2][t2++] = digitTemplates[d2][t1];
						}
						validCount[d2] = t2;
						repeat = true; //perform next pass
						if(t2 == 0) {
							printf("No valid template for digit %d.\n", d2 + 1); //no solution
							goto exit;
						}
						if(t2 == 1) {
							clues[d2] = *digitTemplates[d2][0];; //solved digit
							solvedDigits |= Digit2Bitmap[d2 + 1];
						}
						//goto restart; //20% slower
					}
				}
				goto restart; //1% slower
			}
		}
		bm128 uniques = allPoss;
		uniques ^= duplicates;
		for(int d = 0; d < 9 && (!uniques.isZero()); d++) { //exit the loop when uniques exhausted
			if(validCount[d] == 1) {
				//it is OK a solved digit to occur once in all template candidates
				uniques.clearBits(*digitTemplates[d][0]); //cleanup uniques
				continue;
			}
			for(int t = 0; t < validCount[d]; t++) {
				if(!digitTemplates[d][t]->isDisjoint(uniques)) {
					validCount[d] = 1; //solve this digit
					solvedDigits |= Digit2Bitmap[d + 1];
					digitTemplates[d][0] = digitTemplates[d][t]; //assign pointer
					clues[d] = *digitTemplates[d][0];
					uniques.clearBits(*digitTemplates[d][0]); //cleanup uniques
					repeat = true;
					//goto restart; //1.3% faster for newSolvedCells disabled
				}
			}
		}
		if(repeat && solvedDigits != 511)
			goto restart;

		//filter out templates which haven't at least one disjoint template for each of the rest digits
		tCount = 0;
		for(int d1 = 0; d1 < 9; d1++) {
			int firstRemoved = -1;
			for(int t1 = 0; t1 < validCount[d1]; t1++) {
				//find at least one disjoint template from the rest digits
				int conflict;
				for(int d2 = 0; d2 < 9; d2++) {
					if(d1 == d2)
						continue;
					conflict = 1;
					for(int t2 = 0; t2 < validCount[d2]; t2++) {
						if(digitTemplates[d1][t1]->isDisjoint(*digitTemplates[d2][t2])) {
							conflict = 0;
							break; //stop at first found
						}
					}
					if(conflict) { //invalid template [d1][t1]
						//remove this template from the list
						printf("\nElimination of template %d for digit %d due to conflict with each of the templates for digit %d.\n", t1 + 1, d1 + 1, d2 + 1); //no solution
						digitTemplates[d1][t1] = NULL;
						if(firstRemoved == -1)
							firstRemoved = t1;
						break;
					}
				}
			}
			if(firstRemoved != -1) {
				//cleanup the NULL template pointers
				int t2 = firstRemoved;
				for(int t1 = firstRemoved + 1; t1 < validCount[d1]; t1++) {
					if(digitTemplates[d1][t1])
						digitTemplates[d1][t2++] = digitTemplates[d1][t1];
				}
				validCount[d1] = t2;
				repeat = true; //perform next pass
				if(t2 == 0) {
					printf("No valid template for digit %d.\n", d1 + 1); //no solution
					goto exit;
				}
				if(t2 == 1) {
					clues[d1] = *digitTemplates[d1][0];; //solved digit
					solvedDigits |= Digit2Bitmap[d1 + 1];
				}
				//goto restart; //~8% slower
			}
		}
		rating = 1;
		tCount = 0;
		for(int d = 0; d < 9; d++) {
			printf("%d ", validCount[d]);
			rating *= validCount[d];
			tCount += validCount[d];
		}
		printf("\t%llu (pass 2) tCount = %d\n", rating, tCount);
	} while(repeat && (solvedDigits != 511)); //some template removed and the rest of the templates are still not unique

	rating = 1;
	tCount = 0;
	for(int d = 0; d < 9; d++) {
		printf("%d\t", validCount[d]);
		rating *= validCount[d];
		tCount += validCount[d];
	}
	printf("%d\t%llu\n", tCount, rating);

	//if(tCount > 9) ret = 2; else ret = 1; goto exit;

	if(tCount > 9) {
		//find 9 disjoint templates
		kTemplCount[0] = validCount[0];
		for(int t1 = 0; t1 < validCount[0]; t1++) {
			for(int t2 = 0; t2 < validCount[1]; t2++) {
				bm128 t12 = *digitTemplates[0][t1];
				if(!digitTemplates[1][t2]->isDisjoint(t12))
					continue;
				kTemplCount[1]++;
				t12 |= *digitTemplates[1][t2];
				for(int t3 = 0; t3 < validCount[2]; t3++) {
					if(!digitTemplates[2][t3]->isDisjoint(t12))
						continue;
					kTemplCount[2]++;
					bm128 t123 = t12;
					t123 |= *digitTemplates[2][t3];
					for(int t4 = 0; t4 < validCount[3]; t4++) {
						if(!digitTemplates[3][t4]->isDisjoint(t123))
							continue;
						kTemplCount[3]++;
						bm128 t1234 = t123;
						t1234 |= *digitTemplates[3][t4];
						for(int t5 = 0; t5 < validCount[4]; t5++) {
							if(!digitTemplates[4][t5]->isDisjoint(t1234))
								continue;
							kTemplCount[4]++;
							bm128 t12345 = t1234;
							t12345 |= *digitTemplates[4][t5];
							for(int t6 = 0; t6 < validCount[5]; t6++) {
								if(!digitTemplates[5][t6]->isDisjoint(t12345))
									continue;
								kTemplCount[5]++;
								bm128 t123456 = t12345;
								t123456 |= *digitTemplates[5][t6];
								for(int t7 = 0; t7 < validCount[6]; t7++) {
									if(!digitTemplates[6][t7]->isDisjoint(t123456))
										continue;
									kTemplCount[6]++;
									bm128 t1234567 = t123456;
									t1234567 |= *digitTemplates[6][t7];
									for(int t8 = 0; t8 < validCount[7]; t8++) {
										if(!digitTemplates[7][t8]->isDisjoint(t1234567))
											continue;
										kTemplCount[7]++;
										bm128 t12345678 = t1234567;
										t12345678 |= *digitTemplates[7][t8];
										for(int t9 = 0; t9 < validCount[8]; t9++) {
											if(!digitTemplates[8][t9]->isDisjoint(t12345678))
												continue;
											kTemplCount[8]++;
											ret++;
											//bm128 t123456789 = t12345678;
											//t123456789 |= *digitTemplates[8][t9];
											if(ret % 10000 == 0) {
												printf("Sol: %llu", ret);
												for(int d = 0; d < 8; d++) {
													printf("\t%d", kTemplCount[d]); //kTemplCount[8] is the number of solutions
												}
												printf("\n");
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
		for(int d = 0; d < 9; d++) {
			printf("%d ", kTemplCount[d]); //kTemplCount[8] is the number of solutions
		}
		printf("\t(pass 3) tCount=%d\n", tCount);
	}
exit:
	free(digitTemplates[0]);
	return ret;
}

void templates::init() {
	int nTemplates = 0;
	for(int r1c = 0; r1c < 9; r1c++) { //row 1 columns
		int r1box = r1c / 3;
		int nColTemplates = 0;
		for(int r2c = 0; r2c < 9; r2c++) {
			int r2box = r2c / 3;
			if(r2box == r1box)
				continue;
			for(int r3c = 0; r3c < 9; r3c++) {
				int r3box = r3c / 3;
				if(r3box == r1box || r3box == r2box)
					continue;
				for(int r4c = 0; r4c < 9; r4c++) {
					int r4box = r4c / 3;
					if(r4c == r1c || r4c == r2c || r4c == r3c)
						continue;
					for(int r5c = 0; r5c < 9; r5c++) {
						int r5box = r5c / 3;
						if(r5box == r4box || r5c == r1c || r5c == r2c || r5c == r3c)
							continue;
						for(int r6c = 0; r6c < 9; r6c++) {
							int r6box = r6c / 3;
							if(r6box == r4box || r6box == r5box || r6c == r1c || r6c == r2c || r6c == r3c)
								continue;
							for(int r7c = 0; r7c < 9; r7c++) {
								int r7box = r7c / 3;
								if(r7c == r1c || r7c == r2c || r7c == r3c || r7c == r4c || r7c == r5c || r7c == r6c)
									continue;
								for(int r8c = 0; r8c < 9; r8c++) {
									int r8box = r8c / 3;
									if(r8box == r7box || r8c == r1c || r8c == r2c || r8c == r3c || r8c == r4c || r8c == r5c || r8c == r6c)
										continue;
									for(int r9c = 0; r9c < 9; r9c++) {
										int r9box = r9c / 3;
										if(r9box == r7box || r9box == r8box || r9c == r1c || r9c == r2c || r9c == r3c || r9c == r4c || r9c == r5c || r9c == r6c)
											continue;
										//this combination of r1c..r9c conforms Sudoku rules for valid locations of a single digit
										bm128 t;
										t.clear();
										t.setBit(r1c);
										t.setBit(9 + r2c);
										t.setBit(18 + r3c);
										t.setBit(27 + r4c);
										t.setBit(36 + r5c);
										t.setBit(45 + r6c);
										t.setBit(54 + r7c);
										t.setBit(63 + r8c);
										t.setBit(72 + r9c);
										allTemplates[nTemplates++] = t;
										colTemplates[r1c][nColTemplates++] = t;
									}
								}
							}
						}
					}
				}
			}
		}
	}
//	for(int i = 0; i < 9; i++) {
//		colTemplatesIndexAll[i] = colTemplates_index_type(colTemplates[i], 5184, colTemplatesIndexes[i]); //build colTemplatesIndexes
//	}
}

//do the job defined in global structure "opt"
extern int processTemplate() {
	templates t;
	if(opt.templateOpt->get999911110)
		t.get999911110();
	else if(opt.templateOpt->r4tot4 || opt.templateOpt->r4tot5) {
		char buf[1000];
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz;
			puz.fromString(buf);
			exemplarSet ex;
			bm128 r;
			t.template2rookery(puz, r);
			int cnt;
			if(opt.templateOpt->r4tot4) {
				cnt = t.rookery2templates4(r, ex) * 24; //scale the count by 4! for relabeling
			}
			else {
				bm128 rr = maskLSB[81];
				rr.clearBits(r);
				cnt = t.rookery2templates5(rr, ex) * 120; //scale the count by 5! for relabeling
			}
			ch81 can;
			ch81 fixed;
			if(opt.verbose) {
				//list all exemplars w/o counters
				for(exemplarSet::const_iterator e = ex.begin(); e != ex.end(); e++) {
					e->can.toString(can.chars);
					e->exemplar.toString(fixed.chars);
					printf("%81.81s\t%81.81s\n", can.chars, fixed.chars);
				}
			}
			else {
				//list only one exemplar and counters
				if(cnt) {
					ex.begin()->can.toString(can.chars);
					ex.begin()->exemplar.toString(fixed.chars);
					printf("%81.81s\t%d\t%d\t%81.81s\t%81.81s\n", buf, cnt, (int)ex.size(), can.chars, fixed.chars);
				}
				else {
					printf("%81.81s\t%d\t%d\t%81.81s", buf, cnt, (int)ex.size());
				}
			}
		}
	}
	else {
		fprintf(stderr, "Unknown/missing template sub-command,\n");
		return -1;
	}
	return 0;
}

void countSolutions () {
	templates x;
	char buf[1000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		puz.fromString(buf);
		printf("%s", buf);
		x.nSol(puz.chars);
	}

	//for(int i = 0; i < 81; i++) {
	//	//p[i] = "100000009000000020780030000000005300010200040000000000040900000000010800002000005"[i] - '0'; //17 1 71 103 62 6000 745 121 90 375,207,565,926,600,000 easy (no guesses)
	//	//p[i] = "100000009000000000780030000000005300010200040000000000040900000000010800002000005"[i] - '0'; //multiple solutions
	//	//p[i] = "100000002090400050006000700050903000000070000000850040700000600030009080002000001"[i] - '0'; //41 39 130 32 18 41 8 148 18 104,621,842,391,040 Easter Monster gd=8
	//	//p[i] = "000000039000001005003050800008090006070002000100400000009080050020000600400700000"[i] - '0'; //47 37 108 38 32 96 52 12 16 218,893,425,573,888 Golden Nugget gd=8
	//	//p[i] = "000000012000000003002300400001800005060070800000009000008500000900040500470006000"[i] - '0'; //65 111 109 34 12 50 73 24 47 Platinium Blonde
	//	//p[i] = "000000007020400060100000500090002040000800600600900000005003000030080020700004001"[i] - '0'; //42 10 113 24 42 28 57 144 88 Tungston Rod
	//	//p[i] = "000000003001005600090040070000009050700000008050402000080020090003500100600000000"[i] - '0'; //34 150 66 154 9 68 66 65 7 Fata Morgana
	//	//p[i] = "100000007020400060003000500090040000000062040000900800005000003060200080700001000"[i] - '0'; //34 22 36 18 44 16 43 92 116 156,589,823,655,936 Silver Plate gd=10
	//	//p[i] = "000100203000402000000035001670000480003000600048000039700850000050206000806009000"[i] - '0'; //34 17 4 12 22 8 62 7 80 9.9/9.9/2.6
	//	//p[i] = "000000002001002700030040050004905060000060000060403500090030040007800100800000000"[i] - '0'; //32 38 15 6 53 49 60 38 132 11.4/11.4/10.8
	//	//p[i] = "900000800070060000005004003000003500040090020006100004200500000000080070001006005"[i] - '0'; //61 44 61 20 5 20 25 44 46 11.3/11.3/10.4 champagne
	//	//game 146
	//	//p[i] = "000009002003050080060800400005000020090060001100004900007005200080100070300070000"[i] - '0'; //14 24 60 41 13 33 32 10 18 10.8/10.8/10.1 champagne
	//	//p[i] = "000002004003050070090600200006000010070040008500009700001008500060700080800010000"[i] - '0'; //14 80 281 28 9 10 6 10 49 10.7/10.7/10.6 champagne
	//	//p[i] = "000007004008090060030100900009000050060080007400001600005004200010200030200010000"[i] - '0'; //8 24 29 5 48 43 40 33 23 9.9/9.9/9.9 ronk
	//	//p[i] = "000001004003070080060500700005000030070090001800004600009003200050600090200080000"[i] - '0'; //40 64 5 28 10 5 23 8 17 9.8/9.8/9.8 champagne
	//	//p[i] = "000001002003040050060700800009000010010020003800003400002005600070200090400070000"[i] - '0'; //23 5 14 8 82 35 16 46 36 9.7/9.7/9.7 joel64
	//	//p[i] = "000008004009050030070100500005000070030080002100006300004005200080600090600010000"[i] - '0'; //10 80 43 54 5 10 48 14 29 9.6/9.6/9.6 champagne
	//	//p[i] = "000001002003040050060700400004000070050080001200007800008002900010900060900070000"[i] - '0'; //8 5 281 23 66 29 12 20 24 2,850,089,932,800 9.5/9.5/9.5 m_b_metcalf
	//	//p[i] = "000001002003040050060700800007000010010020008900005400005004600080500070200060000"[i] - '0'; //6.7/6.7/6.7 - joel64
	//	p[i] = "000001002003040050060700800006000070040020003100004900008009500070800060600050000"[i] - '0'; //6.6/6.6/6.6 - joel64 (no guesses)
	//	//p[i] = "000001004003050070060700900005000060070040002800002400009005200050600010100080000"[i] - '0'; //6.5/3.0/3.0 - champagne
	//	//p[i] = "000009004001060090020300600008000070090040008700006500004003200010200030200010000"[i] - '0'; //6.2/6.2/6.2 + ronk
	//	//p[i] = "000002007005060090070400800009000060010080005700003400006005100040200030200010000"[i] - '0'; //4.4/4.4/4.4 - ronk
	//	//p[i] = "000008009009060080020300600004000090080070004700006100005003200040200030200010000"[i] - '0'; //4.2/4.2/4.2 - ronk
	//	//p[i] = "000009005004060010070400300008000090020050008700006500003004100040300020200010000"[i] - '0'; //4.0/4.0/4.0 - ronk
	//	//p[i] = "000001002003040050060700800009000010010020003600004200008003500070800090900050000"[i] - '0'; //3.6/3.6/3.6 - joel64
	//	//p[i] = "000001002003040050060700800009000010010030009300004600008006500020800070900050000"[i] - '0'; //3.4/3.4/3.4 - joel64
	//	//p[i] = "000003009006070050050200800009000080080020007100007600005002400010400030200010000"[i] - '0'; //3.2/3.2/3.2 - ronk
	//	//p[i] = "000001005008090020020300800005000090030040006800007400007006500050400030200010000"[i] - '0'; //3.0/3.0/3.0 - ronk
	//	//p[i] = "000001002003040010040500600007000030010070008900002500004007800020400060800060000"[i] - '0'; //2.8/2.8/2.8 - joel64
	//	//p[i] = "000006001002070090010200300005000060090080005800004700006002400050400030200010000"[i] - '0'; //2.5/2.5/2.5 - ronk
	//	//p[i] = "000001002003040010040500600007000030080050004900002500002007800090300060800060000"[i] - '0'; //2.0/2.0/2.0 - joel64
	//	//p[i] = "000001002003040050050600100007000040060020008400006300006003800040900010900060000"[i] - '0'; //1.5/1.5/1.5 + m_b_metcalf (no guesses)
	//	//p[i] = "000008004003060080060700100005000030030080006100004700006005200040300010200010000"[i] - '0'; //1.2/1.2/1.2 - ronk (no guesses)
	//}
	//nSol(p);
}

//void generate4rookeries() {
//	templates tpl;
//
//	puzzleSet t3all;
//	tpl.get3templates(t3all);
//	lightweightUsetList r4;
//	int n = 0;
//	for(puzzleSet::const_iterator t = t3all.begin(); t != t3all.end(); t++) {
//		puzzleSet t4;
//		tpl.templatePlus1(*t, t4);
//		for(puzzleSet::const_iterator tt = t4.begin(); tt != t4.end(); tt++) {
//			bm128 r;
//			tpl.template2rookery(*tt, r);
//			r4.insert(r);
//		}
//		n++;
//		if(0 == n % 1000) {
//			fprintf(stderr, "t3=%d\tr4=%d\n", n, (int)r4.size());
//			fflush(NULL);
//			//if(n == 10000) return;
//		}
//	}
//	for(lightweightUsetList::const_iterator r = r4.begin(); r != r4.end(); r++) {
//		//dump the rookery
//		ch81 buf;
//		for(int i = 0; i < 81; i++) {
//			buf.chars[i] = r->isBitSet(i) ? '1' : '.';
//		}
//		printf("%81.81s\n", buf.chars);
//	}
//	fprintf(stderr, "t3=%d\tr4=%d\n", n, (int)r4.size());
//	fflush(NULL);
//}

//extern void test() {countSolutions();}
struct uaByTemplate: public map<ch81,usetListBySize> {};
extern void test() {
	templates tpl;
	tpl.count5templates();
	//tpl.countGrids();
	//tpl.generateGrids();
	//tpl.generateXX110();
	return;
/*
	char buf[1000];
//	int n = 0;
	unsigned long long nProcessed = 0;
//	unsigned long long sumGrids = 0;
	unsigned long long sumT5 = 0;
	while(fgets(buf, sizeof(buf), stdin)) {
//		bm128 r4;
		bm128 r5;
//		ch81 r4txt;
//		ch81 r4can;
//		n++;
//		if(n <= 800000) continue;
//		if(n >  801000) break;
//		r4.clear();
		r5.clear();
		for(int i = 0; i < 81; i++) {
			if(buf[i] == '1') {
//				r4.setBit(i);
//				r4txt.chars[i] = 1;
			}
			else {
				r5.setBit(i);
//				r4txt.chars[i] = 0;
			}
		}
//		patminlex pml(r4txt.chars, r4can.chars);
//		puzzleSet t4;
		puzzleSet t5;
		tpl.rookery2templates(r5, t5); // ~20 per second
		int t5initialSize = (int)t5.size();
//		tpl.rookery2templates(r4, t4); // 2502.098 seconds
//		for(puzzleSet::const_iterator p5 = t5.begin(); p5 != t5.end();) {
//			//skip (4 of 5) subsets that have canonical representation less than the examined r4
//			int eqmask = 0;
//			ch81 minT4FromEqualRookery;
//			for(int v = 1; v < 6; v++) {
//				ch81 r4;
//				for(int i = 0; i < 81; i++) {
//					r4.chars[i] = (p5->chars[i] == v || p5->chars[i] == 0) ? 0 : 1;
//				}
//				ch81 can;
//				patminlex pml(r4.chars, can.chars);
//				if(can < r4can) {
//					//puzzleSet::const_iterator tmp = p5;
//					//p5++;
//					//t5.erase(tmp);
//					p5 = t5.erase(p5);
//					goto nextp5;
//				}
//				else if(can == r4can) {
//					eqmask |= 1 << v;
//					ch81 tt4;
//					for(int i = 0; i < 81; i++) {
//						tt4.chars[i] = (p5->chars[i] == v || p5->chars[i] == 0) ? 0 : p5->chars[i];
//					}
//					ch81 can;
//					patminlex pml(tt4.chars, can.chars);
//					if(eqmask && can < minT4FromEqualRookery) {
//						minT4FromEqualRookery = can; //structure copy
//					}
////					eqmask = 1;
//				}
//			}
//			//r4 elimination passed
//			if(eqmask) {
//				//todo: combine minT4FromEqualRookery with only lesser t4
//			}
//			p5++;
//			nextp5:;
//		}
		//printf("%81.81s\t%d\t%d\t%d\t%d\n", buf, (int)t4.size(), t5initialSize, (int)t5.size(), (int)(t4.size() * t5.size()));
		nProcessed++;
		sumT5 += t5initialSize;
		//sumGrids += t4.size() * t5.size();
		//printf("%81.81s\t%d\t%d\n", buf, n, (int)t5.size());
		//if(t5.empty()) printf("%81.81s\t%d\n", buf, n);
		if(0 == nProcessed % 1000) {
			fprintf(stderr, "%llu\t%llu\n", nProcessed, sumT5);
			fflush(NULL);
		}
	}
	printf("Total T5 templates\t%llu\n", sumT5);
	//fprintf(stderr, "Processed=%llu\tSum=%llu\tAverage=%llu\n", nProcessed, sumGrids, sumGrids / nProcessed);

//	puzzleSet r2all;
//	tpl.get2templates(r2all);
//	uaByTemplate ua2digit;
//	for(puzzleSet::const_iterator s = r2all.begin(); s != r2all.end(); s++) {
//		//find first solution grid
//		grid sol;
//		if(1 == solve(s->chars, 1, sol.digits)) {
//			//template can be completed to a valid solution grid
//			//uaByTemplate::mapped_type& x = ua2digit[*s];
//			usetListBySize& ua = ua2digit[*s];
//			digit2bitmap(sol.digits, sol.gridBM);
//			sol.findUAinPuzzle(s->chars);
//			ua.insert(sol.usetsBySize.begin(), sol.usetsBySize.end());
//		}
//	}
//	printf("Total 2: %d\n", (int)ua2digit.size());
//	for(uaByTemplate::const_iterator t = ua2digit.begin(); t != ua2digit.end(); t++) {
//		ch81 res;
//		t->first.toString(res.chars);
//		printf("\n%81.81s\n", res.chars);
//		for(usetListBySize::const_iterator u = t->second.begin(); u != t->second.end(); u++) {
//			//u->toMask81(res.chars);
//			//u->toPuzzleString(t->first.chars, res.chars);
//			//printf("%81.81s\n", res.chars);
//			int m = 0;
//			int n = 0;
//			for(int i = 0; i < 81; i++) {
//				if(t->first.chars[i]) {
//					if(u->isBitSet(i)) {
//						m |= (1U << n++);
//					}
//					else {
//						n++;
//					}
//				}
//			}
//			printf("%8.8X\n", m);
//		}
//	}
//	//r2all.saveToFile(stdout);

//	puzzleSet r3all;
//	tpl.get3templates(r3all);
//	uaByTemplate ua3digit;
//	for(puzzleSet::const_iterator s = r3all.begin(); s != r3all.end(); s++) {
//		//find first solution grid
//		grid sol;
//		if(1 == solve(s->chars, 1, sol.digits)) {
//			//template can be completed to a valid solution grid
//			//uaByTemplate::mapped_type& x = ua2digit[*s];
//			usetListBySize& ua = ua3digit[*s];
//			digit2bitmap(sol.digits, sol.gridBM);
//			sol.findUAinPuzzle(s->chars);
//			ua.insert(sol.usetsBySize.begin(), sol.usetsBySize.end());
//		}
//	}
//	printf("Total 3: %d\n", (int)ua3digit.size());
//	for(uaByTemplate::const_iterator t = ua3digit.begin(); t != ua3digit.end(); t++) {
//		ch81 res;
//		t->first.toString(res.chars);
//		printf("\n%81.81s\n", res.chars);
//		for(usetListBySize::const_iterator u = t->second.begin(); u != t->second.end(); u++) {
//			//u->toMask81(res.chars);
//			u->toPuzzleString(t->first.chars, res.chars);
//			printf("%81.81s\n", res.chars);
//		}
//	}
////	r3all.saveToFile(stdout);

 */
}
