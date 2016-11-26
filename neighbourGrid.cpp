#include "solver.h"
#include "grid.h"
#include "neighbourGrid.h"
#include "patterns.h"

neighbourGrid::neighbourGrid(void) {
	minWormholeSize = 99;
	maxWormholeSize = 0;
}

neighbourGrids::neighbourGrids(void) {
	maxUA = 0;
	nMaxUA = 0;
}

void neighbourGrids::write(ostream &o) {
	o << "NumGrids=" << size();
	o << endl;
	o << "maxUA=" << maxUA;
	o << endl;
	o << "nMaxUA=" << nMaxUA;
	o << endl;
	for(const_iterator ngIter = begin(); ngIter != end(); ngIter++) {
		const lightweightUsetList &usl = ngIter->second.us;
		//if(usl.size() < maxUA) continue;
		char gr[100];
		grid::toString(ngIter->first.chars, gr);
		o.write(gr, 81);
		o << endl;
		for(lightweightUsetList::const_iterator usi = usl.begin(); usi != usl.end(); usi++) {
			uset u = *usi;
			u.positionsByBitmap();
			char uss[500];
			u.toString(uss);
			o << "\t";
			o << uss;
			o << endl;
		}
	}
}

void neighbourGrids::findNeighbourGrids(grid &g) {
	const int maxSolutions = 20;
	//char buf[100];
	//g.toString(buf); //source grid
	//printf("Grid: %s\n", buf);
	//printf("Examining Neighbour Grids.\n");
	transformer myselfTr;
	myselfTr.byGrid(g.digits);
	myselfTr.transform(g.digits, digitsCanon.chars);
	authoMorphisms = myselfTr.aut;
	for(usetListBySize::iterator s = g.usetsBySize.begin(); s != g.usetsBySize.end();) {
		char uatext[256];
		char puz[81];
		unsigned long long nSol;
		s->toString(uatext);
		char sbuf[maxSolutions][81];
		g.ua2puzzle(*s, puz);
		nSol = solve(g.gridBM, puz, maxSolutions, sbuf[0]);
		for(int i = 1; i < nSol; i++) {
			int nCanSol = 0;
			ch81 solCanon;
			transformer tr;
			tr.byGrid(sbuf[i]);
			tr.transform(sbuf[i], solCanon.chars);
			if(memcmp(solCanon.chars, digitsCanon.chars, 81)) { //new essentially different solution
				neighbourGrid &ng = this->operator [](solCanon);
				bm128 ls;
				ls.bitmap128 = s->bitmap128;
				ng.us.insert(ls);
				if(ng.maxWormholeSize < s->nbits)
					ng.maxWormholeSize = s->nbits;
				if(ng.minWormholeSize > s->nbits)
					ng.minWormholeSize = s->nbits;
				unsigned int newSize = (unsigned int)ng.us.size();
				if(maxUA < newSize) {
					maxUA = newSize;
					nMaxUA = 1;
				}
				else if(maxUA == newSize) {
					nMaxUA++;
				}
			}
			else { //isomorph to myself
				myself.us.insert(*s);
				if(myself.maxWormholeSize < s->nbits)
					myself.maxWormholeSize = s->nbits;
				if(myself.minWormholeSize > s->nbits)
					myself.minWormholeSize = s->nbits;
			}
		}
        usetListBySize::iterator ss = s;
        s++;
		g.usetsBySize.erase(ss);
	}
}

extern int neighbourhoodDetails(const char* fname, const char* knownsfname);

