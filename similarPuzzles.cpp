#include "solver.h"
#include "grids.h"
#include "rowminlex.h"
#include "clusterize.h"
#include "options.h"
#include "minimizer.h"
#include "patminlex.h"

extern const unsigned int Bitmap2Digit[];

//struct ch41 {
//	unsigned char compChars[41];
//	bool operator < (const ch41 &rhs) const {
//		return (memcmp(this, &rhs, sizeof(ch41)) < 0);
//	}
//};
//
//struct compPuzzleSet : public set<ch41> {
//private:
//	void compress(ch81 &u, ch41 &c) const {
//		for(int i = 0; i < 40; i++) {
//			c.compChars[i] = (u.chars[i * 2] << 4) | (u.chars[i * 2 + 1]);
//		}
//		c.compChars[40] = u.chars[80];
//	}
//	void uncompress(ch81 &u, ch41 &c) const {
//		for(int i = 0; i < 40; i++) {
//			u.chars[i * 2] = c.compChars[i] >> 4;
//			u.chars[i * 2 + 1] = c.compChars[i] & 0x0f;
//		}
//		u.chars[80] = c.compChars[40];
//	}
//};

void minimizeMore(ch81 &puz, const int * const redundants, const int nRedundants, const int pos) {
	//strip redundants from unique puzzle, or
	//print puz if it is minimal
	//preserve the givens
	//call only with unique puzzle in puz

	//first check whether we deal with the last candidate for removal
	char &removed = puz.chars[redundants[pos]];
	int v = removed;
	if(nRedundants - pos == 1) {
		//check the last redundant
		ch81 ppuz;
		removed = 0;
		if(1 == solve(puz.chars, 2)) { //redundant clue at last position
			//check wether none of the previously preserved candidates became redundant
			for(int i = 0; i < pos - 1; i++) {
				char &rr = puz.chars[redundants[i]];
				int vv = rr;
				if(vv) { //preserved candidate
					rr = 0;
					if(1 == solve(puz.chars, 2)) { //redundant clue at previous position
						//just skip printing the puzzle, it was already printed before
						rr = vv;
						removed = v;
						return;
					}
					rr = vv;
				}
			}
			puz.toString(ppuz.chars);
			fprintf(stdout, "%81.81s\n", ppuz.chars);
			removed = v;
			return;
		}
		//last isn't redundant in this context, print the original
		removed = v;
		//check wether none of the previously preserved candidates became redundant
		for(int i = 0; i < pos - 1; i++) {
			char &rr = puz.chars[redundants[i]];
			int vv = rr;
			if(vv) { //preserved candidate
				rr = 0;
				if(1 == solve(puz.chars, 2)) { //redundant clue at previous position
					//just skip printing the puzzle, it was already printed before
					rr = vv;
					return;
				}
				rr = vv;
			}
		}
		puz.toString(ppuz.chars);
		fprintf(stdout, "%81.81s\n", ppuz.chars);
		return;
	}
	//we have more than 1 candidate for removal
	//try with the first candidate removed and recurse
	removed = 0;
	if(1 == solve(puz.chars, 2)) { //still redundant
		minimizeMore(puz, redundants, nRedundants, pos + 1); //print all minimals
	}
	removed = v;
	//try with the first candidate preserved and recurse
	minimizeMore(puz, redundants, nRedundants, pos + 1); //print all minimals
}

void minimize(ch81 &puz) { //strip redundants from unique puzzle and print all minimals to stdout
	minimizer m;
	m.init(puz.chars);
}
//void minimize(ch81 &puz) { //strip redundants from unique puzzle and print all minimals to stdout
//	int nRedundants = 0;
//	int redundants[81]; //indices of redundant clues
//	//find all redundant clues
//	for(int pos = 0; pos < 81; pos++) {
//		int v = puz.chars[pos];
//		if(v) {
//			puz.chars[pos] = 0;
//			if(1 == solve(puz.chars, 2)) { //redundant clue at [pos]
//				redundants[nRedundants++] = pos;
//			}
//			puz.chars[pos] = v; //restore the clue for next iteration
//		}
//	}
//	if(nRedundants) {
//		minimizeMore(puz, redundants, nRedundants, 0); //print all minimals
//		return;
//	}
//	//print original
//	ch81 ppuz;
//	puz.toString(ppuz.chars);
//	fprintf(stdout, "%81.81s\n", ppuz.chars);
//	return;
//}

//int stripRedundantCluesSS(ch81 &puz, const int startPos = 0) { //strip redundants from unique puzzle
//	int minClues = 100;
//	ch81 bestPuzzle;
//	for(int pos = startPos; pos < 81; pos++) {
//		int v = puz.chars[pos];
//		if(v) {
//			puz.chars[pos] = 0;
//			if(1 == solve(puz.chars, 2)) { //redundant clue at [pos]
//				ch81 testPuzzle = puz; //work with a copy
//				int testClues = stripRedundantCluesSS(testPuzzle, pos + 1);
//				if(testClues < minClues) {
//					bestPuzzle = testPuzzle;
//					minClues = testClues;
//				}
//			}
//			puz.chars[pos] = v; //restore the clue for next iteration
//		}
//	}
//	if(minClues < 100) { //reduced
//		puz = bestPuzzle;
//		return minClues;
//	}
//	//simply count the clues and return the unchanged original
//	int thisClues = 0;
//	for(int pos = 0; pos < 81; pos++) {
//		if(puz.chars[pos]) {
//			thisClues++;
//		}
//	}
//	return thisClues;
//}

void cousins(ch81 &puz) { //
	grid g;
	if(1 != solve(puz.chars, 1, g.digits)) {
		//invalid puzzle
		return;
	}
	//process only the first solution grid, assuming it is the only one
	g.setBM();
	g.findUA12();
	for(usetListBySize::const_iterator ua = g.usetsBySize.begin(); ua != g.usetsBySize.end(); ua++) {
		ch81 helperPuzzle;
		ch81 similarGrids[2];
		memcpy(helperPuzzle.chars, g.digits, 81);
		for(int pos = 0; pos < 81; pos++) {
			//clear the UA cells
			if(ua->isBitSet(pos)) {
				helperPuzzle.chars[pos] = 0;
			}
		}
		solve(g.gridBM, helperPuzzle.chars, 2, similarGrids[0].chars);
		//compose a puzzle with givens from the second solution
		ch81 candidate;
		candidate.clear();
		for(int pos = 0; pos < 81; pos++) {
			if(puz.chars[pos]) {
				candidate.chars[pos] = similarGrids[1].chars[pos];
			}
		}
		//test for uniqueness
		if(1 != solve(candidate.chars, 2)) {
			continue;
		}
		//test for minimality
		if(solverIsIrreducibleByProbing(candidate.chars)) {
			subcanon(candidate.chars, helperPuzzle.chars);
			int size = helperPuzzle.toString(candidate.chars);
			printf("%81.81s\t%d\n", candidate.chars, size);
		}
	}
}

