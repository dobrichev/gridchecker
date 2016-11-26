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

//! Output files for debuging purpose
/**
 * That file, specific to the rating process is a debugging file.
 * it does not exist in standrard mode.
 * all outputs in that file
 * The names of these log files are derived from the name of input file.<br><br>
 * for rated puzzles inputFileName + ".log" <br>

 */

/**
 * If <code>pron</code> print to log file, else do nothing (except setting 
 * <code>puz.stop_rating</code> for FSR::Estop)<br>
 * There are method to output <br><ul>
 * <li>char, null terminated string and int with or without newline.</li>
 * <li>space, one or two newline, "[]",dot, equal, minus.</li>
 * <li>an error message to signal overflow of a table.</li>
 * <li>an error message and set puz.stop_rating to 1. This is used to stop the program.</li>

 * </ul><br>
 * There is only one instance of this class :  EE that is used by technics classes
 * to print information for debugging.
 */
#include <iostream>
#include <fstream>
#include "skfrtype.h"
using namespace std;

namespace skfr {

#ifndef _OPENMP
class FLOG : private ofstream  //no direct access to ofstream  
{
	USHORT 
		//! indicates if file is open 0=not open, 1=open
		pron, 
		
		//! indicates if file doesn't need to be closed 1=no need, 0
		endf; //TODO problem of status
public:

	FLOG(){pron=0;endf=1;}

	//! Open method to call for log file
	/** \return 0 if opened, 1 in case of error. */
	int OpenFL(char * nam) {
		if(pron)
			return 0; 
		pron = 1;
		endf = 0;
		ofstream::open(nam); 
		if(is_open()) {
			(*this) << "opend log fait"<<endl;
		return 0;
		}
		//Console::WriteLine("problem in open log");
		cerr << "problem in open log" << endl;
		pron = 0;
		endf = 1;
		return 1;
	}
	//! Close this log file
	void CloseFL() {
		if(endf)
			return;
		ofstream::close();
		endf = 1;
	}

	// these are debugging commands 
	//! If <code>pron</code> print the char <code>c</code>
	inline void E(char c)	{if(pron)(*this)<<c; }
	//! If <code>pron</code> print the null terminated string <code>c</code>
	inline void E(const char* c)	{if(pron)(*this)<<c; }
	//! If <code>pron</code> print the int <code>x</code>
	inline void E(int x)	{if(pron)(*this)<<x;}
	//! If <code>pron</code> print a newline
	inline void Enl()	{if(pron)(*this)<<endl;}
	//! If <code>pron</code> print a space
	inline void Esp()	{if(pron)(*this)<<' '; }
	//! If <code>pron</code> print "[]"
	inline void Echem(){if(pron)(*this)<<"[]"; }
	//! If <code>pron</code> print a dot
	inline void Etir()	{if(pron)(*this)<<"."; }
	//! If <code>pron</code> print " = "
	inline void Esl()	{if(pron)(*this)<<" = "; }
	//! If <code>pron</code> print " - "
	inline void Ewl()	{if(pron)(*this)<<" - "; }
	//! If <code>pron</code> print the char <code>c</code> + newline
	inline void Enl(char c)	{if(pron)(*this)<<c<<endl; }
	//! If <code>pron</code> print the null terminated string <code>c</code> + newline
	inline void Enl(const char* c)	{if(pron)(*this)<<c<<endl; }
	//! If <code>pron</code> print the int <code>x</code> + newline
	inline void Enl(int x)	{if(pron)(*this)<<x<<endl;}
	//! If <code>pron</code> print two newlines
	inline void Enl2(){if(pron)(*this)<<endl<<endl;}
	//! If <code>pron</code> print an overflow table message
//	void Elimite(char * lib){Enl2();E("table:"); E(lib); Estop("limite atteinte "); }
	//! If <code>pron</code> print an error message and in all cases set <code>puz.stop_rating</code>
//	void Estop(char * lib) {
//		Enl2();E(lib);Enl2(); //puz.stop_rating=1;
//	}
};
#else
class FLOG  //dummy interface implementation
{
	USHORT 
		//! indicates if file is open 0=not open, 1=open
		pron, 
		
		//! indicates if file doesn't need to be closed 1=no need, 0
		endf; //TODO problem of status
public:
	FLOG() {}
	int OpenFL(char * nam) {return 0;}
	void CloseFL() {}

	inline void E(char c) {}
	inline void E(const char* c) {}
	inline void E(int x) {}
	inline void Enl() {}
	inline void Esp() {}
	inline void Echem() {}
	inline void Etir() {}
	inline void Esl() {}
	inline void Ewl() {}
	inline void Enl(char c)	{}
	inline void Enl(const char* c) {}
	inline void Enl(int x) {}
	inline void Enl2() {}
};
#endif

} //namespace skfr