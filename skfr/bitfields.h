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

/* that file contains classes managing bit fields in the program and a specific class, 
   BF_CONVERT doing specific operations on bit fields to improve the overall performance.

   The specific classes are

   BIT16 a bit field of size 16 bits mainly used as a 9 bits field.
   BIT32 a bit field of size 32 bits used  as a27 bit field for regions
   BIT81 a bit field to describe some "cell" properties in the 81 cells context
   BITCAND a bit field sized to a maximum of 320 bits but worked at the used dimension
           that bit field is used to describe candidates properties.
           the bit entry corresponds to the index in the table of candidates
   BITTAG  a bit field sized to a maximum equal to (BITCAND size * 2)
           that bitfield describe the tags properties
*/

#pragma once


#include "skfrtype.h"
#include "t_128.h"

namespace skfr {

extern const int BitCount[512];

/* class BIT16 can be used in any case one need a 16 bits field, 
      for example to store options.
   most uses in that program refer in fact to a 9 bits field

   9 bits field can describe many binary facts in the sudoku game,
   for example it can
   describe the digits candidate in a cell
   describe the relative cells occupied by a digit in a row, column, box
   describe the parity of candidates in a row, column, box ...
*/

//! a 9 bitfield to give position of candidate in a house and similar functions
class BF16 {
public:
	//! bitfield
	USHORT f;
	// constructors
	BF16() {
		f = 0;
	}               
	BF16(int i1) {
		f = 1 << i1;
	}
	BF16(int i1, int i2) {
		f = (1 << i1) | (1 << i2);
	}
	BF16(int i1, int i2, int i3) {
		f = (1 << i1) | (1 << i2) | (1 << i3);
	}
	BF16(int i1, int i2, int i3, int i4) {
		f = (1 << i1) | (1 << i2) | (1 << i3) | (1 << i4);
	}
	BF16(int i1, int i2, int i3, int i4, int i5) {
		f = (1 << i1) | (1 << i2) | (1 << i3) | (1 << i4) | (1 << i5);
	}

	//! set all 9 bits to 0
    inline void SetAll_0() {
		f = 0;
	} 
	//! set all 9 bits to 1
	inline void SetAll_1() {
		f = 0x1ff;
	}			
	//! indicate if all bits are 0
    inline int isEmpty() const {
		return (f == 0);
	}
	//! indicate if at least one bit is on
    inline int isNotEmpty() const {
		return f;
	}
	//! is bit at ch position on
	inline int On(int ch) const {
		return ((f & (1 << ch)));
	}		
	//! is bit at ch position off
	inline int Off(int ch) const {
		return (!(f & (1 << ch)));
	}	
	//! set bit at ch position
	inline void Set(USHORT ch) {
		f |= (1 << ch);
	}	
	//! reset bit at ch position
	inline void Clear(USHORT ch) {
		f &= ~(1 << ch);
	}
	inline BF16 operator &(BF16 & e) const {
		BF16 w;
		w.f = f & e.f;
		return w;
	}
	inline BF16 operator |(BF16 & e) {
		BF16 w;
		w.f = f | e.f;
		return w;
	}
	inline BF16 operator ^(BF16 & e) {
		BF16 w;
		w.f = f ^ e.f;
		return w;
	}
	inline BF16 operator -(BF16 & e) {
		BF16 w;
		w.f = f ^ (e.f & f);
		return w;
	}
	inline int  operator ==(BF16 & e) {
		return(e.f == f);
	}
	inline void operator &=(BF16 & e) {
		f &= e.f;
	}
	inline void operator |=(BF16 & e) {
		f |= e.f;
	}
	inline void operator ^=(BF16 & e) {
		f ^= e.f;
	}
	inline void operator -=(BF16 & e) {
		f ^= (f & e.f);
	}
	/*
	  inline USHORT GetOne() // valable pour un seul en mode très rapide pas de sécurité 
	  { if (f>16) {if(f>64) {if(f&128) return 8; else return 9;}
							else if(f&64) return 7; else return 6;}
			else  if(f<4)return f;//1 ou 2
				   else if(f&4) return 3; else if(f&8) return 4;else return 5;}
	*/
	//! give the first on bit position
	/** \return the position or 9 if there is no bit on */
	int  First() {
		for(int i = 0; i < 9; i++)
			if(On(i))
				return i;
		return 9;
	}
	// optimisable avec un tableau de 512 bytes
	//! get count of on bits
	//USHORT QC() {
	int bitCount() {
		//USHORT n = 0;
		//for(int i = 0 ; i < 9; i++) {
		//	if(On(i))
		//		n++;
		//}
		//return n;
		if(f & 0xFE00)
			return BitCount[f & 0xFF] + BitCount[f >> 8];
		return BitCount[f];
	}
	// optimisable avec un tableau de 512 bytes
	//! is it a pair of position
	int paire() {
		return (bitCount() == 2);
	}     
	//! get count of on bits and a string of their positions 1-9
	USHORT CountEtString(char *s) {
		USHORT n = 0;
		for (int i = 0 ;i < 9; i++) {
			if(On(i))
				s[n++]=(char)(i+'1');
		}
		s[n] = 0; 
		return n;
	}
	//! get a string representing the positions of on bit. 
	/**
	 * The string contains letters A-I or figures 1-9.
	 * \param lettre if 0 use letter A-I, if 1 use figures 1-9
	 */

