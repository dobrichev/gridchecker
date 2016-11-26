/*
Copyright (c) 2011, OWNER: G�rard Penet
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

namespace skfr {

// rating engine interface (C or C++)
int ratePuzzleC(char *ze, int *er, int *ep, int *ed, int *aig);
void setMinMaxC(int mined, int maxed, int minep, int maxep, int miner, int maxer, unsigned int filt);
void setParamC (int o1, int delta, int os, int oq, int ot, int oexclude, int edcycles);
int setTestModeC (int ot, char *logFileName);

void ratePuzzlesC(int nPuzzles, char *ze, int *er, int *ep, int *ed, int *aig, int *ir); //parallel rating

int PrintOptionsOpsudo();

struct puzzleToRate {
	int er;
	int ep;
	int ed;
	char p[81];
};

void rateManyPuzzles(int nPuzzles, puzzleToRate *p);
void rateOnePuzzle(puzzleToRate &p);

} //namespace skfr