int stripRedundantClues(ch81 &puz) { //strip redundants keeping the same solutions (there are other methods for stripping)
	int pencilMarks[81], pencilMarksReduced[81], ret = 0;
	solve(puz.chars, pencilMarks);
	//findPencilMarks(puz.chars, pencilMarks); //returns empty pencilmarks on unsolvable puzzle which is wrong here
	for(int pos = 0; pos < 81; pos++) {
		int v = puz.chars[pos];
		if(v) {
			puz.chars[pos] = 0;
			solve(puz.chars, pencilMarksReduced);
			//findPencilMarks(puz.chars, pencilMarksReduced);
			if(memcmp(pencilMarks, pencilMarksReduced, 81*sizeof(pencilMarks[0]))) {
				//clue at pos does affect pencilmarks and therefore isn't redundant
				puz.chars[pos] = v;
				ret++;
			}
			//else leave the redundant clue removed
		}
	}
	return ret;
}
//void invertAndMinimize(ch81 &puz) {
//	ch81 puzModified[2];
//	//if(solve(puz.chars, 2, puzModified[0].chars) != 1) {
//	//	//unsolvable or multi-solution puzzle
//	if(solve(puz.chars, 1, puzModified[0].chars) != 1) {
//		//unsolvable or multi-solution puzzle
//		printf("incorrect puzzle\n");
//		return;
//	}
//	//invert
//	for(int i = 0; i < 81; i++) {
//		if(puz.chars[i]) {
//			puzModified[0].chars[i] = 0; //clear all givens
//		}
//	}
//	////minimize
//	//stripRedundantClues(puz = puzModified[0]);
//	puz = puzModified[0];
//}

void checkForTwinsSS(const ch81 &puzOriginal, const ch81 &solOriginal, const int * const solBM, puzzleSet &newPuzzles, bool minimals) {
	//compose the complementary puzzle
	ch81 puzModified = solOriginal; //start from the solution
	for(int i = 0; i < 81; i++) {
		if(puzOriginal.chars[i]) {
			puzModified.chars[i] = 0; //clear all givens
		}
	}
	//get all solutions
	const int maxSol = 50000;
	ch81 solutions[maxSol];
	unsigned long long nSol = solve(solBM, puzModified.chars, maxSol, solutions[0].chars);
	if(nSol > 1) {
		//there are twins
		if(nSol == maxSol) {
			printf("checkForTwinsSS: The maximum of %d solutions reached!\n", maxSol);
		}
		for(unsigned int i = 1; i < nSol; i++) { //skip the original solution
			//compose the complementary puzzle
			puzModified = solutions[i];
			for(int i = 0; i < 81; i++) {
				if(puzOriginal.chars[i] == 0) {
					puzModified.chars[i] = 0;
				}
			}
			//canonicalize the puzzle
			ch81 puzCanon;
			subcanon(puzModified.chars, puzCanon.chars);
			if(minimals) {
				if(newPuzzles.find(puzCanon) == newPuzzles.end()) { //don't check duplicates for minimality
					if(0 == solverIsIrreducibleByProbing(puzCanon.chars)) {
						continue;
					}
				}
			}
			newPuzzles.insert(puzCanon);
			if(opt.verbose) {
				char pprint1[81], pprint2[81];
				puzOriginal.toString(pprint1);
				puzModified.toString(pprint2);
				fprintf(stderr, "%81.81s\t%81.81s\n", pprint1, pprint2);
			}
		}
	}
}

void checkForTwins(const ch81 &puzOriginal, puzzleSet &newPuzzles, bool minimals) {
	int solBM[81];
	ch81 sol;
	unsigned long long nSol = solve(puzOriginal.chars, 1, sol.chars);
	if(nSol == 0)
		return; //silently ignore invalid puzzles
	digit2bitmap(sol.chars, solBM);
	checkForTwinsSS(puzOriginal, sol, solBM, newPuzzles, minimals);
	fflush(NULL); //display cached messages
}

void checkForTwins(puzzleSet &puzzles, puzzleSet &newPuzzles, bool minimals) {
	for(puzzleSet::const_iterator p = puzzles.begin(); p != puzzles.end(); p++) {
		checkForTwins(*p, newPuzzles, minimals);
	}
}

void doPlus(const ch81 &puz, puzzleSet &puzzles, puzzleSet &newPuzzles, puzzleSet &msPuzzles) {
	const int maxPuz = 1000;
	ch81 plusPuz[maxPuz];
	unsigned long long plusNumSol[maxPuz];
	int nPuz;
	ch81 puzOriginal = puz;
	ch81 puzOriginalText; //debug
	nPuz = solverPlus(puzOriginal.chars, maxPuz, plusPuz[0].chars, plusNumSol);
	if(nPuz == maxPuz) {
		puz.toString(puzOriginalText.chars);
		printf("{+1}: max limit of nPuz reached: %81.81s\n", puzOriginalText.chars);
	}
	for(int i = 0; i < nPuz; i++) {
		if(plusNumSol[i] == 1) {
			//single solution candidate for new puzzle
			//canonicalize the puzzle
			ch81 puzCanon;
			subcanon(plusPuz[i].chars, puzCanon.chars);
			//check whether it is an isomorph of a known puzzle
			puzzleSet::iterator p = puzzles.find(puzCanon);
			if(p == puzzles.end()) {
				//unknown single solution minimal puzzle
				//add to knowns
				puzzles.insert(puzCanon);
				newPuzzles.insert(puzCanon);
				//now apply some cheap transformations on the newfound
				//checkForTwins(puzCanon, puzzles, newPuzzles);
				//relabel();
			}
		}
		else if(plusNumSol[i] == 0) { //debug
			puz.toString(puzOriginalText.chars);
			printf("{+1}: error: unsolvable puzzle found from: %81.81s\n", puzOriginalText.chars);
		}
		else {
			//multi-solution puzzle found, recurse
			//doPlus(plusPuz[i], puzzles, newPuzzles);

			ch81 puzCanon;
			subcanon(plusPuz[i].chars, puzCanon.chars);
			//check whether it is an isomorph of a known puzzle
			puzzleSet::iterator p = msPuzzles.find(puzCanon);
			if(p == msPuzzles.end()) {
				//unknown multi-solution minimal puzzle
				//add to knowns
				msPuzzles.insert(puzCanon);
				doPlus(plusPuz[i], puzzles, newPuzzles, msPuzzles);
			}
		}
	}
}