	char * String(char * ws,int lettre = 0) {
		// static char ws[10];
		int n = 0;
		for (int i = 0; i < 9; i++) {
			if(On(i))
				ws[n++] = (char)(lettre ? i + 'A' : i + '1');
		}
        ws[n] = 0;
		return ws;
	}
	
}; // BF16

/* BIT32 is used mainly in that program as a 27 region bit field
   describing some binary properties fo these regions
*/


//! a class for bitfield (limited to 32 bits) and set of functions and operators
class BF32 {
public:
	UINT f;   // bitfield
	BF32() {               // constructor
		f = 0;
	}
	/// is bit at ch position on
	inline int On(int ch) {
		return ((f & (1 << ch)));
	}
	/// is bit at ch position off
	inline int Off(int ch) {
		return (!(f & (1 << ch)));
	}
	/// set bit at ch position
	inline void Set(USHORT ch) {
		f |= (1 << ch);
	}
	/// clear bit at ch position
	inline void Clear(USHORT ch) {
		f &= ~(1 << ch);
	}
	/// invert bit at ch position
	inline void Inv(USHORT ch) {
		f ^= (1 << ch);
	}
	inline BF32 operator -(BF32 & e) {
		BF32 w;
		w.f = f ^ (e.f & f);
		return w;
	}
};

/* BIT81 is, with BIT16 a key bit field in that program.
   The 81 bits of the field relate to the 81 cells of the grid.
   Again, that field can have may uses as, but not limited to

   cells controlled by one cell (not the same digit in these cells)
   description of a cell pattern
   unassigned cells
   rookeries (all candidates of the same digit)  
*/

//! Bitfield 81 positions (one per cell) and usual operations made on that field.
/** 
 * This class is used primarely to gather positions of a candidate in 
 * the grid<br> Using logical operations enable to identify pairs or 
 * triplet for instance
 */
class BF81 {
	bm128 ff;
public:

    BF81(){SetAll_0();}
    BF81(const BF81 &r) {*this = r;}
    BF81(const t_128 &r) {this->ff.bitmap128 = r;}
    BF81(int i1) {
		ff = bitSet[i1];
	}
    BF81(int i1, int i2) {
		ff = bitSet[i1];
		ff |= bitSet[i2];
	}
	//!Set all bits to 1
    inline void SetAll_1() {
		ff = maskLSB[81];
	}
	//!Set all bits to 0
    inline void SetAll_0() {
		ff.clear();
	}

	inline int On(int v) const {
		return ff.isBitSet(v);
	}
	inline int Off(int v) const {
		return ff.isBitSet(v) == 0;
	}
	inline void Set(int v) {
		ff.setBit(v);
	}
	//!Clear position <code>v</code>
    inline void Clear(int v) {
		ff.clearBit(v);
	}
	//!Is there any bit On
    inline int IsNotEmpty() const {
		return ff.isZero() == 0;
	}
	//! Is there no bit On
    inline int IsEmpty() const {
		return ff.isZero();
	}

