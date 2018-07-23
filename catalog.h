#ifndef CATALOG_H_INCLUDED

#define CATALOG_H_INCLUDED

#include <stdio.h>

//typedef unsigned int u32;
//typedef unsigned long long u64;

//typedef union {
//	u32 u32[2];
//	u64 u64;
//} u64data;

//typedef struct {
//	u64data data[36][50][44][40][35]; //U4,U61,U62,U63,U64
//} ua46stat;

//struct grid {
//	char givens[81];
//};

extern int comprGrid(const char *g, char *cg);
extern int uncomprGrid(char *g, const char *cg);
//extern void ua46statAdd(ua46stat *t, const int *vector, const unsigned long long representative);
//extern int ua46statGet(const ua46stat *t, const int u4, const int u61, const int u62, const int u63, const int u64, unsigned long long *representative);
//extern int gridByIndex(FILE *catalog, const unsigned long long index, char *g);
extern int processCatalog();

#endif //CATALOG_H_INCLUDED
