#ifndef OPTIONS_H_INCLUDED

#define OPTIONS_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include "anyoption.h"
#include "tables.h"
#include <limits.h>
#include <memory.h>

extern int scanGridsFromFile();
extern int fastScan();

extern unsigned long long solve(const char* in, const unsigned long long maxSolutions, char* out);
extern void digit2bitmap(const char* in, int* out);
extern unsigned long long solve(const int* gridBM, const char* in, const unsigned long long maxSolutions);
extern unsigned long long solve(const char* in, const unsigned long long maxSolutions);
extern unsigned long long solve(const char* in, int* pencilMarks);
extern double solverRate(const char* in);
extern int solverBackdoor(char* in, const bool verbose);

extern int neighbourhood(const char* fname, const char* knownsfname);
extern int uaDist();
extern int processUA();
//extern int groupBySolution(const char *inFileName, const char *outFileName, const bool noPuz, const bool minimals);
extern int groupBySolution(const bool noPuz, const bool verbose);
extern int doSimilarPuzzles();
extern int processPatterns();
extern int processTemplate();

struct scanOptions;
struct solveOptions;
struct nhbOptions;
struct uaOptions;
struct similarOptions;
struct patternOptions;
struct templateOptions;

class options {
	AnyOption anyopt;
	time_t startTime;
public:
	scanOptions *scanOpt;
	solveOptions *solveOpt;
	nhbOptions *nhbOpt;
	uaOptions *uaOpt;
	similarOptions *similarOpt;
	patternOptions *patternOpt;
	templateOptions *templateOpt;
	bool verbose;
	options();
	bool read(int argc, char* argv[]);
	int execCommand();
	void printUsage();
	const char* getValue(const char* key);
	bool getFlag(const char* key);
	const char* getStartTime() const;
};

extern options opt;

