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

struct patternOptions;

class options {
	AnyOption anyopt;
	time_t startTime;
public:
	patternOptions *patternOpt;
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

struct patternOptions {
	const char *enumerate;
	const char *fixclues;
	const char *pg;
	bool statistics;
	bool subcanon;
	bool redundancy;
	patternOptions() { //set defaults for all scan options
		enumerate = NULL;
		fixclues = NULL;
		statistics = false;
		subcanon = false;
		redundancy = false;
	}
	int go() { //collect all pattern options and do the job
		enumerate = opt.getValue("enumerate");
		fixclues = opt.getValue("fixclues");
		//ensure the option for unavoidables are setup
		pg = opt.getValue("pg");
		statistics = opt.getFlag("statistics");
		subcanon = opt.getFlag("subcanon");
		redundancy = opt.getFlag("redundancy");

		return processPatterns();
	}
};

#endif // OPTIONS_H_INCLUDED
