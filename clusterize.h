#ifndef CLUSTERIZE_H_INCLUDED

#define CLUSTERIZE_H_INCLUDED

#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <map>
#include <set>
//#include <iostream>
#include "ch81.h"

using namespace std;

struct ch81RefList;

struct clSizes : public map<int, int> {};

struct ch81RefList : public set<ch81RefList*> {
	const ch81 * cluster;
	ch81RefList() {
		cluster = NULL;
	}
};
struct clusters : public map<ch81, puzzleSet> {};

struct ch81WithReferences : public map<ch81, ch81RefList> {};

void setCluster (const ch81 * theCluster, ch81RefList &references) {
	for(ch81RefList::iterator p1 = references.begin(); p1 != references.end(); p1++) {
		if((*p1)->cluster == NULL) {
			(*p1)->cluster = theCluster;
			setCluster(theCluster, **p1);
		}
	}
}

void clusterize(int sz) {
	struct adder { //just a macro
		static void add(const ch81& puz, ch81WithReferences& m1, ch81WithReferences::iterator& p) {
			ch81 puzCanon;
			subcanon(puz.chars, puzCanon.chars);
			ch81RefList &puzInChildren = m1[puzCanon]; //add canonicalized puzzle in children
			puzInChildren.insert(&(p->second)); //add reference to source
			p->second.insert(&puzInChildren); //in source, add reference to child
		}
	};
	puzzleSet m0; //given list, using old code to read, then copied to list.
	ch81WithReferences list, m1; //given & minusN list
	clusters cl; //clusters' details
	clSizes cs; //cluster sizes

	if(sz < 1 || sz > 6) return; //can't handle this
	
	//load the whole list
	m0.loadFromFile(stdin); //load puzzles
	for(puzzleSet::const_iterator p = m0.begin(); p != m0.end(); p++) {
		list[*p]; //add keys with empty lists with references to {-N} children
	}
	m0.clear();

	//first pass: do {-1} and store references
	for(ch81WithReferences::iterator p = list.begin(); p != list.end(); p++) {
		ch81 puz = p->first; //structure copy
		const char* pp = p->first.chars;
		//apply {-1}
		for(int i = 0; i < 81; i++) {
			if(puz.chars[i] == 0) continue;
			puz.chars[i] = 0; //clear the given
			if(sz == 1) {
				adder::add(puz, m1, p);
			}
			else {
				//apply {-2}
				for(int j = i + 1; j < 81; j++) {
					if(puz.chars[j] == 0) continue;
					puz.chars[j] = 0;
					if(sz == 2) {
						adder::add(puz, m1, p);
					}
					else {
						//apply {-3}
						for(int k = j + 1; k < 81; k++) {
							if(puz.chars[k] == 0) continue;
							puz.chars[k] = 0;
							if(sz == 3) {
								adder::add(puz, m1, p);
							}
							else {
								//apply {-4}
								for(int l = k + 1; l < 81; l++) {
									if(puz.chars[l] == 0) continue;
									puz.chars[l] = 0;
									if(sz == 4) {
										adder::add(puz, m1, p);
									}
									else {
										//apply {-5}
										for(int m = l + 1; m < 81; m++) {
											if(puz.chars[m] == 0) continue;
											puz.chars[m] = 0;
											if(sz == 5) {
												adder::add(puz, m1, p);
											}
											else {
												//apply {-6}
												for(int n = m + 1; n < 81; n++) {
													if(puz.chars[n] == 0) continue;
													puz.chars[n] = 0;
													//if(sz == 6) {
														adder::add(puz, m1, p);
													//}
													//else {
													//	adder::add(puz, m1, p);
													//}
													puz.chars[n] = pp[n]; // restore the {-6} given
												}
												puz.chars[m] = pp[m]; // restore the {-5} given
											}
										}
										puz.chars[l] = pp[l]; // restore the {-4} given
									}
								}
								puz.chars[k] = pp[k]; // restore the {-3} given
							}
						}
						puz.chars[j] = pp[j]; // restore the {-2} given
					}
				}
				puz.chars[i] = pp[i]; //restore the {-1} given
			} //sz > 1
		} //for i
	} //for p
	printf("pass {-%d}, src=%d, children=%d\n", sz, (int)list.size(), (int)m1.size());
	//second pass: obtain clusters
	for(ch81WithReferences::iterator p = m1.begin(); p != m1.end(); p++) {
		ch81RefList &parents = p->second;
		const ch81 *theCluster = parents.cluster;
		if(theCluster == NULL) {
			theCluster = &(p->first);
			setCluster(theCluster, parents);
		}
	}
	//third pass: order clusters
	for(ch81WithReferences::const_iterator p = list.begin(); p != list.end(); p++) {
		puzzleSet &ps = cl[*(p->second.cluster)];
		ps.insert(p->first);
	}
	printf("%d clusters.\n", (int)cl.size());
	//fourth pass: output
	for(clusters::const_iterator p = cl.begin(); p != cl.end(); p++) {
		char buf[81];
		p->first.toString(buf);
		printf("\t%81.81s\t=%d\n", buf, (int)p->second.size());
		cs[(int)p->second.size()]++;
		for(puzzleSet::const_iterator pp = p->second.begin(); pp != p->second.end(); pp++) {
			pp->toString(buf);
			printf("%81.81s\n", buf);
		}
	}
	//fifth pass: output sizes
	printf("\n#clust\t#puz\n");
	for(clSizes::const_iterator p = cs.begin(); p != cs.end(); p++) {
		printf("%d\t%d\n", p->second, p->first);
	}
	cl.clear();
	cs.clear();
	printf("done\n");
}


#endif // CLUSTERIZE_H_INCLUDED