extern int neighbourhood(const char* fname, const char* knownsfname) {
	return neighbourhoodDetails(fname, knownsfname);
	grid g;
	g.fname = fname;
	if(g.readFromFile())
		return -1;
	g.readUAFromFile();

	neighbourGrids ngList;
	ngList.findNeighbourGrids(g);

	if(knownsfname == NULL) {
		ngList.write(cout);
		return 0;
	}

	gridsWithPuzzles gp;
	gp.loadFromFile(knownsfname);
	//gp.reindexBySize();

	char buf[1000];
	gridsWithPuzzles::const_iterator puz;
	puzzlesInGrid::const_iterator p;
	int nKnowns = 0;
	int maxMaxDist = 0;
	int minMinDist = 99;
	cout << "grid, Known Puzzles, Num Wormholes, Min Hamming Distance, Max Hamming Distance" << endl;
	//myself
	cout << endl << "Base grid (" << ngList.authoMorphisms << "):" << endl;
	puz = gp.find(ngList.digitsCanon);
	ngList.digitsCanon.toString(buf);
	cout.width(81);
	cout << buf;
	if(puz != gp.end()) {
		cout << "\t" << puz->second.size() << "\t" << ngList.myself.us.size() << "\t" << ngList.myself.minWormholeSize << "\t" << ngList.myself.maxWormholeSize << endl;
		//write known puzzles for the original grid
		for(p = puz->second.begin(); p != puz->second.end(); p++) {
			p->toString(buf);
			cout << "\t" << buf << endl;
		}
	}
	else {
		cout << "\t0\t" << ngList.myself.us.size() << "\t" << ngList.myself.minWormholeSize << "\t" << ngList.myself.maxWormholeSize << endl;
	}
	cout << endl << "Neighbours with known puzzles:" << endl;
	for(neighbourGrids::const_iterator ng = ngList.begin(); ng != ngList.end(); ng++) {
		puz = gp.find(ng->first);
		if(puz != gp.end()) {
			//known puzzle count
			ng->first.toString(buf);
			cout.width(81);
			cout << buf;
			cout << "\t" << puz->second.size() << "\t" << ng->second.us.size() << "\t" << ng->second.minWormholeSize << "\t" << ng->second.maxWormholeSize << endl;
			nKnowns++;
			if(maxMaxDist < ng->second.maxWormholeSize)
				maxMaxDist = ng->second.maxWormholeSize;
			if(minMinDist > ng->second.minWormholeSize)
				minMinDist = ng->second.minWormholeSize;
			//write known puzzles for the neighbour
			for(p = puz->second.begin(); p != puz->second.end(); p++) {
				p->toString(buf);
				cout << "\t" << buf << endl;
			}
			cout << endl;
			//write UA sets as pseudopuzzles
			for(lightweightUsetList::const_iterator u = ng->second.us.begin(); u != ng->second.us.end(); u++) {
				//uset us = *u;
				//us.positionsByBitmap();
				int s = u->toPseudoPuzzleString(ng->first.chars, buf);
				cout << "\t" << buf << endl;
			}
		}
	}
	cout << endl << "Neighbours without known puzzles:" << endl;
	for(neighbourGrids::const_iterator ng = ngList.begin(); ng != ngList.end(); ng++) {
		puz = gp.find(ng->first);
		if(puz == gp.end()) {
			//unknown puzzle count
			ng->first.toString(buf);
			cout.width(81);
			cout << buf;
			cout << "\t0\t" << ng->second.us.size() << "\t" << ng->second.minWormholeSize << "\t" << ng->second.maxWormholeSize << endl;
			if(maxMaxDist < ng->second.maxWormholeSize)
				maxMaxDist = ng->second.maxWormholeSize;
			if(minMinDist > ng->second.minWormholeSize)
				minMinDist = ng->second.minWormholeSize;
		}
	}
	//ngList.write(cout);
	cout << endl << "Total: " << ngList.size() << ", including " << nKnowns << " with puzzles and " << ngList.size() - nKnowns << " without (1:" << (nKnowns ? (ngList.size() - nKnowns) / nKnowns : 0) << ")." << endl;
	cout << "Max Wormholes = " << ngList.maxUA << ", " << ngList.nMaxUA << " occurences." << endl;
	cout << "Min Hamming Distance = " << minMinDist << ", Max Hamming Distance = " << maxMaxDist << endl;
	return 0;
}

//void r2m1p1(const gridsWithPuzzles &knowns, const gridsWithPuzzles &forTesting);

