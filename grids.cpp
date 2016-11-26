#include "grids.h"
#include "solver.h"
#include "rowminlex.h"

void gridsWithPuzzles::addPuzzle(const char *p) {
	ch81 tp, tg;
	char pp[2][81];
	char buf[100];
	if(solve(p, 2, pp[0]) != 1) {
		ch81 pbuf;
		pbuf = *(ch81*)p;
		pbuf.toString(buf);
		printf("Multisolution puzzle: %81.81s\n", buf);
		return; //wrong puzzle
	}
	transformer tr;
	tr.byGrid(pp[0]);
	//if(tr.aut > 1) {
	//	ch81 pbuf;
	//	pbuf = *(ch81*)pp[0];
	//	pbuf.toString(buf);
	//	printf("Automorphic grid: (%d) %81.81s\n", tr.aut, buf);
	//}
	tr.transform(pp[0], tg.chars); //the grid
	tr.transform(p, tp.chars); //the puzzle
	puzzlesInGrid &pg = (*this)[tg];
	if(pg.count(tp)) {
		tp.toString(buf);
		printf("Duplicate puzzle: %81.81s\n", buf);
	}
	pg.insert(tp);
}

void gridsWithPuzzles::loadFromFile(const char *fname) {
	FILE *file;
	file = fopen(fname, "rt");
	if (! file)
		return;
	loadFromFile(file);
	fclose(file);
}

void gridsWithPuzzles::loadFromFile(FILE *file) {
	char buf[1000];
	while(fgets(buf, sizeof(buf), file)) {
		ch81 puz;
		puz.fromString(buf);
		addPuzzle(puz.chars);
	}
}

void gridsWithPuzzles::saveToFile(FILE * file, const bool noPuz, const bool verbose) {
	char buf[82];
	ch81 gIntersection, gUnion;
	buf[81] = 0;
	for(const_iterator g = begin(); g != end(); g++) {
		g->first.toString(buf);
		fprintf(file, "%81.81s", buf);
		if(!noPuz) {
			fprintf(file, "\t%d", (int)(g->second.size()));
			if(verbose) {
				for(int i = 0; i < 81; i++) {
					gIntersection.chars[i] = g->first.chars[i];
					gUnion.chars[i] = 0;
				}
			}
			for(puzzlesInGrid::const_iterator p = g->second.begin(); p != g->second.end(); p++) {
				p->toString(buf);
				fprintf(file, "\t%81.81s", buf);
				if(verbose) {
					for(int i = 0; i < 81; i++) {
						gIntersection.chars[i] &= p->chars[i];
						gUnion.chars[i] |= p->chars[i];
					}
				}
			}
		}
		fprintf(file, "\n");
		if(verbose) {
			gIntersection.toString(buf);
			fprintf(file, "\t%81.81s\tintersection\n", buf);
			gUnion.toString(buf);
			fprintf(file, "\t%81.81s\tunion\n", buf);
		}
	}
}

void gridsWithPuzzles::reindexBySize() {
	indexBySize.clear();
	for(const_iterator g = begin(); g != end(); g++) {
		indexBySize[(int)g->second.size()].insert((void*)&(g->first));
	}
}

extern int groupBySolution(const bool noPuz, const bool verbose) {
	gridsWithPuzzles gp;
	gp.loadFromFile(stdin);
	gp.saveToFile(stdout, noPuz, verbose);
	return 0;
}
