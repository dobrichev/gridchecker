#ifndef CH81_H_INCLUDED

#define CH81_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <memory.h>
#include <stdio.h>
//#include <vector>
#include <set>
//#include <map>

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


struct puzzleSet : public std::set<ch81> {};


#endif // CH81_H_INCLUDED