void doPlus(const ch81 &puz, puzzleSet &puzzles, puzzleSet &newPuzzles, puzzleSet &multiSol, puzzleSet &newMultiSol) {
	int pencilMarks[81];
	unsigned long long nsol;
	ch81 puzOriginalText; //debug
	nsol = solve(puz.chars, pencilMarks);
	if(nsol == ULLONG_MAX) {
		puz.toString(puzOriginalText.chars);
		printf("{+1}: too many solutions: %81.81s\n", puzOriginalText.chars);
	}
	else if(nsol == 0) {
		puz.toString(puzOriginalText.chars);
		printf("{+1}: no solutions: %81.81s\n", puzOriginalText.chars);
	}
	else if(nsol == 1) {
		puz.toString(puzOriginalText.chars);
		printf("{+1}: single solution: %81.81s\n", puzOriginalText.chars);
	}
	else { //multiple solutions, as expected
		ch81 puzModified = puz;
		for(int pos = 0; pos < 81; pos++) {
			if(puz.chars[pos]) {
				continue; //it is a given
			}
			int pm = pencilMarks[pos];
			if(Bitmap2Digit[pm]) {
				continue; //single possibility
			}
			int valueBM;
			while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
				pm ^= valueBM; //clear this bit from the pencilmarks
				puzModified.chars[pos] = Bitmap2Digit[valueBM];
				int pencilMarks2[81];
				int pencilMarks3[81];
				nsol = solve(puzModified.chars, pencilMarks2);
				if(nsol > 1) {
					//still multi-solution after +1
					for(int j = 0; j < 81; j++) {
						int r = puzModified.chars[j];
						if(r == 0)
							continue;
						if(j == pos)
							continue;
						puzModified.chars[j] = 0;
						nsol = solve(puzModified.chars, pencilMarks3);
						if(0 == memcmp(pencilMarks2, pencilMarks3, 81*sizeof(pencilMarks[0]))) {
							//j is a redundant clue
							puzModified.chars[j] = r;
							goto nextValue;
						}
						puzModified.chars[j] = r;
					}
					//multi-solution w/o redundant clues
					ch81 puzCanon;
					subcanon(puzModified.chars, puzCanon.chars);
					puzzleSet::iterator p = multiSol.find(puzCanon);
					if(p == multiSol.end()) {
						//unknown multi-solution w/o redundant clues
						doPlus(puzModified, puzzles, newPuzzles, multiSol, newMultiSol);
						multiSol.insert(puzCanon);
						newMultiSol.insert(puzCanon);
					}
				}
				else if(nsol == 1) {
					//single solution candidate for new puzzle
					//canonicalize the puzzle
					ch81 puzCanon;
					subcanon(puzModified.chars, puzCanon.chars);
					//check whether it is an isomorph of a known puzzle
					puzzleSet::iterator p;
					p = puzzles.find(puzCanon);
					if(p == puzzles.end()) {
						//unknown single solution candidate
						//check for minimality (no redundant clues)
						//obtain the solution from the pencilmarks
						ch81 sol;
						for(int i = 0; i < 81; i++) {
							sol.chars[i] = Bitmap2Digit[pencilMarks2[i]];
						}
						int solBM[81];
						digit2bitmap(sol.chars, solBM);
						for(int i = 0; i < 81; i++) {
							int t = puzModified.chars[i];
							if(t) {
								puzModified.chars[i] = 0;
								unsigned long long nSol = solve(solBM, puzModified.chars, 2);
								puzModified.chars[i] = t;
								if(nSol == 1) {
									//given i is redundant
									goto nextValue;
								}
							}
						}
						//unknown single solution minimal puzzle
						//add to knowns
						puzzles.insert(puzCanon);
						newPuzzles.insert(puzCanon);
						//now apply some cheap transformations on the newfound
						//checkForTwins(puzModified, sol, solBM, puzzles, newPuzzles);
						//relabel();
					}
				}
				else { //nsol <= 0
					puzModified.toString(puzOriginalText.chars);
					printf("{+1}: no solution after setting a value from pencilmark: %81.81s\n", puzOriginalText.chars);
				}
nextValue:
				;
			}
			puzModified.chars[pos] = puz.chars[pos];
		}
	}
}

void doPlus(puzzleSet &newMultiSol, puzzleSet &puzzles, puzzleSet &newPuzzles, puzzleSet &multiSol) {
	for(puzzleSet::const_iterator p = newMultiSol.begin(); p!= newMultiSol.end(); p++) {
		doPlus(*p, puzzles, newPuzzles, multiSol, newMultiSol);
		//printf("+");
	}
}

void doPlusHC(ch81 &puzModifiedCanonical, puzzleSet &doneMS, puzzleSet &doneMSUnsaved, puzzleSet &valids, puzzleSet &validsUnsaved, puzzleSet &newMinimals) {
	ch81 puzCanon, ppuz[81*9];
	int n = solverPlus1(puzModifiedCanonical.chars, ppuz[0].chars);
	for(int i = 0; i < n; i++) {
		subcanon(ppuz[i].chars, puzCanon.chars);
		//check whether it is processed
		puzzleSet::const_iterator msplussed;
		msplussed = doneMS.find(puzCanon);
		if(msplussed == doneMS.end()) {
			//unprocessed plussed puzzle
			//here we can a) store the puzzle as "processed" in temporary cache, b) store in the persistent cache, c) don't store
			//doneMS.insert(puzCanon);
			//doneMSUnsaved.insert(puzCanon);
			////obtain number of solutions and check for single-solution
			//const int maxSol = 10000;
			//int nSol = solve(puzCanon.chars, maxSol);
			//if(nSol == maxSol) {
			//	fprintf(stderr, "hunt40: maximum number of %d solutions reached!", maxSol);
			//	exit(1);
			//}
			////check for minimality
			//for(int redundantPos = 0; redundantPos < 81; redundantPos++) {
			//	int rClue = puzCanon.chars[redundantPos];
			//	if(rClue == 0)
			//		continue; //not a clue
			//	puzCanon.chars[redundantPos] = 0;
			//	int nRedSol = solve(puzCanon.chars, nSol + 1);
			//	puzCanon.chars[redundantPos] = rClue; //restore the puzzle
			//	if(nSol == nRedSol) {
			//		//same number of solutions => clue at redundantPos is redundant
			//		goto nextPlus;
			//	}
			//}
			unsigned long long nSol = solverIsIrreducibleBySolCount(puzCanon.chars);
			if(0 == nSol) {
				//redundant clue found
				goto nextPlus;
			}
			//minimality test passed
			newMinimals.insert(puzCanon);
			doneMS.insert(puzCanon);
			if(nSol == 1) {
				//single-solution minimal puzzle
				puzzleSet::const_iterator ssplussed;
				ssplussed = valids.find(puzCanon);
				if(ssplussed == valids.end()) {
					//add to known valid puzzles
					valids.insert(puzCanon);
					validsUnsaved.insert(puzCanon);
				}
			}
			else {
				//multi-solution minimal puzzle
				doPlusHC(puzCanon, doneMS, doneMSUnsaved, valids, validsUnsaved, newMinimals);
			}
		}
nextPlus:
		;
	}
}

void doPlusMinimalUnique(puzzleSet &puzzles, const int depth) {
	int remain = depth - 1;
	puzzleSet done;
	puzzleSet more;
	size_t nPuzzles = puzzles.size();
	const ch81 ** const index = (const ch81 **)malloc(nPuzzles * sizeof(ch81 *));
	nPuzzles = 0;
	for(puzzleSet::iterator p = puzzles.begin(); p != puzzles.end(); p++) {
		index[nPuzzles++] = &(*p);
	}
	//suitable for parallelization
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif //_OPENMP
	for(unsigned int i = 0; i < nPuzzles; i++) {
		char p1[81 * 81 * 9];
		subCanoner sc;
		//int p1Size = solverPlus1(index[i]->chars, p1, true, false);
		int p1Size = solverPlus1(index[i]->chars, p1, true, remain == 0);
		if(p1Size) {
			puzzleSet ss;
			puzzleSet ms;
			//split uniques from non-uniques
			for(int j = 0; j < p1Size; j++) {
				ch81 pcan;
				sc.canon(&p1[81 * j], pcan.chars);
				if(remain) {
					if(1 == solve(pcan.chars, 2)) {
						//single solution, done
						ss.insert(pcan);
					}
					else {
						//multiple solutions, add more clues
						ms.insert(pcan);
					}
				}
				else {
					//last step, multiple-solution puzzles were filtered out in solverPlus1 above
					ss.insert(pcan);
				}
			}
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
			{
				more.insert(ms.begin(), ms.end());
				done.insert(ss.begin(), ss.end());
			}
		}
	}
	free(index);
	//at this point we have
	//- unnecessary source in puzzles
	//- puzzles ready for output in result
	//- puzzles to perform {+n} in more
	puzzles.clear();
	puzzles.swap(done); //return uniques
	if(remain && !more.empty()) {
		doPlusMinimalUnique(more, remain); //search for uniques
	}
	puzzles.insert(more.begin(), more.end()); //merge newly found uniques with previously obtained ones
}

