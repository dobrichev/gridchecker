/*
Copyright (c) 2011, OWNER: Gérard Penet
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

Neither the name of the OWNER nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
// RatingEngine

#include "flog.h"				// relay for printing only in tests
#include "opsudo.h"				// storing and managing options
#include "puzzle.h"				// general class to solve a puzzle

#include "utilities.h"
#include "ratingengine.h"

namespace skfr {

OPSUDO Op;
FLOG EE;  //log file as such is one should not be used in multi thread


// puzzle dependant variables to move

//PUZZLE puz;  // must be at the end one occurence per thread



void setMinMaxC(int mined, int maxed, int minep, int maxep, int miner, int maxer, UINT filt){
	Op.mined = mined;
	Op.maxed = maxed;
	Op.minep = minep;
	Op.maxep = maxep;
	Op.miner = miner;
	Op.maxer = maxer;
	Op.filters.SetAll_0();
	Op.filters.f = filt;
}

void setParamC (int o1, int delta, int os, int oq, int ot, int oexclude, int edcycles)
{
	Op.o1=o1;
	Op.delta=delta;
	Op.os=os;
	Op.oq=oq;
	Op.ot=ot;
	Op.oexclude=oexclude;
	Op.edcycles=edcycles;
}
int CallOpenLog(char * name) {return EE.OpenFL(name);}

// in wait state, something is wrong in that process;
// replaced by on unique call in batch start

#pragma message ("setTestModeC(int, char *) is not thread-safe")
int setTestModeC (int ot, char * logFileName){
	static char * actualLogFileName = 0;

	// if already in test mode and new log file name => close previous one and open the nex one
	// if enter in test mode => open log file
	// if exit from test mode => close log file
	// return error code 0 ok, 1 not able to open log file
	int rc =0;
	if (logFileName==0 && ot!=0) return 1; //error
	if (actualLogFileName !=0){
		if (Op.ot)
		{
			if (strcmp(actualLogFileName,logFileName)==0) 
				return 0;
			else
				EE.CloseFL();
		}
	}
	Op.ot = ot;
	if (Op.ot){
		EE.OpenFL(logFileName);
		actualLogFileName= logFileName;
	}
	else
	{
		EE.CloseFL();
		actualLogFileName=0;
	}
	return rc;
}

//! Process a puzzle
/**
 * Normalize puzzle (empty cell '0') in global variable <code>PUZZLE</code>.<br>
 * Verify that puzzle is correct (no duplicate given in a house).<br>
 * Verify that puzzle is valid (one and only one solution).<br>
 * Keep the solution as a string and as positions of the 9 digits
 * Launch the processing
 * 
 * @return 0 if error or return from <code>PUZZLE::Traite</code>
 */
int ratePuzzleC(char *ze, int * er, int * ep, int * ed, int * aig)
{
	PUZZLE puz; //instantiate a puzzle
	puz.options = Op; //clone the options
	// do standard processing
	int rc = puz.Traite(ze);
	*er = puz.ermax;
	*ep = puz.epmax;
	*ed = puz.edmax;
	*aig = puz.stop_rating;
	return  rc;
}

void ratePuzzlesC(int nPuzzles, char *ze, int *er, int *ep, int *ed, int *aig, int *ir) {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
	for(int i = 0; i < nPuzzles; i++) {
		ir[i] = ratePuzzleC(&ze[i * 82], &er[i], &ep[i], &ed[i], &aig[i]);
	}
}

//void rateOnePuzzle(char *ze, int * er, int * ep, int * ed)
void rateOnePuzzle(puzzleToRate &p)
{
	//OPSUDO op; //default options
	PUZZLE puz; //instantiate a puzzle
	//puz.options = op; //clone the options
	//do standard processing
	puz.Traite(p.p);
	p.er = puz.ermax;
	p.ep = puz.epmax;
	p.ed = puz.edmax;
	//clear ratings on error
	if(puz.stop_rating || p.er < 10 || p.er > 120)
		p.er = p.ep = p.ed = 0;
}

//void rateManyPuzzles(int nPuzzles, char *ze, int *er, int *ep, int *ed) {
void rateManyPuzzles(int nPuzzles, puzzleToRate *p) {
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
	for(int i = 0; i < nPuzzles; i++) {
		rateOnePuzzle(p[i]);
	}
}

} //namespace skfr