int neighbourhoodDetails(const char* fname, const char* knownsfname) {
	//printf("Examining Neighbour Grids.\n");
	gridsWithPuzzles gp;
	gp.loadFromFile(knownsfname);
	//gp.reindexBySize();

	//r2m1p1(gp, gp); //debug
	//return 0;

	char buf[3000];
	strcpy(buf, fname);
	strcat(buf, ".neighbours.txt");
	ofstream ofile(buf);
	if(! ofile)
		return -1;
	grid g;
	g.fname = fname;
	if(g.readFromFile())
		return -1;
	strcpy(buf, fname);
	strcat(buf, g.fileUASuffix);
	ifstream uafile(buf);
	if(! uafile)
		return -1;
	g.toString(buf); //source grid
	printf("Grid:\t%s\n", buf);
	transformer myselfTr;
	myselfTr.byGrid(g.digits);
	ch81 digitsCanon;
	myselfTr.transform(g.digits, digitsCanon.chars);
	unsigned int authoMorphisms = myselfTr.aut;
	bool mustTransform = myselfTr.isTransforming();
	if(mustTransform) {
		//memcpy(g.digits, digitsCanon.chars, 81);
		digit2bitmap(digitsCanon.chars, g.gridBM);
		//g.toString(buf);
		//printf("Minlex:\t%81.81s\n", buf);
		digitsCanon.toString(buf);
		printf("Minlex:\t%81.81s\n", buf);
	}
	gridsWithPuzzles::const_iterator known;
	puzzlesInGrid::const_iterator p;
	const puzzlesInGrid *basePuzzles = 0;
	//obtain known puzzles of the original grid
	known = gp.find(digitsCanon);
	if(known != gp.end()) {
		basePuzzles = &known->second;
		for(p = basePuzzles->begin(); p != basePuzzles->end(); p++) {
			p->toString(buf);
			printf("Puz:\t%81.81s\n", buf);
		}
	}
	while(uafile.getline(buf, sizeof(buf))) {
		static const int maxSolutions = 20;
		unsigned long long nSol;
		ch81 puz;
		ch81 *ppuz = (ch81*)buf;
		ch81 uaText;
		uset u;
		u.fromString(buf);
		g.ua2puzzle(u, buf);
		if(mustTransform) {
			myselfTr.transform(buf, puz.chars);
			ppuz = &puz;
		}
		ppuz->toString(uaText.chars);
		//printf("\nUA:\t%81.81s\t%d\n", uaText.chars, u.nbits);
		ch81 sbuf[maxSolutions];
		nSol = solve(g.gridBM, ppuz->chars, maxSolutions, sbuf[0].chars);
		for(unsigned long long i = 1; i < nSol; i++) { //skip the original solution
			ch81 solCanon;
			ch81 solText;
			transformer tr;
			tr.byGrid(sbuf[i].chars);
			tr.transform(sbuf[i].chars, solCanon.chars);
			if(memcmp(solCanon.chars, digitsCanon.chars, 81)) { //new essentially different solution
				sbuf[i].toString(solText.chars);
				known = gp.find(solCanon);
				if(known != gp.end()) {
					printf("\nUA:\t%81.81s\t%d\n", uaText.chars, u.nbits);
					printf("NGrid:\t%81.81s\t%d\t%d\n", solText.chars, (int)known->second.size(), tr.aut);
				}
				else {
					continue;
					printf("\nUA:\t%81.81s\t%d\n", uaText.chars, u.nbits);
					printf("NGrid:\t%81.81s\t%d\t%d\n", solText.chars, 0, tr.aut);
				}
				if(basePuzzles) { //examining a grid with knowns
					for(p = basePuzzles->begin(); p != basePuzzles->end(); p++) {
						int n = 0;
						for(int i = 0; i < 81; i++) {
							if(p->chars[i] && !ppuz->chars[i]) {
								n++;
							}
						}
						ch81 str;
						p->toString(str.chars);
						printf("Puz:\t%81.81s\t%d\n", str.chars, n);
					}
				}
				if(known != gp.end()) {
					for(p = known->second.begin(); p != known->second.end(); p++) {
						ch81 newTransformedPuz;
						tr.reverseTransform(p->chars, newTransformedPuz.chars);
						int n = 0;
						for(int i = 0; i < 81; i++) {
							if(newTransformedPuz.chars[i] && !ppuz->chars[i]) {
								n++;
							}
						}
						ch81 str;
						newTransformedPuz.toString(str.chars);
						printf("NPuz:\t%81.81s\t%d\n", str.chars, n);
					}
				}
			}
		}
	}
	//if(opt.verbose) {
	//	printf("%d UA sets processed.\n", size());
	//}
	return 0;
}


