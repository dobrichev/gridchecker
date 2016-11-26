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
#pragma once

#include "ratEnum.h"
#include "bitfields.h"

namespace skfr {

//! class storing, managing  and testing options of the command line

class OPSUDO
{
public: 
 int o1,   //<main mode 0 basic, 1 ed, 2 ep ,3  (n)x.y
	 delta, // authorized deviation in ED EP mode 2 if -D -P
	 ot,   //<option test on
	 os,   //<option split on
	 oq,   // otpion quick classification at nested level
	 oexclude, //<option code for the limit in processes activated
	          // 0 no exclusion 1 stop at multi chain ....0
     ocopybugs, // set to 1 for the time being, modifs in the code to stick to serate
	 maxed,      //< ed limit for ED filter <
	 mined,      //< ed limit for ed filter >
	 maxep,      //< ep limit for EP filter <
	 minep,      //< ep limit for EP filter >
	 maxer,      //< er limit for -r<     (rate only lowest puzzles
	 miner,      //< er limit for -n(?)> (high rating low ed)
	 edcycles,   //< number of cycles for differed ed
         ptime;      //< write time after er ep ed

	BF16 filters; // filters families activated  ed;ep;er;n

	OPSUDO();  // constructor, overall initial values for command line


};	 

} //namespace skfr