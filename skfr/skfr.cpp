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
// skfr.cpp : main file for sudoku fast rating
// processing exclusively a batch file 
// output = input valid + rating   default input="puz.txt"  
// default output1="puz_rat.txt"  default output1="puz_N_rat.txt"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#include "skfrtype.h"
#include "ratingengine.h"
#include "utilities.h"

namespace skfr {

//! a class for bitfield (limited to 32 bits) and set of functions and operators
class BF32
 {public:  UINT f;   // bitfield
  BF32() {f=0;}               // constructor
	/// is bit at ch position on
	inline int On(int ch)  {return ((f & (1<<ch))); }
	/// is bit at ch position off
	inline int Off(int ch)  {return (!(f & (1<<ch))); }
	/// set bit at ch position
	inline void Set(USHORT ch) {f|=(1<<ch);}
	/// clear bit at ch position
	inline void Clear(USHORT ch) {f&=~(1<<ch) ;}
};

//class OPSUDOCMD

//! class storing  options of the command line
/**
 * warning: all commands using the signs < > must be given within " "  eg "-d<6.2"
 *
 * <li>-d   stop if not diamond also --diamond</li>
 * <li>-p   stop if not pearl also --pearl</li>
 * <li>-D   same as diamond possible deviation 0.2</li>
 * <li>-P   same as pearl possible deviation 0.2</li>
 * <li>-d>  stop if ED lower than    -d&gtxx.y</li>
 * <li>-d<  stop if ED higher than   -d&ltxx.y</li>
 * <li>-p>  stop if EP lower than    -p&gtxx.y</li>
 * <li>-p<  stop if EP higher than   -p&ltxx.y</li>
 * <li>-r<  stop if ER higher than   -r&ltxx.y</li>  
 *
 * only one of the following limitations should be given, 
 * the last one in the command line will act
 * <li>--NoMul stop evaluation at "multi chains" (excluded) internal code 1</li> 
 * <li>--NoDyn stop evaluation at "dynamic" (excluded) internal code 2</li> 
 * <li>--NoDPlus stop evaluation at "dynamic plus" (excluded) internal code 3</li> 
 * <li>--NoLevel2 stop evaluation at "Nested level 2" (excluded) internal code 4</li> 
 * <li>--NoLevel3 stop evaluation at "Nested level 3" (excluded) internal code 5</li> 
 * <li>--NoLevel4 stop evaluation at "Nested level 4" (excluded) internal code 6</li> 
 *
 * <li>-Q   Quick classification for nested level </li>
 *
 * <li>-t   allow printing of the solution (test mode)</li>
 * <li>-n()xx.y  if after n cycles highest rating still lower than xx.y
 *      forces split mode</li>
 * <li>-e  elapsed time per puzzle (total for benchmark allways given)</li></ul>
 * The following options are processed outside that class<ul>
 * <li>-i input file also --input=</li>
 * <li>-s split treatment if dm or dl (or both) defined and forced if -n() defined</li>
 * </ul>
 * For can have all "l" and "m" 
 */
class OPSUDOCMD
{
public: 
 int o1,   //<main mode 0 basic, 1 ed, 2 ep ,3  (n)x.y
	 delta, // authorized deviation in ED EP mode 2 if -D -P
	 ot,   //<option test on
	 os,   //<option split on
	 oq,   // otpion quick classification at nested level
	 oexclude, //<option code for the limit in processes activated
	          // 0 no exclusion 1 stop at multi chain ....0
	 maxed,      //< ed limit for ED filter <
	 mined,      //< ed limit for ed filter >
	 maxep,      //< ep limit for EP filter <
	 minep,      //< ep limit for EP filter >
	 maxer,      //< er limit for -r<     (rate only lowest puzzles
	 miner,      //< er limit for -n(?)> (high rating low ed)
	 edcycles,   //< number of cycles for differed ed
     ptime;     //< write time after er ep ed
    char logFileName[200];
	 /**
	bit 0 =1 if "not rated" or "spit not ok"
	bit 1 =1 if end in "error" or "not finished"
	bit 2=1 if not "diamond" "pearl" with that option
	bit 3=1 if split ok (bit 1 = 0)
	*/
	BF32 filters; // filters families activated  ed;ep;er;n
	OPSUDOCMD(); // constructor to put all values to 0;
	//void Init();  // overall initial values for command line