int neighbourhoodDetails1(const char* fname, const char* knownsfname) {
	//printf("Examining Neighbour Grids.\n");
	gridsWithPuzzles gp;
	gp.loadFromFile(knownsfname);
	//gp.reindexBySize();

	//r2m1p1(gp, gp); //debug
	//return 0;

	char buf[3000];
	//strcpy(buf, fname);
	//strcat(buf, ".neighbours.txt");
	//ofstream ofile(buf);
	//if(! ofile)
	//	return -1;
	grid g;
	g.fname = fname;
	if(g.readFromFile())
		return -1;
	strcpy(buf, fname);
	strcat(buf, g.fileUASuffix);
	ifstream uafile(buf);
	if(! uafile)
		return -1;
	g.toString(buf); //source grid
	printf("Grid:\t%s\n", buf);
	transformer myselfTr;
	myselfTr.byGrid(g.digits);
	ch81 digitsCanon;
	myselfTr.transform(g.digits, digitsCanon.chars);
	unsigned int authoMorphisms = myselfTr.aut;
	bool mustTransform = myselfTr.isTransforming();
	if(mustTransform) {
		//memcpy(g.digits, digitsCanon.chars, 81);
		digit2bitmap(digitsCanon.chars, g.gridBM);
		digitsCanon.toString(buf);
		printf("Minlex:\t%81.81s\n", buf);
	}
	gridsWithPuzzles::const_iterator known;
	puzzlesInGrid::const_iterator p;
	const puzzlesInGrid *basePuzzles;
	//obtain known puzzles of the original grid
	known = gp.find(digitsCanon);
	if(known != gp.end()) {
		basePuzzles = &known->second;
		printf("Known puzzles: %d\n", (int)basePuzzles->size());
		//for(p = basePuzzles->begin(); p != basePuzzles->end(); p++) {
		//	p->toString(buf);
		//	printf("Puz:\t%81.81s\n", buf);
		//}
	}
	unsigned int uaWhCountBySize[81];
	unsigned int uaWhCountBySizeDup[81];
	unsigned int uaTrCountBySize[81];
	unsigned int uaTrCountBySizeDup[81];
	unsigned int uaAllCountBySize[81];
	memset(uaWhCountBySize, 0, 81 * sizeof(uaWhCountBySize[0]));
	memset(uaWhCountBySizeDup, 0, 81 * sizeof(uaWhCountBySizeDup[0]));
	memset(uaTrCountBySize, 0, 81 * sizeof(uaTrCountBySize[0]));
	memset(uaTrCountBySizeDup, 0, 81 * sizeof(uaTrCountBySizeDup[0]));
	memset(uaAllCountBySize, 0, 81 * sizeof(uaAllCountBySize[0]));
	puzzlesInGrid found;
	while(uafile.getline(buf, sizeof(buf))) {
		static const int maxSolutions = 20;
		unsigned long long nSol;
		ch81 puz;
		ch81 *ppuz = (ch81*)buf;
		ch81 uaText;
		uset u;
		u.fromString(buf);
		g.ua2puzzle(u, buf);
		if(mustTransform) {
			myselfTr.transform(buf, puz.chars);
			ppuz = &puz;
		}
		ppuz->toString(uaText.chars);
		ch81 sbuf[maxSolutions];
		nSol = solve(g.gridBM, ppuz->chars, maxSolutions, sbuf[0].chars);
		uaAllCountBySize[u.nbits]++;
		for(unsigned long long i = 1; i < nSol; i++) { //skip the original solution
			ch81 solCanon;
			//ch81 solText;
			transformer tr;
			tr.byGrid(sbuf[i].chars);
			tr.transform(sbuf[i].chars, solCanon.chars);
			if(memcmp(solCanon.chars, digitsCanon.chars, 81)) { //new essentially different solution
				//sbuf[i].toString(solText.chars);
				uaWhCountBySize[u.nbits]++;
				puzzlesInGrid::const_iterator dup = found.find(solCanon);
				if(dup != found.end()) {
					uaWhCountBySizeDup[u.nbits]++;
				}
				known = gp.find(solCanon);
				if(known != gp.end()) {
					uaTrCountBySize[u.nbits]++;
					if(dup != found.end()) {
						uaTrCountBySizeDup[u.nbits]++;
					}
				}
				found.insert(solCanon);
				break;
			}
		}
	}
	printf("Wormhole Distribution\nSize\tTotal\tMirrors\tWorms\tHits\tHits%%\tWormDup\tHitDup\n");
	unsigned int AllUA = 0, TrUA = 0, WhUA = 0, TrUADup = 0, WhUADup = 0;
	for(int i = 0; i < 81; i++) {
		if(uaAllCountBySize[i]) {
			AllUA += uaAllCountBySize[i];
			WhUA += uaWhCountBySize[i];
			WhUADup += uaWhCountBySizeDup[i];
			TrUA += uaTrCountBySize[i];
			TrUADup += uaTrCountBySizeDup[i];
			double r = 100.0 * uaTrCountBySize[i] / (double)uaWhCountBySize[i];
			printf("%i\t%u\t%u\t%u\t%u\t%.4f%%\t%u\t%u\n",
				i, uaAllCountBySize[i], uaAllCountBySize[i] - uaWhCountBySize[i], uaWhCountBySize[i], uaTrCountBySize[i], r,
				uaWhCountBySizeDup[i], uaTrCountBySizeDup[i]);
		}
	}
	printf("%10u UA sets processed.\n", AllUA);
	printf("%10u (%.4f%%) Mirrors.\n", AllUA - WhUA, 100.0 * (AllUA - WhUA) / (double)AllUA);
	printf("%10u (%.4f%%) Wormholes (excl. %u duplicates).\n", WhUA - WhUADup, 100.0 * (WhUA - WhUADup) / (double)AllUA, WhUADup);
	printf("%10u (%.4f%%) Hits (excl. %u duplicates).\n", TrUA - TrUADup, 100.0 * (TrUA - TrUADup) / (double)WhUA, TrUADup);
	if(TrUA) {
		printf("Hit ratio 1:%u (excl. duplicates).\n", (WhUA - WhUADup) / (TrUA - TrUADup));
	}
	//if(opt.verbose) {
	//	printf("%d UA sets processed.\n", size());
	//}
	return 0;
}

