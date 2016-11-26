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
// utilities
//#include "utilities.h"
#include <time.h>
//#include <sys\timeb.h>
#include <string.h> 

namespace skfr {

void strcpy_s(char *d, int size, const char *s) {
	strncpy(d, s, size);
}

void strncpy_s(char *d, int size, const char *s, int n) {
	strncpy(d, s, size < n ? size : n);
}

// catching time as seconds+millis  (seconds since year 1970)
long GetTimeMillis() {
	return (long)(clock() * 1000 / CLOCKS_PER_SEC);
	//struct _timeb tbuf;
	//_ftime64_s(&tbuf); 
	//return ((long)(1000 * tbuf.time) + tbuf.millitm);
}

//=============================
//====================== to be in line with borland string 
char * stpcpy(char * d, char * o)
{strcpy_s(d,strlen(o)+2,o); return (d+strlen(d));}

char const * Blancs(int n,int pastrait) {
	const char *wt = "___________________ ";
	const char *wn = "                    ";
	if(pastrait)
		//return wn + 20 - n;
		//return &"                    "[20 - n];
		return &(wn[20 - n]);
	else
		return &(wt[20 - n]);
}

} //namespace skfr