struct solveOptions {
	//const char * puzFile;
	//const char* groupByGridFile;
	unsigned long long maxsolutioncount;
	bool groupByGrid;
	bool gridsOnly;
	bool minimals;
	bool count;
	bool rate;
	bool backdoor;
	solveOptions() { //set defaults for all solver options
		//puzFile = NULL;
		//groupByGridFile = NULL;
		maxsolutioncount = ULLONG_MAX; //INT_MAX;
		groupByGrid = false;
		gridsOnly = false;
		minimals = false;
		count = false;
		rate = false;
		backdoor = false;
	}
	int go() { //collect all solver options and do the job
		//puzFile = opt.getValue("puzzles");
		//groupByGridFile = opt.getValue("groupbygrid");
		groupByGrid = opt.getFlag("groupbygrid");
		gridsOnly = opt.getFlag("gridsonly");
		minimals = opt.getFlag("minimals");
		count = opt.getFlag("count");
		rate = opt.getFlag("rate");
		backdoor = opt.getFlag("backdoor");
		const char* v = opt.getValue("maxsolutioncount");
		if(v) {
			sscanf(v, "%llu", &maxsolutioncount);
			//maxsolutioncount = atoi(v);
			if(maxsolutioncount <= 0) {
				cout << "Error: Maximum Solution Count must be a positive integer!" << endl;
				return -1;
			}
		}
		//if(puzFile && groupByGridFile) //--solve --puzzles puzlist.txt --groupbygrid result.txt [--gridsonly] [--minimals]
		//	return groupBySolution(puzFile, groupByGridFile, gridsOnly, minimals);
		if(groupByGrid) //--solve --groupbygrid [--gridsonly] < puzzles.txt > result.txt
			return groupBySolution(gridsOnly, opt.verbose);
		//--solve
		char sol[2][81];
		char buf[3000];
		int stat[81][5]; //SS M, MS M, SS NM, MS NM, NS == SingleSolution Minimal, MultipleSolutions NonMinimal, NoSolution
		memset(stat, 0, 81*5*sizeof(stat[0][0]));
		while(fgets(buf, sizeof(buf), stdin)) {
			int nClues = 81;
			printf("%81.81s\t", buf);
			for(int i = 0; i < 81; i++) {
				buf[i] -= '0';
				if(buf[i] <= 0 || buf[i] > 9) {
					buf[i] = 0;
					nClues--;
				}
			}
			printf("%d\t", nClues);
			if(rate) {
				double rating = solverRate(buf);
				printf("%.2f\n", rating);
				goto nextPuzzle;
			}
			if(backdoor) {
				int bd = solverBackdoor(buf, opt.verbose);
				if(opt.verbose) {
					printf("\n");
				}
				else {
					printf("%d\n", bd);
				}
				goto nextPuzzle;
			}
			unsigned long long nSol;
			if(count) { //--solve --count < puzlist.txt > solutionCount.txt
				nSol = solve(buf, maxsolutioncount);
				if(nSol < maxsolutioncount) {
					printf("%llu\n", nSol);
				}
				else {
					printf("max\n");
				}
				stat[nClues][3]++;
			}
			//else if(1) {
			//	int pm[81];
			//	nSol = solve(buf, pm);
			//	if(nSol) {
			//		for(int i = 0; i < 81; i++) {
			//			if(BitCount[pm[i]] == 1) {
			//				for(int j = 1; j <= 9; j++) {
			//					if(pm[i] == Digit2Bitmap[j]) {
			//						sol[0][i] = '0' + j;
			//						break;
			//					}
			//				}
			//			}
			//			else {
			//				sol[0][i] = 'A' - 1 + BitCount[pm[i]];
			//			}
			//		}
			//		printf("%llu\t%81.81s\n", nSol, sol[0]);
			//	}
			//	else {
			//		printf("0\n");
			//	}
			//	stat[nClues][3]++;
			//}
			else { //--solve [--minimals] < puzlist.txt > solved.txt
				nSol = solve(buf, 2, sol[0]);
				switch(nSol) {
				case 1: //single solution
					if(minimals) {
						//check for minimality
						int solBM[81];
						digit2bitmap(sol[0], solBM);
						for(int pos = 0; pos < 81; pos++) {
							int t = buf[pos];
							if(t) {
								buf[pos] = 0;
								unsigned long long n = solve(solBM, buf, 2);
								buf[pos] = t;
								if(n == 1) {
									//given at pos is redundant
									printf("redundant clue %c at %d\n", (char)(t + '0'), pos);
									stat[nClues][2]++;
									goto nextPuzzle;
								}
							}
						}
					}
					for(int i = 0; i < 81; i++) {
						sol[0][i] += '0';
					}
					printf("%81.81s\n", sol[0]);
					stat[nClues][0]++;
					break;
				case 0:
					stat[nClues][4]++;
					printf("no solution\n");
					break;
				default:
					if(minimals) {
						//check for minimality
						nSol = solve(buf, ULLONG_MAX);
						for(int pos = 0; pos < 81; pos++) {
							int t = buf[pos];
							if(t) {
								buf[pos] = 0;
								unsigned long long n = solve(buf, nSol + 1);
								buf[pos] = t;
								if(n == nSol) {
									//given at pos is redundant
									printf("redundant clue %c at %d, ", (char)(t + '0'), pos);
									stat[nClues][3]++;
									goto end_ms;
								}
							}
						}
					}
					stat[nClues][1]++;
end_ms:
					printf("multiple solutions\n");
				}
			}
nextPuzzle:
			;
		}
		if(opt.verbose) {
			fprintf(stderr, "Clues\tSS M\tMS M\tSS NM\tMS NM\tNS\n");
			for(int i = 0; i < 81; i++) {
				if(stat[i][0] | stat[i][1] | stat[i][2] | stat[i][3] | stat[i][4]) {
					fprintf(stderr, "%d\t%d\t%d\t%d\t%d\t%d\n", i, stat[i][0], stat[i][1], stat[i][2], stat[i][3], stat[i][4]);
				}
			}
		}
		return 0;
	}
};