	int  pctl();  // contrôle print on console for test purpose
	// also check command line compatibility
	
};

OPSUDOCMD::OPSUDOCMD()  // constructor, overall initial values for command line
    {   delta=0;		
		o1=ot=os=oq=oexclude=filters.f=0;
		edcycles=1;
		mined=minep=miner=0;maxed=maxep=maxer=200;
		ptime=0;
		logFileName[0]=0;
    }
int  OPSUDOCMD::pctl()  
	// contrôle print on console for test purpose
	// evaluate filters bitfield if min or max required
	// also check command line compatibility (between min and max)
	{
		// split option is exclusively for special "filters"
		if(o1<3) os=0; 	
		cout << "summary of parsing"<<endl;
		if(o1==1) cout << "mode ed delta="<<delta<<endl;
		if(o1==2) cout << "mode ep delta="<<delta<<endl;
		if(o1==3) cout << "special filters on"<<endl;
		if(ot) cout << "test option"<<endl;
		if(maxed<200){cout << "ed<"<<maxed<<endl;filters.f|=1;}
		if(mined){cout << "ed>"<<mined<<endl;filters.f|=1;}
		if(maxep<200){cout << "ep<"<<maxep<<endl;filters.f|=2;}
		if(minep){cout << "ep>"<<minep<<endl;filters.f|=2;} 
		if(maxer<200){cout << "er<"<<maxer<<endl;filters.f|=4;}
		if(miner)
		{
			cout << "differred ed cycl="<<edcycles<<" rating"<<miner<<endl;
			filters.f|=8; 
		}
		if(ptime) cout << "print time"<<endl;
		if(!(filters.f&7)) os=0; // no split on er condition, meaningless
		if(os) cout << "split mode active filters="<<filters.f<<endl;
		if(oexclude) cout << "process limitation code="<<oexclude<<endl;
		if(maxed<=mined || maxep<=minep || maxer<=miner)
		{
			cout << "cancelled due to contradiction in filters"<<endl;
			return 1;
		}
		return 0;
	}

//!Input file containing a list of puzzle to rate.
/**
 * By default default puz.txt in the current directory.
 */
class FINPUT: private ifstream
{
	USHORT 
		//! Indicate if input file is open 1=open, 0=not open.
		open, 
		//! Indicate if end of file has been found 1=EOF, 0=not EOF.
		endf; 
public:
	//!Input file name (without file type).
	const static char * namex; 
	FINPUT();
	//! Set the name of the input file.
	/**
	 * A suffix .txt will be added to this name.
	 * This method should be used before opening the file.
	 */
	void SetName(char * nn);

	//! Open the input file
	/**
	 * Prior you shoud set the input file name if you don't use the default one.
	 * \return 0 if opened, 1 if an error occured.
	 * \sa FINPUT::SetName
	 */
	int OpenFI();
	
	//! Close the input file if it not already closed.
	void CloseFI();
	
	//! Get the next puzzle from the input file and normalize it.
	/*
	 * A normalized puzzle is a 81 char string with '1'-'9' for the givens and '.' for the
	 * empty cells. <br>
	 * This method will stop the reading of the input file if it encounters a too long line 
	 * (more than 250). <br>
	 * It skips lines that are too short (81 char minimum). 
	 * It skip also puzzles that have less than 17 clues.<br>
	 * It normalizes the puzzle, and for that it considers all char 
	 * that are not in the range '1'-'9' as empty cells and converts them to '.'<br>
	 * <b>WARNING :</b> in GG class empty cell are coded as '0' !
	 * \param zp pointer to a buffer 82 char long where the puzzle will be copied.
	 * \return 0 if no read puzzle, 1 if a puzzle has been copied to the buffer.
	 */
	int GetPuzzle(char * zp);  // get next and write on output the puzzle
    
	void GetLogName(char * zn)
	   {strcpy_s(zn,198,namex); 
		int ll=(int)strlen(zn);
        strcpy_s(&zn[ll],30,"_log.txt");
	   }
};



//! Output files for rated and non rated puzzles. 
/**
 * There are 2 output files, one for puzzles that have been rated
 * one for puzzles that cannot be rated (rating too high for the current Version)<br>
 * The names of these output files are derived from the name of input file.<br><br>
 * for rated puzzles inputFileName + "_rat.txt" <br>
 * for non rated puzzles inputFileName + "_N_rat.txt"
 */
class FOUTPUT : public ofstream  // public pour les tests
{
	USHORT 
		//! indicates if file is open 0=not open, 1=open
		open, 
		