class dist8bits {
	unsigned char * d;
	int s;
public:
	dist8bits(size_t size) {
		s = (int)size;
		//size X size matrix
		//with main diagonal == 0
		//with lower left triangle skipped
		//i.e for 7x7 we need 21 elements = ((7 - 1) * 7) / 2
		//   0123456
		//   =======
		// 0 ?012345
		// 1 ??67890
		// 2 ???1234
		// 3 ????567
		// 4 ?????89
		// 5 ??????0
		// 6 ???????
		//using half byte for result [0..15] the space is halved
		//for odd size leave one more half-byte
		int nBytes = (int)(((size - 1) * size) / 4 + (size & 1));
		d = (unsigned char*) malloc(nBytes);
		memset(d, 0xff, nBytes);
	}
	~dist8bits() {
		free(d);
	}
	unsigned int getDist(int x, int y) const {//size=7, x=6, y=5 cell=20; x=4, y=3 cell=15
		int cell = s * y - (y + 1) * y / 2 + x - y - 1;
		unsigned int v = d[cell / 2];
		if(cell & 1) {
			return v >> 4;
		}
		else {
			return v & 0xf;
		}
	}
	void setDist(int x, int y, unsigned char value) {
		int cell = s * y - (y + 1) * y / 2 + x - y - 1;
		if(cell & 1) {
			d[cell / 2] = (d[cell / 2] & 0x0f) | (value << 4);
		}
		else {
			d[cell / 2] = (d[cell / 2] & 0xf0) | value;
		}
	}
};