struct similarOptions {
	const char* puzFile;
	const char* knownsFName;
	const char* mspuzzles;
	const char* minus1plus;
	const char* invert;
	bool minimals;
	bool minus1;
	bool minus8;
	bool minus7;
	bool minus6;
	bool minus5;
	bool subcanon;
	bool twins;
	bool removeredundant;
	bool plus1;
	bool plus2;
	bool bandminlex;
	bool unique;
	bool clusterize;
	bool nosingles;
	bool cousins;
	int depth;
	int minusDepth;
	similarOptions() { //set defaults for all solver options
		puzFile = NULL;
		knownsFName = NULL;
		mspuzzles = NULL;
		minus1plus = NULL;
		invert = NULL;
		minimals = false;
		minus1 = false;
		minus8 = false;
		minus7 = false;
		minus6 = false;
		minus5 = false;
		subcanon = false;
		twins = false;
		removeredundant = false;
		plus1 = false;
		plus2 = false;
		minimals = false;
		bandminlex = false;
		unique = false;
		clusterize = false;
		nosingles = false;
		cousins = false;
		depth = -1;
		minusDepth = -1;
	}
	int go() { //collect all options and do the job
		puzFile = opt.getValue("puzzles");
		knownsFName = opt.getValue("knownpuzzles");
		mspuzzles = opt.getValue("mspuzzles");
		minus1plus = opt.getValue("minus1plus");
		invert = opt.getValue("invert");
		minimals = opt.getFlag("minimals");
		minus1 = opt.getFlag("minus1");
		minus8 = opt.getFlag("9minus8");
		minus7 = opt.getFlag("9minus7");
		minus6 = opt.getFlag("9minus6");
		minus5 = opt.getFlag("9minus5");
		subcanon = opt.getFlag("subcanon");
		twins = opt.getFlag("twins");
		removeredundant = opt.getFlag("removeredundant");
		plus1 = opt.getFlag("plus1");
		plus2 = opt.getFlag("plus2");
		bandminlex = opt.getFlag("bandminlex");
		unique = opt.getFlag("unique");
		clusterize = opt.getFlag("clusterize");
		nosingles = opt.getFlag("nosingles");
		cousins = opt.getFlag("cousins");
		const char* v = opt.getValue("relabel");
		if(v) {
			depth = atoi(v);
			if(depth <= 0) {
				cout << "Error: Relabeling depth must be a positive integer!" << endl;
				return -1;
			}
		}
		v = opt.getValue("minusandup");
		if(v) {
			minusDepth = atoi(v);
			if(minusDepth <= 0) {
				cout << "Error: Minus depth must be a positive integer!" << endl;
				return -1;
			}
		}
		return doSimilarPuzzles();
	}
};

struct nhbOptions {
	nhbOptions() { //set defaults for all solver options
	}
	int go() { //collect all solver options and do the job
		const char *gridFName, *knownsFName;
		gridFName = opt.getValue("gridfile");
		knownsFName = opt.getValue("knownpuzzles");
		bool clusterize = opt.getFlag("clusterize");
		if(clusterize)
			return uaDist();
		if(gridFName)
			return neighbourhood(gridFName, knownsFName);
		cerr << "Error: invalid neighbour search parameters." << endl;
		return -1;
	}
};

