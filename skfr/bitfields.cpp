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

#include "bitfields.h"
#include <memory.h>

namespace skfr {

//-----
void BFTAG::SetAll_0() {
	for(int i = 0; i < 5; i++)
		ff[i].clear();
}
void BFTAG::SetAll_1() {
	for(int i = 0; i < 5; i++)
		ff[i] = maskffff;
}
bool BFTAG::IsNotEmpty() const {
	for(int i = 0; i < 5; i++)
		if(!ff[i].isZero())
			return true;
	return false;
}
bool BFTAG::IsEmpty() const {
	for(int i = 0; i < 5; i++)
		if(!ff[i].isZero())
			return false;
	return true;
}

inline unsigned int popCount32(unsigned int v) { // count bits set in this (32-bit value)
	v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24; // count
}
//  GP 2011 10 9 should be replaced by a process using BF_CONVERT
//
int BFTAG::Count() const {
	int c = 0;
	for(int i = 0; i < 5; i++)
		c += popcount_128(ff[i].bitmap128.m128i_m128i);
	return c;
}

// GP 2011 10 9
// should be replaced by a quick process using true32 false32
//fx= (f&true32)<<1 + (f&false32 >>1)
//----
BFTAG BFTAG::Inverse() const {
	BFTAG w;
	for(int i = 0; i < 5; i++)
		w.ff[i].alterOddEven(ff[i]);
	return w;
}

BFTAG BFTAG::TrueState() const {
	BFTAG w = (*this);
	for(int i = 0; i < 5; i++)
		w.ff[i] &= true128;
	return w;
}

BFTAG BFTAG::FalseState() const {
	BFTAG w = (*this);
	for(int i = 0; i < 5; i++)
		w.ff[i] &= false128;
	return w;
}

//----
void BFTAG::String(USHORT * r, USHORT &n) const {
	n = 0;
	//for(int i = 0; i < 5; i++) {
	//	if(!ff[i].isZero()) {
	//		for(unsigned int k = 0; k < 128; k++) {
	//			if(ff[i].isBitSet(k))
	//				r[n++] = (USHORT)(i * 128 + k);
	//		}
	//	}
	//}
	static const unsigned char toPos[] = { //TODO: move to the global lookups
		0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8};
	for(int i = 0; i < 5; i++) {
		int m = ff[i].nonzeroOctets(); // 16-bit mask of octets having non-zero bits
		int add8 = 0; //0 for lower 8 bits, 8 for higher 8 bits
		while(m) { //exit if no more octets with bits set
			if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
				m >>= 8;
				add8 = 8;
			}
			int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
			int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
			int octetValue = ff[i].bitmap128.m128i_u8[octetIndex];
			do {
				int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
				int bitIndex = (i * 128) + (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
				r[n++] = (USHORT)bitIndex; //store
				octetValue ^= octetLSB; //clear the processed bit from the temporay copy
			} while(octetValue); //loop until all bits within this octed are processed
			m ^= octetIndexLSB; //clear the octet processed
		}
	}
}
void BFTAG::operator &= (const BFTAG &z2) {
	//for(int i = 0; i < BFTAG_size; i++)
	//	f[i] &= z2.f[i];
	for(int i = 0; i < 5; i++)
		ff[i] &= z2.ff[i];
}
void BFTAG::operator |= (const BFTAG &z2) {
	for(int i = 0; i < 5; i++)
		ff[i] |= z2.ff[i];
}
bool BFTAG::operator == (const BFTAG &z2) const {
	for(int i = 0; i < 5; i++)
		if(!(ff[i] == z2.ff[i]))
			return false;
	return true;
}
void BFTAG::operator -= (const BFTAG &z2) {
	for(int i = 0; i < 5; i++)
		ff[i] -= z2.ff[i];
}

bool BFTAG::substract(const BFTAG &z2) {
	bm128 accum;
	ff[0] -= z2.ff[0]; accum = ff[0];
	ff[1] -= z2.ff[1]; accum |= ff[1];
	ff[2] -= z2.ff[2]; accum |= ff[2];
	ff[3] -= z2.ff[3]; accum |= ff[3];
	ff[4] -= z2.ff[4]; accum |= ff[4];
	return !accum.isZero();
}


/* search in the area x cycle to xy chain
   1) a cycle can give no elimination.
   2) cycle search can be very long especially in Y mode

   To search cycles, 
   a) look first for the shortest elimination
   b) compute the max length for a shorter cycle 
      (max of the same slice in the "difficulty" scale)
   c) expand all and sort out tags eliminated and tags in loop
   d) Locate tags seing 2 tags in loop and try to expand that loop  

   Note, this is not a key process. Eliminations have been found
   It would be easy to speed it up using the even odd property
   Doing more is tougher but possible

*/
/* Look (no derived weak link) for the shortest way from start  to end
   the calling sequence provides an empty or partially filled situation
*/
int BFTAG::SearchChain(const BFTAG *to, USHORT start, USHORT end) {
	int npas = 0; 
	// to be safe in nested mode, dimension increased to 200
	USHORT tta[200], ttb[200], *told = tta, *tnew = ttb, itold, itnew;
	(*this).String(tta, itold);
	while(npas++ < 30) {
		tnew = (told == tta) ? ttb : tta; // new the second table
		itnew = 0;
		// EE.E("cycle");zpln.PrintListe(told,itold,1); 
		//int aig = 1; // to detect an empty pass
		for(int it = 0; it < itold; it++) {
			BFTAG x = to[told[it]];
			//x -= (*this);
			//if(x.IsNotEmpty()) {
			if(x.substract(*this)) {
				(*this) |= x; // flag it in the BFTAG and load in new
				// here tx dimension increased. In nested mode, could require it
				USHORT itx;
				x.String(&tnew[itnew], itx);
				itnew += itx;
			}
		}

		if(On(end)) {
			//EE.E("cycle fin npas= ");EE.E(npas);zpln.PrintListe(tnew,itnew,1); 
			return npas; // eliminations found
		}
		if(!itnew)
			return 0;     // empty pass should never be
		itold=itnew;
		told=tnew; // new -> old
	}// end while
	// should send a warning message for debugging purpose
	return 0;
}
/* same process, but start == end  (cycle)
   and all tags of the path must belong to the loop
*/
int BFTAG::SearchCycle(const BFTAG *to, USHORT start, const BFTAG &loop) {
	int npas = 0; 
	USHORT tta[100], ttb[100], *told = tta, *tnew = ttb, itold, itnew;
	(*this).String(tta, itold);
	while(npas++ < 30) {
		tnew = (told == tta) ? ttb : tta; // new the second table
		itnew = 0;
		// EE.E("cycle");zpln.PrintListe(told,itold,1); 
		for(int it = 0; it < itold; it++) {
			BFTAG x = to[told[it]];
			x -= (*this);
			x &= loop;
			if(x.IsNotEmpty()) {
				(*this) |= x; // flag it in the BFTAG and load in new
				USHORT itx;
				x.String(&tnew[itnew], itx);
				itnew += itx;
			}
		}
		if(On(start))
			return npas; // eliminations found
		if(!itnew)
			return 0;     // empty pass should never be
		itold = itnew;
		told = tnew; // new -> old
	}// end while
	// should send a warning message for debugging purpose
	return 0;
}

/* same process, but partial
   we only go to the tag 'relay' that should come first but may be not
*/
int BFTAG::SearchCycleChain(const BFTAG *to, USHORT i, USHORT relay, const BFTAG &loop) {
	int npas = 0; 
	USHORT tta[100], ttb[100], *told = tta, *tnew = ttb, itold, itnew;
	(*this).String(tta, itold); 
	(*this).Set(i); // lock crossing back i
	while(npas++ < 20) {
		tnew = (told == tta) ? ttb : tta; // new the second table
		itnew = 0;
		for(int it = 0; it < itold; it++) {
			BFTAG x = to[told[it]];
			x -= (*this);
			x &= loop;
			if(x.IsNotEmpty()) {
				(*this) |= x; // flag it in the BFTAG and load in new
				USHORT itx;
				x.String(&tnew[itnew], itx);
				itnew += itx;
			}    
		}
		if(On(relay))
			return npas; // target found
		if(!itnew)
			return 0;     // empty pass should never be
		itold = itnew;
		told = tnew; // new -> old
	}// end while
	// may be a warning here for debugging purpose
	return 0;
}
/* trackback gives back a path out of the search
   to make easier the split in modules, the path is given as a table of tags

   The forward path as such can not be used. It must be reworked step by step.
   The specific rules
     start with a weak link in loop mode
	 go thru in Y loop moe
   have to be applied again.

   This will be done giving a "relay" candidate (the first in X or XY loop mode)


   applied to  the result of the forward search (most often in test mode)
   end is the end tag.

   The process works for sure in elimination mode , but also in cycle mode 
   the candidate of the end step can not be in the first step ... 

   The process can be slightly improved considering the following

   a cycle here ends always with a strong link and has an odd number of steps
   a chain has always and even number of steps and ends with a weak link
   all that to be checked carefully

   to avoid circular path with strong links, each tag used is cleared

   not yet enough to be revised
   need really to build forward step by step before going backward
 */
int BFTAG::TrackBack(const BFTAG *to, USHORT start, USHORT end, USHORT * tt, USHORT & itt, USHORT relay) const {
	// first we have to build forward step by step 
	if(itt > 40) {
		/*  debugging infromation to be relocated in the calling sequence
		EE.E("trackback to many steps=");
		EE.E(itt);
		EE.E(" start=");
		zpln.ImageTag(start); 
		EE.E(" end=");
		zpln.ImageTag(end); 
		EE.E(" relay=");
		zpln.ImageTag(relay);
		EE.Enl();*/
		return 1;
	}
	BFTAG steps[50], allsteps; // surely never so many
	USHORT tx[50][200], itx[50], npas = 0;
	steps[0].SetAll_0();
	steps[0].Set(start); 
	allsteps = steps[0]; // one way to force weak link at the start in loop mode
	tx[0][0] = start;
	itx[0] = 1; // initial is start to go forward
	while(npas < (itt - 1)) { // must end in step itt
		BFTAG * step = &steps[npas+1];
		step->SetAll_0();
		USHORT *ta = tx[npas], *tb = tx[npas + 1],
			ita = itx[npas], itb = 0;
		for(int it = 0; it < ita; it++) {
			//BFTAG x = (to[ta[it]] - allsteps) & (*this);	  // still free and in the overall path
			BFTAG x = to[ta[it]];
			x -= allsteps;
			x &= (*this);	  // still free and in the overall path
			if(x.IsNotEmpty()) {
				(*step) |= x; // flag it in the BFTAG and load in new
				allsteps |= x; // and in the total 
				USHORT ity;
				x.String(&tb[itb], ity);
				itb += ity;
			}
		}
		if(step->On(end))
			break;
		if(step->On(relay)) { // if "relay" is reached, force tb to relay only
			for(int k = 0; k < itb; k++) { // clear allsteps
				USHORT xx = tb[k];
				if(xx-relay) allsteps.Clear(xx);
			}	
			if(start == end)
				allsteps.Clear(end);
			itb = 1;
			tb[0] = relay;  
		}

		npas++;
		itx[npas] = itb; 
		//   EE.E("passe npas=");EE.E(npas); EE.Esp();
		//   zpln.PrintListe(tb,itb,1);
	}// end while
	if((npas + 2) - itt) {
		/* debugging code to relocate
		if(1 && Op.ot) {
			EE.E("invalid trackback end phase 1 npas==");
			EE.E(npas);
			EE.E(" itt=");
			EE.Enl(itt);
			EE.E(" start=");
			zpln.ImageTag(start); 
			EE.E(" end=");
			zpln.ImageTag(end); 
			EE.E(" relay=");
			zpln.ImageTag(relay);
			EE.Enl();
			for(int iw = 0; iw <= npas; iw++) {
				EE.E("step iw=");
				EE.E(iw);
				steps[iw].Image("",0);
			}
		}
		 end of debugging code to relocate*/ 
		return 1;
	}
	//second phase, goback using the tx[] tables

	tt[0] = start;
	tt[itt - 1] = end; 
	for(int i = itt - 2; i > 0; i--) { // we go back from the end
		tt[i] = 0; 
		USHORT last = tt[i + 1],
			*ta = tx[i],
			ita = itx[i]; 
		for(int jj = 0; jj < ita; jj++) {
			int j = ta[jj]; // j must be in the forward way
			if(to[j].Off(last))
				continue; // must be parent of last tag
			tt[i] = j;
			break; // should always find one the first is ok
		}
		// error in the process this is a debugging message
		if(!tt[i]) {
			/* debugging code to relocate
			EE.E("invalid trackback step=");
			EE.E(i); 
			EE.E(" last=");
			zpln.ImageTag(last); 
			EE.E(" start=");
			zpln.ImageTag(start); 
			EE.E(" end=");
			zpln.ImageTag(end); 
			EE.E(" relay=");
			zpln.ImageTag(relay);
			EE.Enl();
			allsteps.Image("allsteps", 0);
			 end of debugging code to relocate */
			return 1;
		}
	}
	return 0;
}

// GP 2011 10 9  <<<<<<<<<<<<<<<<<<<<<<<  suggested to move that in PUZZLE
// this is more sensitive in performance that the previous ones
/* final expansion in nested mode of a specific BFTAG */
//void BFTAG::Expand(BFTAG * to, USHORT i) {
//	int n = 1;
//	while(n) {
//		n = 0;
//		for(int j = 2; j < /*puz.col + 2*/ BFTAG_BitSize; j++) {
//			if((j - i) && (*this).On(j)) {
//				BFTAG x = to[j];
//				x -= (*this);
//				if(x.IsNotEmpty()) {
//					(*this) |= x;
//					//n++;
//					n = 1;
//				}
//			}
//		} // end j
//	} // end while
//}

} //namespace skfr