map<int,int> s;
void uaDistance(const ch81 &a, const ch81 &b, char * ab, char *ba) {
	//if((*ab | *ba) == 1)
	//	return; //no shorter distances exists

	//compose puzzle of the common clues
	ch81 p;
	const int maxSol = 30;
	ch81 sol[maxSol];
	p.clear();
	int n = 0;
	for(int i = 0; i < 81; i++) {
		if(a.chars[i] == b.chars[i]) {
			//p.chars[i] = a.chars[i];
			n++;
		}
	}
	s[n]++;

	//int nSol = (int)solve(p.chars, maxSol, sol[0].chars);
	//if(nSol < maxSol) {
	//	ch81 pText;
	//	a.toString(pText.chars);
	//	printf("\n%81.81s\n", pText.chars);
	//	b.toString(pText.chars);
	//	printf("%81.81s\n", pText.chars);

	//	findUaBySolutions(sol[0].chars, nSol);

	//	//grid g;
	//	//memcpy(g.digits, a.chars, 81);
	//	//digit2bitmap(g.digits, g.gridBM);
	//	//g.findUAbyPuzzle(p.chars);

	//	//for(usetListBySize::const_iterator u = g.usetsBySize.begin(); u != g.usetsBySize.end(); u++) {
	//	//	u->toPuzzleString(a.chars, pText.chars);
	//	//	printf("%81.81s\t%d\n", pText.chars, u->nbits);
	//	//	u->toPuzzleString(b.chars, pText.chars);
	//	//	printf("%81.81s\n", pText.chars);
	//	//}

	//	printf("%d, %d\n", n, nSol);
	//	*ab = 1;
	//}
}

void findUADistance(int index, const ch81 * list, int nGrids, char *distances, const allTranformations &theTransformations) {
	printf("\nGrid Index %d\n", index);
	for(int t = 0; t < allTranformations::count; t++) {
		ch81 theGrid;
		theTransformations.transform(list[index], theGrid, t);
		for(int g = index/* + 1*/; g < nGrids; g++) {
			uaDistance(theGrid, list[g], &distances[g * nGrids + index], &distances[index * nGrids + g]);
		}
	}
}

int uaDist() {
	ch81 * list;
	char * distMatrix;
	size_t nGrids;
	allTranformations theTransformations;

	//compose transformations

	//load the whole list
	{
		puzzleSet m0; //given grids, using old code to read, then copied to list.
		m0.loadFromFile(stdin); //load puzzles
		nGrids = m0.size();
		list = (ch81*) malloc(sizeof(ch81) * nGrids);
		distMatrix = (char*) malloc(nGrids * nGrids);
		int i = 0;
		for(puzzleSet::const_iterator p = m0.begin(); p != m0.end(); p++) {
			list[i++] = *p;
		}
		m0.clear();
	}

	for(int i = 0; i < nGrids /*- 1*/; i++) {
		findUADistance(i, list, (int)nGrids, distMatrix, theTransformations);
	}

	//clusterize
	//print
	int nOnes = 0;
	for(int i = 0; i < nGrids - 1; i++) {
		for(int j = i + 1; j < nGrids; j++) {
			if(distMatrix[i * nGrids + j] == 1) {
				nOnes++;
			}
		}
	}
	printf("Grids at UA distance 1 = %d\n", nOnes);

	free(distMatrix);
	free(list);
	for(map<int,int>::const_iterator p = s.begin(); p != s.end(); p++) {
		printf("%d\t%d\n", p->first, p->second);
	}
	return 0;
}