struct uaOptions {
	unsigned long long maxsolutioncount;
	unsigned int nAttempts;
	unsigned int nCells;
	unsigned int mcnUaSizeLimit;
	unsigned int maxuasize;
	unsigned int minuasize;
	unsigned int minvalency;
	bool randomSearch;
	bool digit4Search;
	bool digit5Search;
	bool unav12;
	bool unavmorph;
	bool mcnNoAutoUA;
	bool minus1;
	bool subcanon;
	bool findvalency;
	const char *gridFileName;
	const char *mspuzzles;
	uaOptions() { //set defaults for all ua manipulation options
		maxsolutioncount = 0;
		nAttempts = 10000;
		nCells = 54;
		mcnUaSizeLimit = 13;
		maxuasize = 81;
		minuasize = 0;
		minvalency = 0;
		randomSearch = false;
		digit4Search = false;
		digit5Search = false;
		unav12 = false;
		unavmorph = false;
		mcnNoAutoUA = false;
		minus1 = false;
		subcanon = false;
		findvalency = false;
		gridFileName = NULL;
		mspuzzles = NULL;
	}
	int readOptions() { //only collect all options
		//char* uafname = NULL;
		randomSearch = opt.getFlag("unavrandom");
		digit4Search = opt.getFlag("unav4");
		digit5Search = opt.getFlag("unav5");
		unav12 = opt.getFlag("unav12");
		unavmorph = opt.getFlag("unavmorph");
		mcnNoAutoUA = opt.getFlag("mcnnoautoua");
		minus1 = opt.getFlag("minus1");
		subcanon = opt.getFlag("subcanon");
		findvalency = opt.getFlag("findvalency");
		const char* v = opt.getValue("mcnuasizelimit");
		if(v) {
			mcnUaSizeLimit = atoi(v);
			if(mcnUaSizeLimit <= 0) {
				cout << "Error: Size limit must be a positive integer!" << endl;
				return -1;
			}
		}
		v = opt.getValue("attempts");
		if(v) {
			nAttempts = atoi(v);
			if(nAttempts <= 0) {
				cout << "Error: Number of attempts must be a positive integer!" << endl;
				return -1;
			}
		}
		v = opt.getValue("unknowns");
		if(v) {
			nCells = atoi(v);
			if(nCells < 20 || nCells > 70) {
				cout << "Error: Number of unknowns must be between 20 and 70!" << endl;
				return -1;
			}
		}
		v = opt.getValue("maxuasize");
		if(v) {
			maxuasize = atoi(v);
			if(maxuasize < 4) {
				cout << "Error: Maximal UA size must be at least 4!" << endl;
				return -1;
			}
		}
		v = opt.getValue("minuasize");
		if(v) {
			minuasize = atoi(v);
		}
		v = opt.getValue("minvalency");
		if(v) {
			minvalency = atoi(v);
		}
		v = opt.getValue("maxsolutioncount");
		if(v) {
			maxsolutioncount = atoi(v);
		}
		gridFileName = opt.getValue("gridfile");
		mspuzzles = opt.getValue("mspuzzles");
		return 0;
	}
	int go() { //collect all options and do the job
		int err;
		if((err = readOptions()))
			return err;
		return processUA();
	}
};

