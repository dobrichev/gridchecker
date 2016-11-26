#if 1
#include "skfr/ratingengine.h"

struct fskfr {
	//buffers
	static const int bufSize = 128 * 6;
	skfr::puzzleToRate puzzlesToRate[bufSize];
	unsigned int *res[bufSize]; //where the compressed result goes (ED,EP,ER,0)
	int count;
	//constructor
	fskfr();
	//add to buffer
	void skfrMultiER(const char *p, unsigned int *res);
	//rate partially populated buffer
	void skfrCommit();
	//rate single puzzle
	//int skfr::skfrER(const char *p) const;
};
#endif