int doMinus1(const ch81 &puz, ch81 *msCanon) {
	int n = 0;
	ch81 puzModified = puz;
	//apply -1
	for(int pos = 0; pos < 81; pos++) {
		int l1OldClue = puzModified.chars[pos];
		if(l1OldClue == 0) {
			continue; //not a clue
		}
		puzModified.chars[pos] = 0; //remove the clue
		//ch81 puzModifiedCanonical;
		subcanon(puzModified.chars, msCanon[n++].chars);
		puzModified.chars[pos] = l1OldClue; //add back the clue
	}
	return n;
}

void doMinus1(const ch81 &puz, puzzleSet &multiSol, puzzleSet &newMultiSol) {
	ch81 puzModified = puz;
	//apply -1
	for(int pos = 0; pos < 81; pos++) {
		int l1OldClue = puzModified.chars[pos];
		if(l1OldClue == 0) {
			continue; //not a clue
		}
		puzModified.chars[pos] = 0; //remove the clue
		puzzleSet::const_iterator msp;
		ch81 puzModifiedCanonical;
		subcanon(puzModified.chars, puzModifiedCanonical.chars);
		msp = multiSol.find(puzModifiedCanonical);
		if(msp == multiSol.end()) {
			multiSol.insert(puzModifiedCanonical);
			newMultiSol.insert(puzModifiedCanonical);
			//printf("+");
		}
		//else if(newMultiSol.size() < 50000) { //50000
		//	//printf("-");
		//	//count the clues
		//	int nClues = 0;
		//	for(int i = 0; i < 81; i++) {
		//		if(puzModifiedCanonical.chars[i])
		//			nClues++;
		//	}
		//	if(nClues == 37) {
		//		doMinus1(puzModifiedCanonical, multiSol, newMultiSol);
		//	}
		//}
		puzModified.chars[pos] = l1OldClue; //add back the clue
	}
}

void doMinus1(const ch81 &puz, puzzleSet &dest) {
	ch81 puzModified = puz;
	//apply -1
	for(int pos = 0; pos < 81; pos++) {
		int l1OldClue = puzModified.chars[pos];
		if(l1OldClue == 0) {
			continue; //not a clue
		}
		puzModified.chars[pos] = 0; //remove the clue
		ch81 puzModifiedCanonical;
		subcanon(puzModified.chars, puzModifiedCanonical.chars);
		dest.insert(puzModifiedCanonical);
		puzModified.chars[pos] = l1OldClue; //add back the clue
	}
}

void doMinus1(const puzzleSet &src, puzzleSet &dest) {
	for(puzzleSet::const_iterator testPuzzle = src.begin(); testPuzzle != src.end(); testPuzzle++) {
		doMinus1(*testPuzzle, dest);
	}
}

void doMinus1(const puzzleSet * const puzForMinus, puzzleSet &multiSol, puzzleSet &newMultiSol) {
	puzzleSet::const_iterator testPuzzle;
	for(testPuzzle = puzForMinus->begin(); testPuzzle != puzForMinus->end(); testPuzzle++) {
		doMinus1(*testPuzzle, multiSol, newMultiSol);
	}
}
void doMinus1(FILE * pFile, FILE * msFile) {
	char buf[1000];
	while(fgets(buf, sizeof(buf), pFile)) {
		ch81 puz, puzCanon[81];
		puz.fromString(buf);
		int n = doMinus1(puz, puzCanon);
		for(int i = 0; i < n; i++) {
			ch81 pText;
			puzCanon[i].toString(pText.chars);
			fprintf(msFile, "%81.81s\n", pText.chars);
		}
	}
}

void do9Minus8() {
	char buf[2000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		puz.fromString(buf);
		int givensPerDigit[10] = {0,0,0,0,0,0,0,0,0,0};
		int givensCells[10][9];
		for(int i = 0; i < 81; i++) {
			int d = puz.chars[i];
			if(d == 0) continue;
			givensCells[d][givensPerDigit[d]] = i;
			givensPerDigit[d]++;
		}
		for(int d = 1; d <= 9; d++) {
			if(givensPerDigit[d] != 9) continue;
			ch81 test = puz; //structure copy
			//clear all occurrences of d
			for(int i = 0; i < 9; i++) {
				test.chars[givensCells[d][i]] = 0;
			}
			//set a single cell to a given and test for uniqueness
			for(int i = 0; i < 9; i++) {
				test.chars[givensCells[d][i]] = d; //set as given
				if(1 == solve(test.chars, 2)) {
					ch81 txt;
					test.toString(txt.chars);
					printf("%81.81s\n", txt.chars);
				}
				test.chars[givensCells[d][i]] = 0; //back to non-given
			}
		}
	}
}

void do9Minus7() { //similar to minus8 but adding 2 clues at a time
	char buf[2000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		puz.fromString(buf);
		int givensPerDigit[10] = {0,0,0,0,0,0,0,0,0,0};
		int givensCells[10][9];
		for(int i = 0; i < 81; i++) {
			int d = puz.chars[i];
			if(d == 0) continue;
			givensCells[d][givensPerDigit[d]] = i;
			givensPerDigit[d]++;
		}
		for(int d = 1; d <= 9; d++) {
			if(givensPerDigit[d] != 9) continue;
			ch81 test = puz; //structure copy
			//clear all occurrences of d
			for(int i = 0; i < 9; i++) {
				test.chars[givensCells[d][i]] = 0;
			}
			//set two cells as givens and test for uniqueness
			for(int i = 0; i < 9 - 1; i++) {
				test.chars[givensCells[d][i]] = d; //set the first given
				for(int j = i + 1; j < 9; j++) {
					test.chars[givensCells[d][j]] = d; //set the second given
					if(1 == solve(test.chars, 2)) {
						ch81 txt;
						test.toString(txt.chars);
						printf("%81.81s\n", txt.chars);
					}
					test.chars[givensCells[d][j]] = 0; //back second to non-given
				}
				test.chars[givensCells[d][i]] = 0; //back first to non-given
			}
		}
	}
}

void do9Minus6() { //similar to minus7 but adding 3 clues at a time
	char buf[2000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		puz.fromString(buf);
		int givensPerDigit[10] = {0,0,0,0,0,0,0,0,0,0};
		int givensCells[10][9];
		for(int i = 0; i < 81; i++) {
			int d = puz.chars[i];
			if(d == 0) continue;
			givensCells[d][givensPerDigit[d]] = i;
			givensPerDigit[d]++;
		}
		for(int d = 1; d <= 9; d++) {
			if(givensPerDigit[d] != 9) continue;
			ch81 test = puz; //structure copy
			//clear all occurrences of d
			for(int i = 0; i < 9; i++) {
				test.chars[givensCells[d][i]] = 0;
			}
			//set three cells as givens and test for uniqueness
			for(int i = 0; i < 9 - 2; i++) {
				test.chars[givensCells[d][i]] = d; //set the first given
				for(int j = i + 1; j < 9 - 1; j++) {
					test.chars[givensCells[d][j]] = d; //set the second given
					for(int k = j + 1; k < 9; k++) {
						test.chars[givensCells[d][k]] = d; //set the third given
						if(1 == solve(test.chars, 2)) {
							ch81 txt;
							test.toString(txt.chars);
							printf("%81.81s\n", txt.chars);
						}
						test.chars[givensCells[d][k]] = 0; //back third to non-given
					}
					test.chars[givensCells[d][j]] = 0; //back second to non-given
				}
				test.chars[givensCells[d][i]] = 0; //back first to non-given
			}
		}
	}
}