		//! indicates if file doesn't need to be closed 1=no need, 0
		endf; //TODO problem of status
public:
	//! Pointer to the input puzzle 
	/** It should be set up before calling PrintErEpEd */
	static char* zpuzx;

	FOUTPUT(){open=0;endf=1;}

	inline void Setzpuz(char * x) {zpuzx=x;}
	//! Open method to call for rated puzzles
	/** \return 0 if opened, 1 in case of error. */
	int OpenFO1();
	
	//! Open method to call for non rated puzzles
	/** \return 0 if opened, 1 in case of error. */
	int OpenFO2(); 

	//! Close this output file
	void CloseFO(){if(endf) return; ofstream::close();endf=1;}
 
	//! Output the current puzzle and its rating
	/**
	 * Current puzzle (string form) should be pointed by <code> zpuz</code> class member
	 * \param er ER rating as an integer (108 will be ouput as 10.8)
	 * \param ep EP rating (highest move until first placement)
	 * \param ed ED rating (first move)
	 */
	void PrintErEpEd(int er,int ep,int ed);
private :
	int OpenFO(char * nam);
    
};

FINPUT::FINPUT(){open=endf=0;}
void FINPUT::SetName(char * nn){namex=nn;}


const char * FINPUT::namex="puz";

int FINPUT::OpenFI()
{
	if(open) return 0; 
	open=1;
	char zn[200]; 
	strcpy_s(zn,198,namex); 
	int ll=(int)strlen(zn);
	strcpy_s(&zn[ll],30,".txt"); // add file type
	ifstream::open(zn); 
	if(is_open()) return 0 ;
	//Console::WriteLine("problem in open input");
	cerr<<"problem in open input\n";
	endf=1;
	return 1; 
}

void FINPUT::CloseFI()
{
	if(endf) return; 
	ifstream::close();
	endf=1;
}

int FINPUT::GetPuzzle(char * zp)
{
	if(endf) return 0;
	char ze[400]; // big buffer to avoid problems linked to getline
	while(!eof() )
	{
		getline(ze,300); 
		ze[301]=0;  
		int i=(int)strlen(ze); 
		if(i>250)
		{
			endf=1;return 0;
		}
		if(i<81)continue; 
		zp[81]=0; // file truncated to 81
		// standard file  clues or "." minimum 17 clues
		int n=0;
		for(int i=0;i<81;i++) 
		{
			char c=ze[i]; 
			if(c<'0' || c>'9') zp[i]='.'; 
			else {n++;zp[i]=ze[i];}
		}
		if(n<17) continue; 
		return 1;
	}// end while
	endf=1; 
	return 0;
}

char* FOUTPUT::zpuzx;
 
int FOUTPUT::OpenFO1()  
	{
		char zn[200]; 
		strcpy_s(zn,198,FINPUT::namex); 
		int ll=(int)strlen(zn);
        strcpy_s(&zn[ll],30,"_rat.txt");
	    return OpenFO(zn);
	}

int FOUTPUT::OpenFO2() 
{
	char zn[200];
	strcpy_s(zn,198,FINPUT::namex);
	int ll=(int)strlen(zn);
    strcpy_s(&zn[ll],30,"_N_rat.txt");
	return OpenFO(zn);
}

int FOUTPUT::OpenFO(char * nam)
{
	if(open) return 0; 
	open=1;
	endf=0;
	ofstream::open(nam); 
	if(is_open()) 
		return 0;
	//Console::WriteLine("problem in open output");
	cerr<<"problem in open output\n";
	open =0;
	endf=1;
	return 1;
}

void FOUTPUT::PrintErEpEd(int er,int ep,int ed)
    {(*this)<<zpuzx<<" ";
		int era=er/10,erb=er%10,epa=ep/10,epb=ep%10,eda=ed/10,edb=ed%10;
		(*this)	 <<"ED="<<era<<"."<<erb<<"/"<<epa<<"."<<epb<<"/"<<eda<<"."<<edb; 
	} 

// global variables
FINPUT finput;
FOUTPUT foutput,se_refus;
OPSUDOCMD Opc;

//==== specific data
char finput_name[128]; // string file names if given
const char * version="V2.0.1 dated October 16th 2012";

// =====================functions written below

//int Convert_String_to_string(char * str,System::String  ^Str);
int Search_ccd(char * ww,int n);  // search commands 2 or 3  bytes
int Search_long(char * ww);   // search for extended commands eg: input=FILE
int GetRating(char * z);     // look for xx.y
void Filter_quasi_ed(char * ww); // finish command -n(?)>
int CallOpenLog(char *);
//==========================================
//! Start the batch of puzzle rating
/**
 * Batch_Start analyzes the command line.<br>
 * Command line uses Unicode char that are converted to null terminated string.
 * It looks for options and control coherence (min & max).<br>
 * Open input outputs files and 
 * makes all batch initialisations.
 * \param args arguments from the command line
 * \return 0 if an error occured (incoherence between min max options 
 * or error when opening files), 1 if all is ok
 */
//int Batch_Start(array<System::String ^> ^args)
int Batch_Start(int argc, char *argv[])
{
	//char ww[512];
	char *ww;
	cout <<"Version: "<< version<<endl;
	//int narg=args->GetLength(0);
	int narg=argc;
	for(int i=0;i<narg;i++)
	{
		//if(!Convert_String_to_string(ww,args[i])) continue;// ignore if error
		ww = argv[i];
		int val1;
		int ir=Search_ccd(ww,3);	// look for 3 char command
		if(ir>=0)
		{
			if(Opc.o1 && Opc.o1<3) continue; // ignored if -d -p
			Opc.o1=3; // special filter on
			if(ir<5) 
			{
				val1=GetRating(&ww[3]); // get rating threshold
				if (val1<0) continue;
			}
			switch (ir)
			{
			case 0:Opc.maxed=val1; break;  //-d<
			case 1:Opc.mined=val1; break;  //-d>
			case 2:Opc.maxep=val1; break;  //-p<
			case 3:Opc.minep=val1; break;  //-p>
			case 4:Opc.maxer=val1; break;  //-r<
			case 5:Filter_quasi_ed(ww);   break;  // -n(?)>
			}
			continue;// don't try command 2
		}// end command 3 bytes
		ir=Search_ccd(ww,2);
		if(ir>=0)
		{
			if(ir==6)
				{ir=Search_long(&ww[2]);  //"--" 
				 if(ir>5) // this is a command to limit the process
				 {Opc.oexclude= ir-5; continue;}
			    }
			if(ir>=0)
				switch (ir)
				{
				case 0:Opc.o1=1; break;  //-d
				case 1:if(Opc.o1-1)Opc.o1=2; break;  //-p
				case 2: 
					if(ww[1]=='-')strcpy_s(finput_name,128,&ww[8]);  
					else  strcpy_s(finput_name,128,&ww[2]); 
					finput.SetName(finput_name);
					cout <<"input asked   "<<finput_name<<endl;
					break;  //-i
				case 3: Opc.os=1; break;  //-s
				case 4: Opc.ptime=1; break;  //-e
				case 5: Opc.ot=1; break;  //-t
				case 7: Opc.o1=1;Opc.delta=2; break;  //-D
				case 8: if(Opc.o1-1)Opc.o1=2;Opc.delta=2; break;  //-P
				case 9: Opc.oq=1; break; //-Q 
				}
		}// end command 2 bytes
	}// end loop on options
	// output options parsing on console and verify coherence between min and max
	if(Opc.pctl()) return 0;	//return if incoherence in options
	// open input and outputs files (rated and non rated files)
	if(finput.OpenFI()) return 0; // 1 if error open
	if(foutput.OpenFO1()) return 0; // 1 if error open
	if(se_refus.OpenFO2()) return 0; // 1 if error open
	// if debugging option is on, call open ficlog with the appropriate name
	if(Opc.ot) {finput.GetLogName(Opc.logFileName); 
	           if(CallOpenLog(Opc.logFileName)) return 0;
			   cout << "open log fait"<<endl;}
	// set option to the rating engine
   setParamC (Opc.o1, Opc.delta, Opc.os,Opc.oq,Opc.ot , Opc.oexclude, Opc.edcycles);
   setMinMaxC(Opc.mined,Opc.maxed, Opc.minep, Opc.maxep, Opc.miner, Opc.maxer,Opc.filters.f);
 //  if(PrintOptionsOpsudo()) return 0;
   return 1;
} 

//========================================
//! try to locate a command length n (2 or 3)
/**
 * Can recognize 2 char command only if no additional char
 * ie "-i" is good "-i3" is not recognized
 * \param ww an option
 * \param n 2 or 3 length of searched option
 * \return index of command, -1 if not found
 */
int Search_ccd(char * ww,int n)
{
	// List of 3 char command, 6 commands
	const char * ccd3[]={"-d<" , "-d>" , "-p<" , "-p>" , "-r<" , "-n("  };
	// List of 2 char command, 9 commands
	const char * ccd2[]={"-d" ,  "-p" , "-i" , "-s" ,  "-e" , "-t" ,"--", 
	               "-D" , "-P", "-Q"  };
	char wt[4]; 
	strncpy_s(wt,4,ww,n);
	wt[3]=0;
	for(int i=0;i<((n==2)?10:6);i++)
		if(!strcmp(wt,(n==3)?ccd3[i]:ccd2[i])) 
			return i;
	return -1;
}

//=============================================
//! Try to identify long form of options beginning with "--"
/**
 * \param ww
 * \return index of command, -1 if none recognized
 */
int Search_long(char * ww)
{
	const char * cc[]={"diamond" ,  "pearl" , "input=" , "split" ,
               "elapsed" , "test",  "NoMul",
			   "NoDyn", "NoDPlus","NoLevel2","NoLevel3","NoLevel4"};
	char wt[20]; 
	for(int i=0;i<12;i++) // loop on the 11 possible commands
	{
		// copy options and truncate at possible option length
		int j=(int)strlen(cc[i]); 
		for(int k=0;k<j;k++)wt[k]=ww[k];
		wt[j]=0;
		if(!strcmp(wt,cc[i])) 
			return i;
	}
	return -1;
}

//=================================================
//! Get a rating from an options string
/**
 * Command like "-d<xx.x" include a rating "xx.x" that 
 * is converted to an int (x10)
 * rating are not accepted above 11.9 and cannot begin with 0
 * so there are between 1.0 and 11.9
 * \param z part of the option that represent a rating "xx.x" or "x.x"
 * \return rating x 10, -1 if error
 */
int GetRating(char * z)
{	int val=0;
	if(z[0]<'1' || z[0]>'9') return -1;
	val=z[0]-'0';	// get first digit
	if(z[1]-'.') 
	{	// 2 digits
		if(val-1) return -1;	// error if first digit != 1
        if(z[1]<'0' || z[1]>'1') return -1;  //10.x or 11.x
		val=10*val+z[1]-'0';
		z++; //skip one position in inflow
	} 
	if(z[1]-'.')return -1;// now must be the '.'
	if(z[2]<'0' || z[2]>'9') return -1; 
	val=10*val+z[2]-'0';  // now val is in the range 010 to 120 for 1.0 to 11.9
	return val;}

//========================================
//! Converting a String unicode to a classical string
/** 
 * \param str a buffer large enough to get the converted string
 * \param Str unicode string
 * \return 0 if an error occured, 1 OK
 */
//int Convert_String_to_string(char * str,System::String  ^Str)
//{
//	pin_ptr<const wchar_t> wch = PtrToStringChars(Str);
//	size_t convertedChars = 0;
//	size_t sizeInBytes = ((Str->Length + 1) * 2);
//	errno_t err = 0;
//	err = wcstombs_s(&convertedChars, str, 512,  wch, sizeInBytes);
//	if (err != 0)return 0; 
//	return 1;
//}
//===========================================
//! Finish command  command -n(y)xx.x
/** 
 * y must be in the range [2 9]<br>
 * xx.x must be in the range [4.0 11.0]
 * update <code>Op.edcycles, Op.miner, Op.os</code> 
 */
void Filter_quasi_ed(char * z) // finish command  command -n(?)
{
	if(z[3]<'2' || z[3]>'9') return ;
	int w=GetRating(&z[5]);
	if(w<40 || w>110) return;
	Opc.edcycles=z[3]-'0';
	Opc.miner=w; 
	Opc.os=1;
}

// loop to process the batch and puzzles verifications

// builing an appropriate message depending on the elapsed time te-ts
void PrintTime(long ts,long te,int cons)
{UINT dt=te-ts,dtmil=dt%1000,dts=dt/1000,dth=dts/3600;   dth=dth%1000;
if(cons)
   { cout << endl<<"total elapsed time "; 
     UINT dtm=dts/60; dts=dts%60 ,   dth=dtm/60, dtm=dtm%60;
     if(dth) cout <<dth<<"h "; 
	 if(dth || dtm) cout <<dtm<<"m "; 
     cout	<<dts <<"s ";
	 if(dtmil<10) cout << "00"; else  if(dtmil<100) cout << '0';
	 cout <<dtmil<<"ms " <<endl;   return;
    }
  foutput	 <<";"<<dts <<".";
  if(dtmil<10) foutput << "00"; else  if(dtmil<100) foutput << '0';
  foutput <<dtmil<<"s "<<endl;  
}

//! Evaluation loop of puzzle and print the result for each puzzle
#ifdef _OPENMP
void batchGoParallel() { //currently unused
	const int maxChunkSize = 3 * 64 * 10;
	const int chunkSize = maxChunkSize; //one might want to control the actual chunk size via parameter
	int nPuzzles = 0;
	int tER[maxChunkSize], tEP[maxChunkSize], tED[maxChunkSize], tAIG[maxChunkSize], tIR[maxChunkSize];
	char tZE[maxChunkSize][82];
	while(1) {
		//read a chunk
		while(nPuzzles < chunkSize && finput.GetPuzzle(tZE[nPuzzles])) {
			nPuzzles++;
		}
		//rate the chunk
		ratePuzzlesC(nPuzzles, tZE[0], tER, tEP, tED, tAIG, tIR);
		//output chunk ratings
		for(int i = 0; i < nPuzzles; i++) {
			foutput.Setzpuz(tZE[i]);		// prepare to output the puzzle string
			if(Opc.os) {   // split option
				if(tIR[i] < 4)
					se_refus << tZE[i] << endl;
				else
					foutput << tZE[i] << endl;
			}
			else {
				if(tAIG[i])
					tER[i] = 0;
				bool refus = (tER[i] < 10) || (tER[i] > 120);
				if(refus) {
					se_refus.PrintErEpEd(tER[i], tEP[i], tED[i]);
					se_refus << endl;
				}
				else {
					foutput.PrintErEpEd(tER[i], tEP[i], tED[i]);
					foutput<<endl;
				}
			}
		}
		if(nPuzzles < chunkSize)
			break;	//loop exit condition
		nPuzzles = 0; //empty the chunk
	} //chunk loop
}
#endif

void batchGoSequential() {
	char ze[82];
	foutput.Setzpuz(ze);		// prepare to output the puzzle string
	while(finput.GetPuzzle(ze)) 
	{
		long tpuz_start=GetTimeMillis(); // for puzzle elapsed time
		int er, ep, ed, aig;

		int ir=ratePuzzleC(ze, & er, & ep, & ed, & aig); 
		if(Opc.os)    // split option
        {
			if(ir<4) se_refus<< ze 
		//		<< " " << Op.ermax << " " << Op.epmax<< " " << Op.edmax
				<< endl; 
			else  foutput << ze 
		//		<< " " << Op.ermax << " " << Op.epmax<< " " << Op.edmax 
				<<endl; 
		}
		else
		{  
			if(aig)er=0;
			bool refus = (er<10) || (er>120);
			if(refus)se_refus.PrintErEpEd(er,ep,ed);
			else  foutput.PrintErEpEd(er,ep,ed);
			if(Opc.ptime && !refus ) 
			{
				long tpuz_end=GetTimeMillis();
				PrintTime(tpuz_start, tpuz_end, 0);
			}
			if(refus) se_refus << endl;
			else  foutput<<endl;
		}
	}
}
void Batch_Go() {
	long tstart=GetTimeMillis();// for the overall elapsed time

#ifdef _OPENMP
	//TODO: check for parameters incompatible to parallel mode and exit if any
	batchGoParallel();
#else
	batchGoSequential();
#endif

	// print global elapsed time
	long tend=GetTimeMillis();
	PrintTime(tstart,tend,1);
}


} //namespace skfr

#if 0
int main(int argc, char *argv[]) {
	if(skfr::Batch_Start(argc, argv)) 
		skfr::Batch_Go();
	return 0;
}
#endif
