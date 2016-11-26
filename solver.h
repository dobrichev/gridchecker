#ifndef SOLVER_H_INCLUDED

#define SOLVER_H_INCLUDED

#include "tables.h"
#include "grid.h"

extern unsigned long long solve(const char* in, const unsigned long long maxSolutions);
extern unsigned long long solve(const char* in, const unsigned long long maxSolutions, char* out);
extern unsigned long long solve(const int* gridBM, const char* in, const unsigned long long maxSolutions);
extern unsigned long long solve(const int* gridBM, const char* in, const unsigned long long maxSolutions, char* out);
extern unsigned long long solve(const int* gridBM, const char* in, uaCollector *theCollector);
extern unsigned long long solve(const char* in, int* pencilMarks);

extern double solverRate(const char* in);
extern int solverBackdoor(char* in, const bool verbose);

extern void findPencilMarks(char* in, short* pencilMarks, short *pmHints = NULL);
extern bool hasEDSolution(const int* gridBM, const char* in);

extern int solverPlus(char* in, const int maxPuzzles, char* out, unsigned long long *nSol);
extern int solverPlus1(const char* in, char* out, bool redundancyCheck = true, bool unique = false);
extern int solverPlus2(char* in, char* out);
extern int solverPlus1Unique(char* in, char* out);

extern unsigned long long solverIsIrreducibleBySolCount(char *puz);
extern int solverIsIrreducibleByProbing(char *puz);
extern int solverIsIrreducible(char *puz);

extern int solverPattern(char* in, const int nClues, const int* cluePositions, int (*puzFound)(void *context, const char* puz));
extern int solverRelabel(const char* in, const int maxDiff, const bool minimals, const bool unique, const bool nosingles, int (*callBack)(const char *puz, void *context), void *context);

extern void digit2bitmap(const char* in, int* out);

#endif // SOLVER_H_INCLUDED