void do9Minus5() { //similar to minus6 but adding 4 clues at a time
	char buf[2000];
	while(fgets(buf, sizeof(buf), stdin)) {
		ch81 puz;
		puz.fromString(buf);
		int givensPerDigit[10] = {0,0,0,0,0,0,0,0,0,0};
		int givensCells[10][9];
		for(int i = 0; i < 81; i++) {
			int d = puz.chars[i];
			if(d == 0) continue;
			givensCells[d][givensPerDigit[d]] = i;
			givensPerDigit[d]++;
		}
		for(int d = 1; d <= 9; d++) {
			if(givensPerDigit[d] != 9) continue;
			ch81 test = puz; //structure copy
			//clear all occurrences of d
			for(int i = 0; i < 9; i++) {
				test.chars[givensCells[d][i]] = 0;
			}
			//set three cells as givens and test for uniqueness
			for(int i = 0; i < 9 - 3; i++) {
				test.chars[givensCells[d][i]] = d; //set the first given
				for(int j = i + 1; j < 9 - 2; j++) {
					test.chars[givensCells[d][j]] = d; //set the second given
					for(int k = j + 1; k < 9 - 1; k++) {
						test.chars[givensCells[d][k]] = d; //set the third given
						for(int l = k + 1; l < 9; l++) {
							test.chars[givensCells[d][l]] = d; //set the fourth given
							if(1 == solve(test.chars, 2)) {
								ch81 txt;
								test.toString(txt.chars);
								printf("%81.81s\n", txt.chars);
							}
							test.chars[givensCells[d][l]] = 0; //back fourth to non-given
						}
						test.chars[givensCells[d][k]] = 0; //back third to non-given
					}
					test.chars[givensCells[d][j]] = 0; //back second to non-given
				}
				test.chars[givensCells[d][i]] = 0; //back first to non-given
			}
		}
	}
}

void doMinusPlusMinimalUnique(int minusDepth, puzzleSet &puzzles) {
	puzzleSet minused;
	puzzleSet *src = &puzzles;
	puzzleSet *dest = &minused;
	puzzleSet *tmp;
	int plusDepth = minusDepth;
	if(minusDepth >= 10) {
		plusDepth = minusDepth % 10;
		minusDepth /= 10;
	}
	else {
		//apply {-0} {+1..given_one_digit_depth}
		minusDepth = 0;
	}
	for(int i = 0; i < minusDepth; i++) {
		doMinus1(*src, *dest); //dest = src - 1
		src->clear();
		tmp = dest;
		dest = src;
		src = tmp;
	}
	//at this point we have src pointing to {-n} and empty dest
	doPlusMinimalUnique(*src, plusDepth);
	if(src != &puzzles) {
		puzzles.swap(*src);
	}
}

void hiClue(const char *puzFName, const char *mspuzFName) {
	//read known puzzles
	puzzleSet puzzles;
	puzzleSet newPuzzles;
	puzzleSet *puzForMinus;
	puzzles.loadFromFile(puzFName);
	if(puzzles.empty())
		return;

	int pass = 0; //debug

	//checkForTwins(puzzles, newPuzzles);
	//if(!newPuzzles.empty()) {
	//	printf("%d twins found within original puzzles.\n", newPuzzles.size());
	//	//save the puzzles
	//	newPuzzles.appendToFile(puzFName);
	//	newPuzzles.appendToFile("new_puzzles.txt");
	//}

	//read known multi-solution pseudopuzzles
	puzzleSet multiSol;
	multiSol.loadFromFile(mspuzFName);

	puzForMinus = &puzzles; //at first pass apply {-} to all knowns

	//apply {-1} to known puzzles and obtain unknown multi-solution pseudopuzzles
	puzzleSet newMultiSol;
	while(!puzForMinus->empty()) { //until closure
		doMinus1(puzForMinus, multiSol, newMultiSol);

		//apply {+} to unknown multi-solution pseudopuzzles
		newPuzzles.clear();
		doPlus(newMultiSol, puzzles, newPuzzles, multiSol);
		puzForMinus = &newPuzzles; //from now on process only newfound puzzles
		pass++;
		if(!newPuzzles.empty()) {
			//save the puzzles
			newPuzzles.appendToFile(puzFName);
			newPuzzles.appendToFile("new_puzzles.txt");
		}
		if(!newMultiSol.empty()) {
			//save the multi-solution pseudopuzzles
			newMultiSol.appendToFile(mspuzFName);
			newMultiSol.appendToFile("new_puzzlesMS.txt");
		}
		printf("pass=%d, new multi-solution ppuzzles=%d, new puzzles=%d\n", pass, (int)newMultiSol.size(), (int)newPuzzles.size());
		newMultiSol.clear();
	}
}

void hiClueNoMSCache(const char *mspuzFName, const char *knownsFName, const char *newFName) {
	//read known puzzles
	puzzleSet puzzles;
	puzzleSet newPuzzles;
	puzzleSet msPuzzles;
	//puzzleSet *puzForMinus;
	//puzzles.loadFromFile(knownsFName);
	puzzles.loadFromFile(knownsFName, false); //we are sure all puzzles are already canonicalized
	//if(puzzles.empty())
	//	return;

	puzzleSet::size_type nNewPuzzles = 0;
	int nProcessedMS = 0;

	//process known multi-solution pseudopuzzles
	FILE *pfile;
	if(mspuzFName == NULL)
		return;
	pfile = fopen(mspuzFName, "rt");
	if(pfile == NULL) {
		printf("error: Can't open file %s", mspuzFName);
		return;
	}
	char buf[1000];
	while(fgets(buf, sizeof(buf), pfile)) {
		ch81 puz;
		puz.fromString(buf);
		doPlus(puz, puzzles, newPuzzles, msPuzzles);
		if(!newPuzzles.empty()) {
			//save the puzzles
			//newPuzzles.appendToFile(knownsFName);
			newPuzzles.appendToFile(newFName);
			nNewPuzzles += newPuzzles.size();
			newPuzzles.clear();
		}
		nProcessedMS++;
		//if(nProcessedMS >= 1000000) break; //debug
	}
	fclose(pfile);

	printf("Processed %d multi-solution pseudos, found %d new puzzles\n", nProcessedMS, (int)nNewPuzzles);
}