struct scanOptions {
	enum fixModes {
		exhaustive,
		fixband,
		fix1digit,
		fix2digits,
		fix3digits,
		fix4digits,
		fix1box,
		fix2boxes,
		fixauto
	};
	fixModes fixMode;
	unsigned int nClues;
	unsigned int progressSeconds;
	bool batchMode;
	const char *gridFileName;
	const char *cluemask;
	bool storepseudos;
	bool fast17;
	bool veryfast17;
	bool bandcompletions;
	bool scanfixedbandstack;
	bool scanunav;
	bool scanpuzzleset;
	bool invertpuzzleset;
	bool stopatfirst;
	bool minimalsOnly;
	bool shorter;
	scanOptions() { //set defaults for all scan options
		nClues = 17;
		progressSeconds = 60;
		batchMode = false;
		gridFileName = NULL;
		cluemask = NULL;
		fixMode = scanOptions::exhaustive;
		storepseudos = false;
		fast17 = false;
		veryfast17 = false;
		bandcompletions = false;
		scanfixedbandstack = false;
		scanunav = false;
		scanpuzzleset = false;
		invertpuzzleset = false;
		stopatfirst = false;
		minimalsOnly = false;
		shorter = false;
	}
	int go() { //collect all scan options and do the job
		//ensure the option for unavoidables are setup
		const uaOptions uaOpt;
//		if(opt.uaOpt == NULL) {
//			opt.uaOpt = &uaOpt;
//		}
		opt.uaOpt->readOptions();

		const char* v = opt.getValue("numclues");
		if(v) {
			nClues = atoi(v);
//			if(nClues < 1 || nClues > 40) {
//				cout << "Error: Number of clues must be between 1 and 40!" << endl;
//				return -1;
//			}
		}
		batchMode |= opt.getFlag("batch");
		gridFileName = opt.getValue("gridlist");
		if(gridFileName) {
			//batch mode
			batchMode = true;
		}
		else {
			//possible single grid mode
			gridFileName = opt.getValue("gridfile");
			if(NULL == gridFileName) {
				gridFileName = opt.getValue("subgridlist");
				if(NULL == gridFileName) {
					gridFileName = opt.getValue("isubgridlist");
					if(NULL != gridFileName) {
						scanpuzzleset = invertpuzzleset = true;
					}
				}
				else
					scanpuzzleset = true;
			}
		}
		cluemask = opt.getValue("cluemask");
		if(opt.getFlag("fixband"))
			fixMode = scanOptions::fixband;
		if(opt.getFlag("fix1digit"))
			fixMode = scanOptions::fix1digit;
		if(opt.getFlag("fix2digits"))
			fixMode = scanOptions::fix2digits;
		if(opt.getFlag("fix3digits"))
			fixMode = scanOptions::fix3digits;
		if(opt.getFlag("fix4digits"))
			fixMode = scanOptions::fix4digits;
		if(opt.getFlag("fix1box"))
			fixMode = scanOptions::fix1box;
		if(opt.getFlag("fix2boxes"))
			fixMode = scanOptions::fix2boxes;
		if(opt.getFlag("fixauto"))
			fixMode = scanOptions::fixauto;
		if(opt.getFlag("exhaustive"))
			fixMode = scanOptions::exhaustive;
		if(opt.getFlag("storepseudos"))
			storepseudos = true;
		fast17 = opt.getFlag("fast17");
		veryfast17 = opt.getFlag("veryfast17");
		bandcompletions = opt.getFlag("bandcompletions");
		scanfixedbandstack = opt.getFlag("scanfixedbandstack");
		scanunav = opt.getFlag("scanunav");
		stopatfirst = opt.getFlag("stopatfirst");
		minimalsOnly = opt.getFlag("minimals");
		shorter = opt.getFlag("shorter");
		if(fast17 || veryfast17) {
			//ignore most of the rest of the options
			batchMode = true;
			nClues = 17;
			fixMode = fix2digits;
			storepseudos = false;
			opt.uaOpt->digit5Search = false;
			opt.uaOpt->digit4Search = true;
			opt.uaOpt->nAttempts = 5000;
			opt.uaOpt->nCells = 54;
			opt.uaOpt->randomSearch = true;
			opt.uaOpt->maxuasize = 22;
			minimalsOnly = false;
		}
		v = opt.getValue("progressseconds");
		if(v) {
			progressSeconds = atoi(v);
			if(progressSeconds <= 0) {
				cout << "Error: Progress can be updated in positive time intervals." << endl;
				return -1;
			}
		}

		if(nClues == 0)
			return -1;
		if(opt.getFlag("fastscan")) {
			return fastScan();
		}
		return scanGridsFromFile();
	}
};

struct patternOptions {
	const char *enumerate;
	const char *fixclues;
	const char *scanfor;
	const char *settle;
	const char *pg;
	bool statistics;
	bool subcanon;
	bool redundancy;
	bool patcanon;
	patternOptions() { //set defaults for all scan options
		enumerate = NULL;
		fixclues = NULL;
		scanfor = NULL;
		settle = NULL;
		pg = NULL;
		statistics = false;
		subcanon = false;
		redundancy = false;
		patcanon = false;
	}
	int go() { //collect all pattern options and do the job
		enumerate = opt.getValue("enumerate");
		fixclues = opt.getValue("fixclues");
		//ensure the option for unavoidables are setup
		scanfor = opt.getValue("scanfor");
		settle = opt.getValue("settle");
		pg = opt.getValue("pg");
		statistics = opt.getFlag("statistics");
		subcanon = opt.getFlag("subcanon");
		redundancy = opt.getFlag("redundancy");
		patcanon = opt.getFlag("patcanon");

		return processPatterns();
	}
};

struct templateOptions {
	bool get2templates;
	bool get2rookeries;
	bool get999911110;
	bool r4tot4;
	bool r4tot5;
	templateOptions() { //set defaults for all template options
		get2templates = false;
		get2rookeries = false;
		get999911110 = false;
		r4tot4 = false;
		r4tot5 = false;
	}
	int go() { //collect all options and do the job
		get2templates = opt.getFlag("get2templates");
		get2rookeries = opt.getFlag("get2rookeries");
		get999911110 = opt.getFlag("get999911110");
		r4tot4 = opt.getFlag("r4tot4");
		r4tot5 = opt.getFlag("r4tot5");
		return processTemplate();
	}
};

#endif // OPTIONS_H_INCLUDED
