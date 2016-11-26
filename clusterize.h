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

void clusterize () {
	puzzleSet m0; //given list, using old code to read, then copied to list.
	ch81WithReferences list, m1; //given & minusN list
	clusters cl; //clusters' details
	clSizes cs; //cluster sizes
	
	//load the whole list
	m0.loadFromFile(stdin); //load puzzles
	for(puzzleSet::const_iterator p = m0.begin(); p != m0.end(); p++) {
		list[*p]; //add keys with empty lists with references to {-N} children
	}
	m0.clear();

	//first pass: do {-1} and store references
	for(ch81WithReferences::iterator p = list.begin(); p != list.end(); p++) {
		ch81 puz = p->first; //structure copy
		int t;
		for(int i = 0; i < 81; i++) {
			//apply {-1}
			if(0 != (t = puz.chars[i])) {
				puz.chars[i] = 0; //clear the given
				//apply {-2}
				for(int j = i + 1; j < 81; j++) {
					int tt = puz.chars[j];
					if(tt) {
						puz.chars[j] = 0;
						//apply {-3}
						for(int k = j + 1; k < 81; k++) {
							int ttt = puz.chars[k];
							if(ttt) {
								puz.chars[k] = 0;
								//apply {-4}
								for(int l = k + 1; l < 81; l++) {
									int tttt = puz.chars[l];
									if(tttt) {
										puz.chars[l] = 0;
										//apply {-5}
										for(int m = l + 1; m < 81; m++) {
											int ttttt = puz.chars[m];
											if(ttttt) {
												puz.chars[m] = 0;
												/////
												ch81 puzCanon;
												subcanon(puz.chars, puzCanon.chars);
												ch81RefList &puzInChildren = m1[puzCanon]; //add canonicalized puzzle in children
												puzInChildren.insert(&(p->second)); //add reference to source
												p->second.insert(&puzInChildren); //in source, add reference to child
												/////
												puz.chars[m] = ttttt; // restore the {-5} given
											}
										}
										puz.chars[l] = tttt; // restore the {-4} given
									}
								}
								puz.chars[k] = ttt; // restore the {-3} given
							}
						}
						puz.chars[j] = tt; // restore the {-2} given
					}
				}
				puz.chars[i] = t; //restore the {-1} given
			}
		}
	}
	printf("pass {-4}, src=%d, children=%d\n", (int)list.size(), (int)m1.size());
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