int hunt40 (const char *puzFName) { //Apply {-1} to seed then {+} targeting high clue minimal multi-solution puzzles.
	int ret = 0; //exit after closure
	char buf[2000];
	//load already processed multi-solution puzzles from x.donems.txt
	//Only part of the puzzles is stored permanently on file. (startup puzzles for {+})
	//During the execution all processed puzzles are added to collection to minimize duplication
	//When collection approaches 2GB (32-bit windows process RAM limitation) the cache is reduced.
	puzzleSet doneMS, doneMSUnsaved;
	FILE *fDoneMS;
	strcpy(buf, puzFName);
	strcat(buf, ".donems.txt");
	fDoneMS = fopen(buf, "a+t");
	//doneMS.loadFromFile(fDoneMS, false, 38);
	doneMS.loadFromFile(fDoneMS, false);
	//load known valid puzzles from x.puz.txt
	//These are known minimal unique puzzles.
	//Newborn valids are appended here.
	puzzleSet valids, validsUnsaved;
	FILE *fValid;
	strcpy(buf, puzFName);
	strcat(buf, ".puz.txt");
	fValid = fopen(buf, "a+t");
	valids.loadFromFile(fValid, false);
	//load known minimal multi-solution puzzles from x.mspuz.txt
	//This is the target collection of minimal multi-solution puzzles which can't grow up.
	//Seed puzzles (see below) are forced to be processed again.
	puzzleSet knownMS;
	FILE *fknownMS;
	strcpy(buf, puzFName);
	strcat(buf, ".mspuz.txt");
	fknownMS = fopen(buf, "a+t");
	knownMS.loadFromFile(fknownMS, false);
	//load unprocessed multi-solution puzzles from x.newms.txt
	//This is the "seed".
	//All minimals are used as seed for next pass.
	//The process terminates after the seed exhausted. (closure)
	//Initial seed must be obtained from external methoods.
	puzzleSet newMS, newMinimals;
	FILE *fNewMS;
	strcpy(buf, puzFName);
	strcat(buf, ".newms.txt");
	fNewMS = fopen(buf, "rt");
	newMS.loadFromFile(fNewMS, false);
	fclose(fNewMS);
	//append knowns to processed list to avoid duplicates in the output
	doneMS.insert(valids.begin(), valids.end());
	//doneMS.insert(newMS.begin(), newMS.end()); //this suppresses external seed to be added 
	doneMS.insert(knownMS.begin(), knownMS.end());
	//knownMS.clear();
	//process the initial givens
	while(!newMS.empty()) {
		//apply -1 to unprocessed puzzles
		puzzleSet::size_type nPuzzles = newMS.size();
		puzzleSet::size_type nProgress = 1 + (nPuzzles / 30);
		printf("%d ppuz ", (int)nPuzzles);
		//ch81 puzModified;
		puzzleSet::iterator testPuzzle;
		for(testPuzzle = newMS.begin(); testPuzzle != newMS.end(); testPuzzle++) {
			ch81 puzModified = *testPuzzle;
			//update progress
			nPuzzles--;
			if((nPuzzles % nProgress) == 0)
				printf(".");
			//if(doneMS.find(puzModified) != doneMS.end()) //debug
			//	continue;
			//apply -1
			for(int pos = 0; pos < 81; pos++) {
				int m1OldClue = puzModified.chars[pos];
				if(m1OldClue == 0) {
					continue; //not a clue
				}
				puzModified.chars[pos] = 0; //remove the clue
				ch81 puzModifiedCanonical;
				subcanon(puzModified.chars, puzModifiedCanonical.chars);
				if(doneMS.find(puzModifiedCanonical) == doneMS.end()) {
					//unprocessed multi-solution puzzle found
					//apply +
					doPlusHC(puzModifiedCanonical, doneMS, doneMSUnsaved, valids, validsUnsaved, newMinimals);
					//store the puzzle as processed (for +)
					doneMS.insert(puzModifiedCanonical);
					doneMSUnsaved.insert(puzModifiedCanonical);
				}
				puzModified.chars[pos] = m1OldClue; //add back the clue
			}
			if(doneMS.size() > 19400000) { //~1.9GB
				printf("(Cache overflow)");
				for(int i = 0; i < 1000000; i++)
					doneMS.erase(doneMS.begin());
				//testPuzzle++;
				//newMinimals.insert(testPuzzle, newMS.end()); //add still unprocessed puzzles from old seed to the newly created seed
				//break; //doneMS.size() > 18800000 in the outer loop will force restarting
			}
		}
		newMS.clear();
		//store the newborns
		doneMSUnsaved.saveToFile(fDoneMS); //append "plussed" puzzles to file
		validsUnsaved.saveToFile(fValid); //append newborn valid puzzles to file
		//temporary store in newMS items from newMinimals not duplicated in knownMS, then append them to knownMS file
		for(testPuzzle = newMinimals.begin(); testPuzzle != newMinimals.end(); testPuzzle++) {
			if(knownMS.find(*testPuzzle) == knownMS.end()) {
				newMS.insert(*testPuzzle);
			}
		}
		newMS.saveToFile(fknownMS); //append newborn multi-solution minimal puzzles to file
		knownMS.insert(newMS.begin(), newMS.end()); //append saved knowns to exclude duplicates in the file
		newMS.clear();
		fflush(NULL); //commit all writes
		printf(" %d grown, %d minimal, %d unique.\n", (int)doneMSUnsaved.size(), (int)newMinimals.size(), (int)validsUnsaved.size());
		validsUnsaved.clear();
		doneMSUnsaved.clear();
		fNewMS = fopen(buf, "wt");
		newMinimals.saveToFile(fNewMS); //overwrite the seed file
		fclose(fNewMS);
		fflush(NULL); //commit all writes
		newMS.swap(newMinimals);
		//suppress out of memory due to huge doneMS size
		if(doneMS.size() > 19200000 && !newMS.empty()) { //~1.9GB
			printf("Restarting to cleanup the cache.\n");
			ret = 1;
			break;
		}
	}
	if(ret == 0)
		printf("Closure.\n");
	//doneMS.clear();
	//valids.clear();
	//newMinimals.clear();
	//knownMS.clear();
	return ret;
}

