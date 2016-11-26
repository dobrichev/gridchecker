#ifndef CH81_H_INCLUDED

#define CH81_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <memory.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <map>
#include "rowminlex.h"

using namespace std;

struct ch81 {
	char chars[81];
	bool operator < (const ch81 &rhs) const {
		return (memcmp(this, &rhs, sizeof(ch81)) < 0);
	}
	static int toString(const void* c, char *p) {
		int nClues = 0;
		for(int i = 0; i < 81; i++) {
			if(((const char*)c)[i]) {
				p[i] = ((const char*)c)[i] + '0';
				nClues++;
			}
			else
				p[i] = '.';
		}
		//p[81] = 0;
		return nClues;
	}
	int toString(char *p) const {
		int nClues = 0;
		for(int i = 0; i < 81; i++) {
			if(chars[i]) {
				p[i] = chars[i] + '0';
				nClues++;
			}
			else
				p[i] = '.';
		}
		//p[81] = 0;
		return nClues;
	}
	int fromString(const char *p) {
		int nClues = 81;
		for(int i = 0; i < 81; i++) {
			chars[i] = p[i] - '0';
			if(chars[i] <= 0 || chars[i] > 9) {
				chars[i] = 0;
				nClues--;
			}
		}
		return nClues;
	}
	int patternFromString(const char *p) {
		int nClues = 81;
		for(int i = 0; i < 81; i++) {
			chars[i] = ((p[i] >= '1' && p[i] <= '9') ? 1 : (nClues--, 0));
		}
		return nClues;
	}
	void clear() {
		memset(chars, 0, 81);
	}
};

typedef pair <ch81, int> Ch81_Int_Pair;
struct puzzleSetTagInt : public map<ch81, int> {
	void loadFromFile(FILE *f, const bool canonicalize = true, const int maxSize = 100) {
		char buf[1000];
		puzzleSetTagInt::size_type n = 0;
		int v;
		while(fgets(buf, sizeof(buf), f)) {
			ch81 puz, puzCanon;
			buf[100] = 0;
			int puzSize = puz.fromString(buf);
			sscanf(buf + 82, "%d", &v);
			if(puzSize > maxSize)
				continue;
			n++;
			if(canonicalize) {
				subcanon(puz.chars, puzCanon.chars);
				//if(find(puzCanon) != end()) {
				//	puzCanon.toString(puz.chars);
				//	printf("Duplicate puzzle: %81.81s\n", puz.chars);
				//}
				//else {
				//	insert(puzCanon);
				//}
				insert(Ch81_Int_Pair(puzCanon, v));
			}
			else {
				insert(Ch81_Int_Pair(puz, v));
			}
		}
		n -= size();
		if(n) {
			printf("%d duplicates found.\n", (int)n);
		}
	}
	void loadFromFile(const char *pFName, const bool canonicalize = true) {
		FILE *pfile;
		if(pFName == NULL)
			return;
		pfile = fopen(pFName, "rt");
		if(pfile == NULL) {
			printf("error: Can't open file %s", pFName);
			return;
		}
		loadFromFile(pfile, canonicalize);
		fclose(pfile);
	}
	void saveToFile(FILE *pfile) const {
		if(pfile == NULL) {
			return;
		}
		for(const_iterator p = begin(); p != end(); p++) {
			ch81 pText;
			p->first.toString(pText.chars);
			fprintf(pfile, "%81.81s\t%d\n", pText.chars, p->second);
		}
	}
	void appendToFile(const char *pFName) const {
		FILE *pfile;
		if(pFName == NULL)
			return;
		pfile = fopen(pFName, "at");
		if(pfile == NULL) {
			printf("error: Can't open file %s", pFName);
			return;
		}
		saveToFile(pfile);
		fclose(pfile);
	}
	void saveToFile(const char *pFName) const {
		FILE *pfile;
		if(pFName == NULL)
			return;
		pfile = fopen(pFName, "wt");
		if(pfile == NULL) {
			printf("error: Can't open file %s", pFName);
			return;
		}
		saveToFile(pfile);
		fclose(pfile);
	}
};


struct puzzleSet : public set<ch81> {
	void loadFromFile(FILE *f, const bool canonicalize = true, const int maxSize = 100) {
		char buf[1000];
		puzzleSet::size_type n = 0;
		while(fgets(buf, sizeof(buf), f)) {
			ch81 puz, puzCanon;
			int puzSize = puz.fromString(buf);
			if(puzSize > maxSize)
				continue;
			n++;
			if(canonicalize) {
				if(puzSize == 81) { //complete grid
					transformer tr;
					tr.byGrid(puz.chars);
					ch81 digitsCanon;
					tr.transform(puz.chars, puzCanon.chars);
				}
				else {
					subcanon(puz.chars, puzCanon.chars);
				}
				//if(find(puzCanon) != end()) {
				//	puzCanon.toString(puz.chars);
				//	printf("Duplicate puzzle: %81.81s\n", puz.chars);
				//}
				//else {
				//	insert(puzCanon);
				//}
				insert(puzCanon);
			}
			else {
				insert(puz);
			}
		}
		n -= size();
		if(n) {
			printf("%d duplicates found.\n", (int)n);
		}
	}
	void loadFromFile(const char *pFName, const bool canonicalize = true) {
		FILE *pfile;
		if(pFName == NULL)
			return;
		pfile = fopen(pFName, "rt");
		if(pfile == NULL) {
			printf("error: Can't open file %s", pFName);
			return;
		}
		loadFromFile(pfile, canonicalize);
		fclose(pfile);
	}
	void saveToFile(FILE *pfile) const {
		if(pfile == NULL) {
			return;
		}
		for(const_iterator p = begin(); p != end(); p++) {
			ch81 pText;
			p->toString(pText.chars);
			fprintf(pfile, "%81.81s\n", pText.chars);
		}
	}
	void appendToFile(const char *pFName) const {
		FILE *pfile;
		if(pFName == NULL)
			return;
		pfile = fopen(pFName, "at");
		if(pfile == NULL) {
			printf("error: Can't open file %s", pFName);
			return;
		}
		saveToFile(pfile);
		fclose(pfile);
	}
	void saveToFile(const char *pFName) const {
		FILE *pfile;
		if(pFName == NULL)
			return;
		pfile = fopen(pFName, "wt");
		if(pfile == NULL) {
			printf("error: Can't open file %s", pFName);
			return;
		}
		saveToFile(pfile);
		fclose(pfile);
	}
};


#endif // CH81_H_INCLUDED
