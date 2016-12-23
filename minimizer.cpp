#define _CRT_SECURE_NO_DEPRECATE

#include <memory.h>

#include "minimizer.h"
#include "options.h"

int xskipped[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //debug/timings/counters

int minimizer::init(const char * const givens) {
	char buf[256];
	int nClues = 0;
	usetListBySize filteredUA;
	//find the only solution grid
	if(solve(givens, 2, buf) != 1)
		return 1;
	for(int i = 0; i < 81; i++) {
		puz.chars[i] = givens[i];
		g.digits[i] = buf[i];
	}
	//setup grid
	g.setBM();
	g.usetsBySize.clear();
	//obtain the forced clues
	fixedGivens.clear();
	nFixedGivens = 0;
	fg.clear();
	fng.clear();
	for(int i = 0; i < 81; i++) {
		int c;
		if((c = puz.chars[i])) {
			nClues++;
			puz.chars[i] = 0;
			if(solve(puz.chars, 2) != 1) {
				//lucky, a forced clue found
				fixedGivens.chars[i] = 1;
				fg.setBit(i);
				nFixedGivens++;
			}
			puz.chars[i] = c;
		}
		else {
			fng.setBit(i);
		}
	}
	int nFloating = nClues - nFixedGivens;
	if(opt.verbose) {
		ch81 p;
		puz.toString(p.chars);
		fprintf(stderr, "\n%81.81s\t%d\n", p.chars, nClues);
		fprintf(stderr, "%d givens\t%d fixed\t%d floating\n", nClues, nFixedGivens, nFloating);
	}
	if(nFloating == 0) {
		//minimal input, just output the original
		if(opt.verbose) {
			fprintf(stderr, "method\tminimal original\n");
		}
		ch81 p;
		puz.toString(p.chars);
		printf("%81.81s\t%d\n", p.chars, nClues);
		return 0;
	}
	else if (nFloating == 1) {
		//output the original w/o this clue
		if(opt.verbose) {
			fprintf(stderr, "method\tremove the single floating\n");
		}
		uset fl;
		fl.bitmap128 = maskLSB[81];
		fl.clearBits(fg);
		fl.clearBits(fng);
		fl.positionsByBitmap();
		puz.chars[fl.positions[0]] = 0;
		ch81 p;
		puz.toString(p.chars);
		printf("%81.81s\t%d\n", p.chars, nClues - 1);
		return 0;
	}
	//else if (nFloating <= 9) { //512 KB bool[] + BitCount[]
	else if (nFloating <= 17) { //32=512 MB std::vector<bool>
		if(opt.verbose) {
			fprintf(stderr, "method\tmark\n");
		}
		combineFloating();
		return 0;
	}
	//hard case: too many floating cells
	if(opt.verbose) {
		fprintf(stderr, "method\tUA hitting ");
	}
	//find UA sets (random search depends of forced givens)
	g.findUA12();
	g.findUA4digits();
	g.findUArandom(fixedGivens.chars, 53, 2000);
	for(usetListBySize::const_iterator u = g.usetsBySize.begin(); u != g.usetsBySize.end(); u++) {
		if(nFixedGivens && (!u->isDisjoint(fg))) {
			//skip UA hit by forced clues
			continue;
		}
		uset uu(*u);
		uu.clearBits(fng);
		uu.positionsByBitmap();
		filteredUA.insert(uu);
	}
	g.usetsBySize.clear();
	numUsets = 0;
	for(usetListBySize::const_iterator u = filteredUA.begin(); numUsets < MAX_USETS && u != filteredUA.end(); numUsets++, u++) {
		usets[numUsets] = *u;
	}
	if(opt.verbose) {
		fprintf(stderr, "(%d out of %d)\n", numUsets, (int)filteredUA.size());
	}
	filteredUA.clear();
	numUsetPages = 1 + (numUsets - 1) / 128;
	//create bitmap indexes for the UA, but keep also the originals for faster by-given access later
	bm128 ss;
	int nSlices = numUsets / 16 + (numUsets % 16 ? 1 : 0);
	for (int slice = 0; slice < nSlices; slice++) { //process 16 rows from the source simultaneously
		//process first 80 bits
		for (int srcCol = 0; srcCol < 10; srcCol++) { //process 8 bits per "column" simultaneously
			for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source
				ss.bitmap128.m128i_u8[srcSliceRow] = usets[slice*16+srcSliceRow].bitmap128.m128i_u8[srcCol];
			}
			ss.transposeSlice(ss); // 16 bits * 8 columns for the target
			for (int destRow = 0; destRow < 8; destRow++) {
				hittingMasks[srcCol * 8 + destRow].pages[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[destRow];
			}
		}
		//process 81-th bit
		for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source, only first bit is used
			ss.bitmap128.m128i_u8[srcSliceRow] = usets[slice*16+srcSliceRow].bitmap128.m128i_u8[10];
		}
		ss = _mm_slli_epi64(ss.bitmap128.m128i_m128i, 7); // move bit 0 to bit 7
		ss.bitmap128.m128i_u16[0] = _mm_movemask_epi8(ss.bitmap128.m128i_m128i);
		hittingMasks[80].pages[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[0];
	}
	//populate setMask with ones except the possible end when insufficient number of UA has been collected
	for(int i = 0; i < numUsetPages; i++) {
		state[0].setMask.pages[i] = maskffff;
	}
	int i = (MAX_USETS - numUsets) % 128;
	if(i) {
		state[0].setMask.pages[numUsetPages - 1] = maskLSB[128 - i];
	}

	//enumerate minimals
	state[nFixedGivens].setMask = state[0].setMask;
	state[nFixedGivens].clues = fg;
	state[nFixedGivens].deadClues = fng;
	state[nFixedGivens].redundantCandidates = fg; //any forced given is candidate for redundancy
	state[nFixedGivens].uaIndex = 0;
	enumerateState(nFixedGivens);
	if(opt.verbose) {
		fprintf(stderr, "UA in dead clues, branches skipped = %d of %d\n", xskipped[1], xskipped[9]);
		fprintf(stderr, "All UA killed events = %d\n", xskipped[10]);
		fprintf(stderr, "Secondary UA generations = %d of %d\n", xskipped[3], xskipped[2]);
		fprintf(stderr, "Largest Secondary UA generation = %d\n", xskipped[12]);
		fprintf(stderr, "Final minimals = %d, non-minimals = %d\n", xskipped[4], xskipped[5]);
		fprintf(stderr, "Intermediate minimals = %d, non-minimals = %d\n", xskipped[6], xskipped[7]);
		fprintf(stderr, "Recursions = %d unchecked, %d checked\n", xskipped[8], xskipped[6]);
		fprintf(stderr, "Minimality check clues skipped = %d of %d\n", xskipped[0], xskipped[11]);
	}
	return 0;
}

int minimizer::initFast(const minimizer & parent, int stateIndex) {
	g = parent.g;
	uset puzBin(parent.state[stateIndex].clues);
	puzBin.positionsByBitmap();
	g.ua2InvariantPuzzle(puzBin, puz.chars);

	//setup grid
	g.usetsBySize.clear();
	nFixedGivens = stateIndex;
	fg = parent.state[stateIndex].clues;
	fng = parent.state[stateIndex].deadClues;
	g.findUAbyPuzzle(puz.chars);
	numUsets = 0;
	for(usetListBySize::const_iterator u = g.usetsBySize.begin(); numUsets < MAX_USETS && u != g.usetsBySize.end(); numUsets++, u++) {
		usets[numUsets] = *u;
	}
	g.usetsBySize.clear();
	if(xskipped[12] < numUsets) xskipped[12] = numUsets;
	numUsetPages = 1 + (numUsets - 1) / 128;
	//create bitmap indexes for the UA, but keep also the originals for faster by-given access later
	bm128 ss;
	int nSlices = numUsets / 16 + (numUsets % 16 ? 1 : 0);
	for (int slice = 0; slice < nSlices; slice++) { //process 16 rows from the source simultaneously
		//process first 80 bits
		for (int srcCol = 0; srcCol < 10; srcCol++) { //process 8 bits per "column" simultaneously
			for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source
				ss.bitmap128.m128i_u8[srcSliceRow] = usets[slice*16+srcSliceRow].bitmap128.m128i_u8[srcCol];
			}
			ss.transposeSlice(ss); // 16 bits * 8 columns for the target
			for (int destRow = 0; destRow < 8; destRow++) {
				hittingMasks[srcCol * 8 + destRow].pages[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[destRow];
			}
		}
		//process 81-th bit
		for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source, only first bit is used
			ss.bitmap128.m128i_u8[srcSliceRow] = usets[slice*16+srcSliceRow].bitmap128.m128i_u8[10];
		}
		ss = _mm_slli_epi64(ss.bitmap128.m128i_m128i, 7); // move bit 0 to bit 7
		ss.bitmap128.m128i_u16[0] = _mm_movemask_epi8(ss.bitmap128.m128i_m128i);
		hittingMasks[80].pages[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[0];
	}
	//populate setMask with ones except the possible end when insufficient number of UA has been collected
	for(int i = 0; i < numUsetPages; i++) {
		state[0].setMask.pages[i] = maskffff;
	}
	int i = (MAX_USETS - numUsets) % 128;
	if(i) {
		state[0].setMask.pages[numUsetPages - 1] = maskLSB[128 - i];
	}
	//enumerate minimals
	state[nFixedGivens].setMask = state[0].setMask;
	state[nFixedGivens].clues = fg;
	state[nFixedGivens].deadClues = fng;
	state[nFixedGivens].redundantCandidates = fg; //any forced given is candidate for redundancy
	state[nFixedGivens].uaIndex = 0;
	enumerateState(nFixedGivens);
	return 0;
}

void minimizer::enumerateState(int stateIndex) {
	uset &u = usets[state[stateIndex].uaIndex];
	for(unsigned int n = 0; n < u.nbits; n++) {
		int cluePos = u.positions[n];
		if(state[stateIndex].deadClues.isBitSet(cluePos)) {
			continue;
		}
		state[stateIndex].clues.setBit(cluePos);
		//hit the corresponding unavoidable sets
		int nextUA = -1;
		for(int i = state[stateIndex].uaIndex / 128; i < numUsetPages; i++) {
			state[stateIndex + 1].setMask.pages[i] = state[stateIndex].setMask.pages[i];
			state[stateIndex + 1].setMask.pages[i].clearBits(hittingMasks[cluePos].pages[i]);
			//find the next unhit UA if exists
			if(nextUA == -1) {
				 nextUA = state[stateIndex + 1].setMask.pages[i].getFirstBit1Index(); //-1 if none
				if(nextUA != -1) {
					nextUA += i * 128;
				}
			}
		}
		if(nextUA == -1) { //all known UA are hit
			xskipped[10]++;
			//check for uniqueness and backtrack
			ch81 p, pp;
			state[stateIndex].clues.toPuzzle(g.digits, p.chars);
			if(solve(p.chars, 2) != 1) {
				xskipped[2]++;
				if(solverIsIrreducibleByProbing(p.chars)) {
					//enumerate this subtree after collecting more UA sets
					xskipped[3]++;
					minimizer * sm = new minimizer;
					sm->initFast(*this, stateIndex);
					delete sm;
				}
			}
			else {
				//check for minimality
				//if(solverIsIrreducible(p.chars)) {
				bm128 rc(state[stateIndex].redundantCandidates);
				rc.clearBit(cluePos);
				if(isIrreducible(p.chars, rc)) {
					//output
					xskipped[4]++;
					int nClues = p.toString(pp.chars);
					printf("%81.81s\t%d\n", pp.chars, nClues);
				}
				else {
					xskipped[5]++;
					//ignore non-minimals
				}
			}
		}
		else {
			//there are more UA to hit
			if(stateIndex > 24) { //25=same, 22=better, 20=worse, 23=better
				//check the partial puzzle for minimality
				ch81 p;
				state[stateIndex].clues.toPuzzle(g.digits, p.chars);
				//bm128 rc(state[stateIndex].redundantCandidates);
				//rc.clearBit(cluePos);
				if(solverIsIrreducibleByProbing(p.chars)) {
					xskipped[6]++;
					//hit next unavoidable set and recurse
					state[stateIndex + 1].clues = state[stateIndex].clues;
					state[stateIndex + 1].deadClues = state[stateIndex].deadClues;
					state[stateIndex + 1].redundantCandidates = u.bitmap128;
					state[stateIndex + 1].redundantCandidates.clearBits(maskLSB[cluePos]); //??? why cluePos+1 doesn't work?
					state[stateIndex + 1].redundantCandidates |= state[stateIndex].redundantCandidates;
					state[stateIndex + 1].uaIndex = nextUA;
					enumerateState(stateIndex + 1);
				}
				else {
					xskipped[7]++;
					//ignore partial non-minimals
				}
			}
			else {
				xskipped[8]++;
				//hit next unavoidable set and recurse
				state[stateIndex + 1].clues = state[stateIndex].clues;
				state[stateIndex + 1].deadClues = state[stateIndex].deadClues;
				state[stateIndex + 1].redundantCandidates = u.bitmap128;
				state[stateIndex + 1].redundantCandidates.clearBits(maskLSB[cluePos]); //??? why cluePos+1 doesn't work?
				state[stateIndex + 1].redundantCandidates |= state[stateIndex].redundantCandidates;
				state[stateIndex + 1].uaIndex = nextUA;
				enumerateState(stateIndex + 1);
			}
		}
		//done with this clue, undo it as given and mark it as dead
		state[stateIndex].deadClues.setBit(cluePos);
		state[stateIndex].clues.clearBit(cluePos);
		if(n < u.nbits - 1) {
			xskipped[9]++;
			//check if initial puzzle without dead clues is unique
			ch81 p;
			state[stateIndex].deadClues.toPseudoPuzzle(g.digits, p.chars);
			if(1 != solve(p.chars, 2)) {
				xskipped[1]++;
				//this branch leads to nowhere
				//fprintf(stderr, ".");
				return;
			}
		}
	}
}

void minimizer::combineFloating() {
	uset fl;
	fl.bitmap128 = maskLSB[81];	//all 81 cells are floating
	fl.clearBits(fg);		//exclude forced givens
	fl.clearBits(fng);		//exclude forced non-givens
	fl.positionsByBitmap();	//create cells index and determine size
	size_t size = 1 << fl.nbits;	//the number of combinations
	int maxFloatingForRemoval = fl.nbits - (17 - nFixedGivens);	//don't expect valid puzzles with 16 or less givens

	//mark which combinations lead to multiple solutions or have redundant clues
	//std::vector<bool> invalids(size, false);
	bool *invalids = new bool[size];
	memset(invalids, 0, size * sizeof(bool));

	checkAndMark check(invalids, fl, puz, size); //parallel code

	//stage 1: check for uniqueness in increasing clues removal number and mark multiple-solutions puzzles as invalid
	//start from 2 since we know any single removal keeps the puzzle unique (else the given would be forced)
	for(int numBits = 2; numBits <= maxFloatingForRemoval; numBits++) {
		//loop over all sets of size numBits
		for(size_t mask = (1 << numBits) - 1; mask < size; mask = nextPerm(mask)) {
			if(invalids[mask]) {
				//this puzzle is a subet of some previously checked non-unique puzzle
				continue;
			}
			check.add(mask); //parallel code
#if 0 //serial code
			//compose a puzzle with initial givens with those from the mask excluded
			ch81 p = puz; //structure copy
			for(int theBit = 0; theBit < fl.nbits; theBit++) {
				if(mask & (1 << theBit)) {
					//clear this floating clue
					p.chars[fl.positions[theBit]] = 0;
				}
			}
			if(1 == solve(p.chars, 2)) {
				//still unique, do nothing
			}
			else {
				//Multiple solutions, mark all supersets as invalid
				//http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=bitManipulation
				size_t nmask = ~mask;
				for(size_t i = nmask; (~i) < size; i = (i - 1) & nmask) {
					invalids[~i] = true;
				}
			}
#endif //serial code
		}
		check.finalize(); //parallel code
	}
	//stage 2: output the non-invalid puzzles with most clues removed, marking rest as invalid=non-minimal
	for(int numBits = maxFloatingForRemoval; numBits > 0; numBits--) {
		//loop over all sets of size numBits
		for(size_t mask = (1 << numBits) - 1; mask < size; mask = nextPerm(mask)) {
			if(invalids[mask]) {
				//this puzzle has been previously discarded
				continue;
			}
			//this combination is the largest and still unique => is minimal
			//output the puzzle
			ch81 p = puz; //structure copy
			for(unsigned int theBit = 0; theBit < fl.nbits; theBit++) {
				if(mask & (1 << theBit)) {
					//clear this floating clue
					p.chars[fl.positions[theBit]] = 0;
				}
			}
			ch81 pp;
			int n = p.toString(pp.chars);
			printf("%81.81s\t%d\n", pp.chars, n);
			//mark all subsets as "invalid non-unique"
			for(size_t i = mask; i; i = (i - 1) & mask) {
				if(invalids[i]) {
					continue;
				}
				invalids[i] = true;
			}
		}
	}
	delete [] invalids;
}