int relCallBack(const char *puz, void *context) {
	ch81 pText;
	pText.toString(puz, pText.chars);
	printf("%81.81s\n", pText.chars);
	fflush(NULL);
	return 0;
}
extern int doSimilarPuzzles () {
	const char *pFName = opt.similarOpt->puzFile;
	const char *knownsfname = opt.similarOpt->knownsFName;
	const char *msFName = opt.similarOpt->mspuzzles;
	const char *m1pFName = opt.similarOpt->minus1plus;
	int depth = opt.similarOpt->depth;
	if(opt.similarOpt->bandminlex) { //--similar --bandminlex < puzzles.txt > out.txt
#if 0
		char buf[1000];

		//band automorphism
		//transformer tt;
		//for(int i = 0; i < 416; i++) {
		//	tt.byBand(bands[i], 0);
		//	printf("%d\t%d\n", i, tt.aut);
		//}
		//return 0;

		int bbCount[416][3][416][3]; //band1, boxinband1 crosses band2, boxinband2; band1 <= band2
		memset(bbCount, 0, sizeof(bbCount));
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz, sol, out, solTr;
			int nClues = puz.fromString(buf);
			//fprintf(stdout, "%81.81s\t%d", buf, nClues);
			unsigned long long nSol = solve(puz.chars, 1, sol.chars);
			if(nSol == 0) {
				fprintf(stdout, "\tInvalid puzzle");
				continue;
			}
			int bb[6];
			int crossReordering[6][3];
			transformer tr[6];
			for(int band = 0; band < 6; band++) {
			//int higherMin = 0;
			//int lowerMax = 100;
			//for(int band = 0; band < 6; band+=3) {
				tr[band].byBand(sol.chars, band);
				tr[band].transform(puz.chars, out.chars);
				tr[band].transform(sol.chars, solTr.chars);
				out.toString(buf);
				//int pop1 = 0, pop2 = 0;
				int b416 = getBandNum(solTr.chars);
				bb[band] = b416 - 1;
				if(band > 2) {
					crossReordering[band][bandByCellIndex[tc.swap[tr[band].box][tr[band].row[0]][tr[band].col[0]]]] = 0; //original box mapped to box 0
					crossReordering[band][bandByCellIndex[tc.swap[tr[band].box][tr[band].row[0]][tr[band].col[3]]]] = 1;
					crossReordering[band][bandByCellIndex[tc.swap[tr[band].box][tr[band].row[0]][tr[band].col[6]]]] = 2;
				}
				//else {
				//	crossReordering[band][stackByCellIndex[tc.swap[tr[band].box][tr[band].row[0]][tr[band].col[0]]]] = 0; //original box mapped to box 0
				//	crossReordering[band][stackByCellIndex[tc.swap[tr[band].box][tr[band].row[0]][tr[band].col[3]]]] = 1;
				//	crossReordering[band][stackByCellIndex[tc.swap[tr[band].box][tr[band].row[0]][tr[band].col[6]]]] = 2;
				//}
				//for(int i = 0; i < 27; i++) {
				//	if(out.chars[i])
				//		pop1++;
				//	if(out.chars[i + 27])
				//		pop2++;
				//}
				//int pop3 = nClues - pop1 - pop2;
				//int min = pop1;
				//int max = pop1;
				//if(min > pop2) min = pop2;
				//if(max < pop2) max = pop2;
				//if(min > pop3) min = pop3;
				//if(max < pop3) max = pop3;
				//if(higherMin < min) higherMin = min;
				//if(lowerMax > max) lowerMax = max;
				//solTr.toString(buf);
				//fprintf(stdout, "%3.3d\t%27.27s\t%27.27s\t%27.27s\n", b416, buf, buf + 27, buf + 54);
				////swapBands23(buf);
				////fprintf(stdout, "%3.3d\t%d\t%27.27s\t%27.27s\t%27.27s\n", b416, pop, buf, buf + 27, buf + 54);
				//if(band == 4 && ((bb[4] == 380 && bb[1] == 13) || (bb[4] == 13 && bb[1] == 380)))
				//	fprintf(stdout, "%3.3d\t%27.27s\t%27.27s\t%27.27s\n", b416, buf, buf + 27, buf + 54);
			}
			for(int i = 0; i < 3; i++) {
				for(int j = 0; j < 3; j++) {
					//int s = 3 + crossReordering[i][j]; // band i[0..2] crosses original stack s[3..5] at permuted box j[0..2]
					int s = 3 + stackByCellIndex[tc.swap[tr[i].box][tr[i].row[0]][tr[i].col[3 * j]]];
					bbCount[bb[i]][j][bb[s]][crossReordering[s][i]]++;
					//int sb; //stack s[3..5] crosses band i[0..2] at permuted box sb[0..2]
					//for(sb = 0; sb < 3; sb++) {
					//	if(i == crossReordering[s][sb]) {
					//		bbCount[bb[i]][j][bb[s]][sb]++;
					//		break;
					//	}
					//}
				}
			}
			//fprintf(stdout, "\n");
			//fprintf(stdout, "%d %d\n", higherMin, lowerMax);
		}
		//normalize band1index to be <= band2index
#if defined(__INTEL_COMPILER)
#pragma noparallel
#endif //__INTEL_COMPILER
		for(int i = 0; i < 416; i++) {
			for(int j = 0; j < i; j++) {
				for(int k = 0; k < 3; k++) {
					for(int l = 0; l < 3; l++) {
						bbCount[j][l][i][k] += bbCount[i][k][j][l];
						bbCount[i][k][j][l] = 0;
					}
				}
			}
		}
		//for band1index == band2index normalize band1box to be <= band2box
#if defined(__INTEL_COMPILER)
#pragma noparallel
#endif //__INTEL_COMPILER
		for(int i = 0; i < 416; i++) {
			for(int k = 0; k < 2; k++) {
				for(int l = k + 1; l < 3; l++) {
					bbCount[i][k][i][l] += bbCount[i][l][i][k];
					bbCount[i][l][i][k] = 0;
				}
			}
		}
		for(int i = 0; i < 416; i++) {
			for(int j = i; j < 416; j++) {
			//for(int j = 0; j < 416; j++) { //debug
				for(int k = 0; k < 3; k++) {
					for(int l = 0; l < 3; l++) {
						if(bbCount[i][k][j][l]) {
							fprintf(stdout, "%3.3d\t%d\t%3.3d\t%d\t%d\n", i + 1, k + 1, j + 1, l + 1, bbCount[i][k][j][l]);
						}
					}
				}
			}
		}
#endif
	}
	else if(depth >= 0) { //--similar --relabel <depth> [--minimals]
		char buf[1000];
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz;
			puz.fromString(buf);
			solverRelabel(puz.chars, depth, opt.similarOpt->minimals, opt.similarOpt->unique, opt.similarOpt->nosingles, relCallBack, NULL);
		}
	}
	else if(opt.similarOpt->plus1) { //--similar --plus1 [--subcanon] [--unique] [--minimals] [--knownpuzzles <file>]< x.txt > y.txt
		char buf[1000];
		puzzleSet puzzles;
		puzzleSet newPuzzles;
		if(knownsfname) {
			puzzles.loadFromFile(pFName, opt.similarOpt->subcanon); // --knownpuzzles w/o --subcanon expects canonicalized knowns
			opt.similarOpt->subcanon = true;
		}
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz, puzCanon, ppuz[81*9];
			puz.fromString(buf);
			//int n = solverPlus1Unique(puz.chars, ppuz[0].chars);
			int n = solverPlus1(puz.chars, ppuz[0].chars, opt.similarOpt->minimals, opt.similarOpt->unique);
			for(int i = 0; i < n; i++) {
				if(opt.similarOpt->subcanon) {
					subcanon(ppuz[i].chars, puzCanon.chars);
					if(knownsfname) {
						if(newPuzzles.find(puzCanon) != newPuzzles.end()) { //not found
							if(puzzles.find(puzCanon) != puzzles.end()) { //not in knowns
								newPuzzles.insert(puzCanon);
							}
						}
						continue;
					}
					puzCanon.toString(buf);
				}
				else {
					ppuz[i].toString(buf);
				}
				fprintf(stdout, "%81.81s\n", buf);
			}
			if(n && !knownsfname)
				fflush(NULL);
		}
		if(knownsfname) {
			if(!newPuzzles.empty()) {
				newPuzzles.saveToFile(stdout);
			}
			puzzles.clear();
			newPuzzles.clear();
		}
	}
	else if(opt.similarOpt->minusDepth != -1) { //--similar --minusandup 13 < x.txt > y.txt (13 = {-1}{+1..3})
		puzzleSet puzzles;
		puzzles.loadFromFile(stdin); // always canonicalize the input
		doMinusPlusMinimalUnique(opt.similarOpt->minusDepth, puzzles); //result is returned in the place of source
		puzzles.saveToFile(stdout);
	}
	else if(opt.similarOpt->plus2) { //--similar --plus2 < x.txt > y.txt
		//bug: 64-bit compilation crashes for ................12.12.34.56.....7..8.48365.273.7.8.56...1.4..8..34.18.7..8.673..1
		char buf[1000];
		const int maxChunkSize = 24 * 36;
		ch81 puz[maxChunkSize];
		int chunkSize;
		do {
			chunkSize = 0;
			while((chunkSize < maxChunkSize) && fgets(buf, sizeof(buf), stdin)) {
				puz[chunkSize++].fromString(buf);
			}
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif //_OPENMP
			for(int i = 0; i < chunkSize; i++) {
				ch81 ppuz[81*80*9]; //increase stack limit on linux "ulimit -s 16000"
				int n = solverPlus2(puz[i].chars, ppuz[0].chars);
				if(n)
#ifdef _OPENMP
#pragma omp critical
#endif //_OPENMP
				{
					for(int j = 0; j < n; j++) {
						ppuz[j].toString(buf);
						fprintf(stdout, "%81.81s\n", buf);
					}
					fflush(NULL);
				}
			}
		} while(chunkSize == maxChunkSize);
	}
	else if(m1pFName) { //--similar --minus1plus x
		while(hunt40(m1pFName)); //loop until closure
	}
	//else if(opt.similarOpt->invert) { //--similar --invert <grid | .> < x.txt > y.txt //update 8 May 2017
	else if(opt.similarOpt->invert) { //--similar --invert <grid | . | a | d> < x.txt > y.txt
		const int maxSol = 1000000;
		char buf[1000];
		//int fixedGrid = opt.similarOpt->invert[1]; //grid is given
		int invertmode = 0;
		if(opt.similarOpt->invert[0] == '.') {
			//output any of the completions
			invertmode = 1;
		}
		else if(opt.similarOpt->invert[0] == 'a') {
			//output all completions
			invertmode = 2;
		}
		else if(opt.similarOpt->invert[0] == 'd') {
			//output essentially different completions
			invertmode = 3;
		}
		else if(opt.similarOpt->invert[0] >= '1' && opt.similarOpt->invert[0] <= '9') {
			//validate provided completion and output the inverse subgrid
			invertmode = 4;
		}
		if(invertmode == 0) {
			fprintf(stderr, "incorrect invert option\n");
			return 1;
		}
		//ch81 gr[maxSol];
		ch81 *gr = new ch81[maxSol];
		if(invertmode == 4) {
			gr[0].fromString(opt.similarOpt->invert);
		}
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz, ppuz;
			puz.fromString(buf);
            int nSol;
            //prepare solutions in array gr
            switch(invertmode) {
            	case 1: //any completion
                    nSol = (int)solve(puz.chars, 1, gr[0].chars); // first solution
    				if(nSol == 0) {
    					//unsolvable puzzle
    					printf("incorrect puzzle: %81.81s\n", buf);
    					delete [] gr;
    					return 1;
    				}
            		break;
            	case 2: //all completions
            	case 3: //essentially different completions
                    nSol = (int)solve(puz.chars, maxSol, gr[0].chars); // all solutions
    				if(nSol == maxSol) {
    					//weak puzzle
    					printf("The maximum of %d solutions reached: %81.81s\n", maxSol, buf);
    					delete [] gr;
    					return 1;
    				}
    				if(nSol == 0) {
    					//unsolvable puzzle
    					printf("The puzzle has no valid completions: %81.81s\n", buf);
    					delete [] gr;
    					return 1;
    				}
//    				if(invertmode == 3) { //ED solutions
//    					//filter out isomorphs
//    					puzzleSet can;
//    					for(int n = 0; n < nSol; n++) {
//    						rowminlex(gr[n].chars, ppuz.chars); //slow
//    						std::pair<puzzleSet::iterator, bool> res = can.insert(ppuz);
//    						if(!res.second) {
//    							//duplicate, mark it by resetting the first value to 0
//    							gr[n].chars[0] = 0;
//    						}
//    					}
//    				}
            		break;
            	default:
            		//do nothing when grid is given
            		break;
            }
			puzzleSet can;
            for(int n = 0; n < nSol; n++) { //print all solutions
            	//if(gr[n].chars[0] == 0) continue; //skip solutions marked as duplicate
                for(int i = 0; i < 81; i++) {
                    if(puz.chars[i]) {
                        ppuz.chars[i] = 0; //clear the given
                    }
                    else {
                        ppuz.chars[i] = gr[n].chars[i]; //set the non-given from the solution grid
                    }
                }
				if(invertmode == 3) { //ED solutions
					//filter out isomorphs
					ch81 c;
					//subcanon(ppuz.chars, c.chars);
					patminlex pml(ppuz.chars, c.chars);
					std::pair<puzzleSet::iterator, bool> res = can.insert(c);
					if(!res.second) { //already printed?
						continue;
					}
				}
                ppuz.toString(buf);
                fprintf(stdout, "%81.81s\n", buf);
            }
		}
		delete [] gr;
	}
	else if(opt.similarOpt->removeredundant) { //--similar --removeredundant < x.txt > y.txt
		char buf[1000];
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz;
			puz.fromString(buf);
			minimize(puz);
		}
	}
	else if(opt.similarOpt->minus1) { //--similar --minus1 < 37all.txt > 37allm1.txt
		//apply unconditional {-1} to all puzzles in stdin
		FILE *pfile;
		pfile = stdin;
		FILE *msfile;
		msfile = stdout;
		doMinus1(pfile, msfile);
	}
	else if(opt.similarOpt->minus8) { //--similar --9minus8 < 999911110.txt > 999111110.txt
		//for digits with 9 givens from stdin check whether after removal of 8 of them the puzzle still has unique solution
		do9Minus8();
	}
	else if(opt.similarOpt->minus7) { //--similar --9minus7 < 999911110.txt > 999211110.txt
		//for digits with 9 givens from stdin check whether after removal of 7 of them the puzzle still has unique solution
		do9Minus7();
	}
	else if(opt.similarOpt->minus6) { //--similar --9minus6 < 999911110.txt > 999311110.txt
		//for digits with 9 givens from stdin check whether after removal of 6 of them the puzzle still has unique solution
		do9Minus6();
	}
	else if(opt.similarOpt->minus5) { //--similar --9minus5 < 999911110.txt > 999411110.txt
		//for digits with 9 givens from stdin check whether after removal of 5 of them the puzzle still has unique solution
		do9Minus5();
	}
	else if(opt.similarOpt->twins) { //--similar --twins [--subcanon] [--minimals]
		puzzleSet puzzles;
		puzzles.loadFromFile(stdin, opt.similarOpt->subcanon);
		puzzleSet newPuzzles;
		newPuzzles.insert(puzzles.begin(), puzzles.end());
		checkForTwins(puzzles, newPuzzles, opt.similarOpt->minimals);
		for(puzzleSet::const_iterator p = puzzles.begin(); p != puzzles.end(); p++) {
			newPuzzles.erase(*p);
		}
		if(!newPuzzles.empty()) {
			newPuzzles.saveToFile(stdout);
		}
	}
	else if(opt.similarOpt->cousins) { //--similar --cousins < x.txt > y.txt
		char buf[1000];
		while(fgets(buf, sizeof(buf), stdin)) {
			ch81 puz;
			puz.fromString(buf);
			cousins(puz);
		}
	}
	else if(opt.similarOpt->subcanon) { //--similar --subcanon --puzzles puz.txt --mspuzzles puzcanon.txt
		if(pFName && msFName) {
			puzzleSet puzzles;
			puzzles.loadFromFile(pFName);
			puzzles.saveToFile(msFName);
			puzzles.clear();
		}
		else { //--similar --subcanon < puz.txt > puzcanon.txt
			char buf[1000];
			while(fgets(buf, sizeof(buf), stdin)) {
				ch81 puz, puzCanon;
				puz.fromString(buf);
				//subcanon2(puz.chars, puzCanon.chars);
				subcanon(puz.chars, puzCanon.chars);
				puzCanon.toString(puz.chars);
				fprintf(stdout, "%81.81s\n", puz.chars); //fprintf(stdout, "%81.81s\t%81.81s\n", buf, puz.chars);
			}
		}
	}
	else if(opt.similarOpt->clusterize) { //--similar --clusterize < puz.txt > clusters.txt
		clusterize();
	}
	else if(msFName) { //--similar --mspuzzles 36MS_new_ordered.txt --knownpuzzles puzAll_can.txt [--puzzles puzzles_new.txt]
		hiClueNoMSCache(msFName, knownsfname, pFName); //apply {+} to mspuzzles, add new to knownpuzzles and puzzles
	}
	else { //--similar --knownpuzzles puzAll_can.txt --puzzles puzzles_new.txt
		hiClue(pFName, knownsfname); //apply {-} to puzzles until 37, then {+}, add new to knownpuzzles and puzzles
	}
	return 0;
}

//void test() { //sort canonicalized puzzles and discard duplicates
//	puzzleSet puzzles;
//	puzzles.loadFromFile(stdin, false);
//	puzzles.saveToFile(stdout);
//}
