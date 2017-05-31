#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <time.h>
#include "options.h"

const char *versionString = "GridChecker v1.28 (2017-05-13)";

//#define DIRTY_TEST

#ifdef DIRTY_TEST
extern void test(); //debug
#endif //DIRTY_TEST

int main(int argc, char* argv[])
{

	clock_t start, finish;
	int ret;
	start = clock();
#ifdef DIRTY_TEST
	test();
	ret = 0;
#else
	if(opt.read(argc, argv))
		return 0;
	ret = opt.execCommand();
#endif //DIRTY_TEST
	finish = clock();
	fprintf(stderr, "\nTotal time %2.3f seconds.\n", (double)(finish - start) / CLOCKS_PER_SEC);
	return ret;
}

#ifdef DIRTY_TEST
//#include "solver.h"
//extern void solverXXL(const char* in);
//extern void solverXXL1(const char* in);
//extern void test() {
//	char buf[2000];
//	while(fgets(buf, sizeof(buf), stdin)) {
//		//printf("%81.81s\t", buf);
//		//for(int i = 0; i < 81; i++) {
//		//	buf[i] -= '0';
//		//	if(buf[i] <= 0 || buf[i] > 9) {
//		//		buf[i] = 0;
//		//	}
//		//}
//		solverXXL1(buf);
//		//printf("\n");
//	}
//}

//#include "rowminlex.h"
//#include "ch81.h"
//#include "uset.h"
//extern void test() {
//	char buf[2000];
//	lightweightUsetList res;
//	ch81 inPattern, outPattern;
//	bm128 canBitmap;
//	while(fgets(buf, sizeof(buf), stdin)) {
//		inPattern.patternFromString(buf);
//		subcanon(inPattern.chars, outPattern.chars);
//		canBitmap.clear();
//		for(int i = 0; i < 81; i++) {
//			if(outPattern.chars[i]) {
//				canBitmap.setBit(81 - i);
//			}
//		}
//		res.insert(canBitmap);
//	}
//	//print the sorted unique patterns
//	for(lightweightUsetList::const_iterator p = res.begin(); p != res.end(); p++) {
//		for(int i = 0; i < 81; i++) {
//			outPattern.chars[i] = ((p->isBitSet(81 - i)) ? '1' : '.');
//		}
//		printf("%81.81s\n", outPattern.chars);
//	}
//}

//#include "rowminlex.h"
//#include "ch81.h"
//extern void test() {
//	char buf[2000];
//	ch81 inPattern, outPattern;
//	while(fgets(buf, sizeof(buf), stdin)) {
//		inPattern.patternFromString(buf);
//		subcanon(inPattern.chars, outPattern.chars);
//		outPattern.toString(buf);
//		printf("%81.81s\n", buf);
//	}
//}

#endif //DIRTY_TEST