	BF81 operator | (const BF81 & b) const {
		BF81 w(*this);
		w |= b;
		return w;
	}

    void operator |= (const BF81 & b) {
		ff |= b.ff;
	}

	BF81 operator & (const BF81 & b) const {
		BF81 w(*this);
		w.ff &= b.ff;
		return w;
	}

    void operator &= (const BF81 & b) {
		ff &= b.ff;
	}

	BF81 operator ^ (const BF81 & b) const {
		BF81 w(*this);
		w.ff ^= b.ff;
		return w;
	}

    void operator ^= (const BF81 & b) {
		ff ^= b.ff;
	}

	BF81 operator -(const BF81 & b) const {
		BF81 w(*this);
		w.ff -= b.ff;
		return w;
	}
	//! Clear all position that are on in <code>b</code>
    void operator -=(const BF81 & b) {
		ff -= b.ff;
	}

    int operator ==(const BF81 & b) const {
		return ff == b.ff;
	}

    int EstDans(const BF81 &fe) const {
		return ff.isSubsetOf(fe.ff);
	}

	//! Find the first position on
	/** \return found position or 128 if none */
	int First() const {
		for(int i = 0; i < 81; i++)
			if(On(i))
				return i;
		return 128;
	}     

	//! Count the on bits
    int Count() {
		return popcount_128(ff.bitmap128.m128i_m128i);
	}
	int String(int *r) const {
		int n = 0;
		static const unsigned char toPos[] = { //TODO: move to the global lookups
			0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8
		};
		int m = ff.nonzeroOctets(); // 16-bit mask of octets having non-zero bits
		int add8 = 0; //0 for lower 8 bits, 8 for higher 8 bits
		while(m) { //exit if no more octets with bits set
			if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
				m >>= 8;
				add8 = 8;
			}
			int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
			int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
			int octetValue = ff.bitmap128.m128i_u8[octetIndex];
			do {
				int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
				int bitIndex = (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
				r[n++] = (USHORT)bitIndex; //store
				octetValue ^= octetLSB; //clear the processed bit from the temporay copy
			} while(octetValue); //loop until all bits within this octed are processed
			m ^= octetIndexLSB; //clear the octet processed
		}
//		r[n]=0;
		return n;
	}
};


/* with BFCAND, we enter in a new kind of bit fields.
   BFCAND describes some candidates properties.
   some of theses properties as
   "is a weak link to another candidate defined" 
   are requiring a n * n bitfield where 'n' is the number of candidates.
   The bit field is designed for the maximum size, 
   but the process will be adjusted to the actual size.
   
   The size is 320 bits (bit 0 never used) which should be enough.
   The process is cancelled in case the number of candidates is higher
   
   some important uses of that field are 

   storing candidates contributing to a path
   storing defined links candidate to candidate
*/

//bitfield 320 bits used to store candidates used in a path

class BFCAND {
	bm128 ff[3];
public:
	BFCAND() {
		SetAll_0();
	}
	BFCAND(int a, int b) {
		SetAll_0();
		Set(a);
		Set(b);
	}
	BFCAND(const BFCAND &old) {
		(*this) = old;
	}
	void SetAll_0() {
		ff[0].clear();
		ff[1].clear();
		ff[2].clear();
	} 
	inline int On(int v) const {
		return (ff[v >> 7].isBitSet(v & 127));
	}
	inline int Off(int v) const {
		return (On(v) == 0);
	}
	inline void Set(int v) {
		ff[v >> 7].setBit(v & 127);
	}
};

/* BFTAG is the key bitfield in the tagging process.
   Each candidates has 2 corresponding tags
     one for the status "true" in the solution
     one for the status "false" in the solution.
   all candidates linked by bivalues are sharing the same couple of tags

   The main use of that bit field is a square table n * n where 'n' is the number of tags.

   The table is designed to the maximum (2 * BFCAND size), but processed according to the actual size.
   bits 0 and 1 are not used.
*/

#define BFTAG_size 20
#define BFTAG_BitSize 20*32

/// \brief Bitfield 640 bits used in chains and within a layer in the solver.
///
/// <b>WARNING </b>: using bit at position 0 implies error when using BFTAG::First<br>
/// This bitfield uses 20 32bits unsigned int.<br>
/// Size of this bitfield class can be changed by 2 <code># define</code> lines<br>
/// A bit represents normally a tag.<br>
/// Bivalues are using 2 consecutive bits  (low order bit 0,1; same value for other bits); this is used
/// in the following method BFTAG::Layers, BFTAG::Layer_Conflict, BFTAG::FLayer, BFTAG::FInv<br>
/// For performance reason, the field is processed partially depending of the number of bit used. 
/// <code>static int isize</code> gives the number of int used and is used by all operator and by 
/// BFTAG::InitUn, BFTAG::NonNul, BFTAG::Nul<br>
/// The global variable <code>col</code> gives the number of bits (col+2)(used for optimization 
/// in the following method : 
/// BFTAG::ImageMarques, BFTAG::Count, BFTAG::Inverse, BFTAG::String, BFTAG::First)<br>
/// <b>WARNING </b>: <br>
/// 1 - As <code>col</code> is a global variable, be sure to give right value to it before calling one of these
/// method. <br>
/// 2 - All method are not thread safe due to the usage of static member as temporary variables BFTAG::io and
/// BFTAG::jo.
class BFTAG {
	bm128 ff[5];
public:
	enum init {
		NoInit = 0,
		InitZero = 1,
		InitOne = 2,
		InitFalseState = 3,
		InitTrueState = 4
	};
	BFTAG(const BFTAG &x) {
		(*this) = x;
	}
	BFTAG(const init initMode = InitZero) {
		switch(initMode) {
			case NoInit:
				break;
			case InitOne:
				SetAll_1();
				break;
			case InitFalseState:
				for(int i = 0; i < 5; i++)
					ff[i] = false128;
				break;
			case InitTrueState:
				for(int i = 0; i < 5; i++)
					ff[i] = true128;
				break;
			default: // InitZero
				SetAll_0();
				break;
		}
	}
	///\brief Clear all bits
	void SetAll_0();
	///\brief Set all bits
	void SetAll_1();
	///\brief is bit in position <code>v</code> On
	inline bool On(int v) const {
		return (ff[v >> 7].isBitSet(v & 127));
	}
	///\brief is bit in position <code>v</code> Off
	inline bool Off(int v) const {
		return (!On(v));
	}
	///\brief Set bit in position <code>v</code> to On
	inline void Set(int v) {
		ff[v >> 7].setBit(v & 127);
	}
	///\brief Clear bit in position <code>v</code> 
	inline void Clear(int v) {
		ff[v >> 7].clearBit(v & 127);
	}
	BFTAG Inverse() const;
	void operator &= (const BFTAG &z2);
	void operator |= (const BFTAG &z2);
	void operator -= (const BFTAG &z2);
	bool operator == (const BFTAG &z2) const;
	bool substract(const BFTAG &z2); //perform -= and return IsNotEmpty()
	bool IsNotEmpty() const;
	bool IsEmpty() const;
	///\brief get on bits count
	int Count() const;
	BFTAG TrueState() const;
	BFTAG FalseState() const;
	///\brief fill the array (first parameter) with the list of position of On Bit
	/// 
	///\param tr pointer to an array to be filled
	///\param n used to return number of position
	void String(USHORT *tr, USHORT &n) const;

	// new features not tested in preparation
	int SearchChain(const BFTAG *to, USHORT i, USHORT j);
	int SearchCycle(const BFTAG *to, USHORT i, const BFTAG &loop);
	int SearchCycleChain(const BFTAG *to, USHORT i, USHORT relay, const BFTAG &loop);
	int TrackBack(const BFTAG *to, USHORT start, USHORT end, USHORT * tt,
		USHORT & itt, USHORT relay) const;
	//void Expand(BFTAG * to, USHORT i); // in nested mode, expansion limited to one tag
};

} // namespace skfr