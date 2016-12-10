#ifndef BITMASK768_H_INCLUDED

#define BITMASK768_H_INCLUDED

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "tables.h"
#include "t_128.h"
#include <immintrin.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <intrin.h> //MS _BitScanForward
#pragma intrinsic(_BitScanForward)
#endif //compiler

#if 0
struct bm256 {
private:
	inline static __m128i andnot(const __m128i l, const __m128i r) {return _mm_andnot_si128(l, r);};
	inline static bool equals(const __m128i l, const __m128i r) {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(l, r));};
	inline __m128i operator& (const __m128i r) const {return _mm_and_si128(bitmap128.m128i_m128i, r);};
public:
	t_128 bitmap128;
	bm256() {};
	bm128(const bm128 &v) : bitmap128(v.bitmap128) {};
	bm128(const __m128i &v) {bitmap128.m128i_m128i = v;};
	bm128(const t_128 &v) {bitmap128.m128i_m128i = v.m128i_m128i;};
	//bm128(const t_128 v) {bitmap128.m128i_m128i = v.m128i_m128i;};
	inline __m128i operator| (const bm128 &r) const {return _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline bool operator== (const bm128& r) const {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
	inline bool operator!= (const bm128& r) const {return 0xFFFF != _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
	inline __m128i operator& (const bm128 &r) const {return _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator&= (const bm128& r) {bitmap128.m128i_m128i = _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const bm128& r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const __m128i r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r);};
	inline void operator^= (const bm128& r) {bitmap128.m128i_m128i = _mm_xor_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator-= (const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, maskLSB[81].m128i_m128i);}; //81-complementary
	inline void operator<<= (const int bits) {bitmap128.m128i_m128i = _mm_slli_epi16(bitmap128.m128i_m128i, bits);};
	inline bool isDisjoint(const bm128& r) const {return equals(andnot(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i), bitmap128.m128i_m128i);};
	//inline bool slow isDisjoint(const bm128& r) const {return equals(_mm_and_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i), _mm_setzero_si128());};
	inline int mask8() const {return _mm_movemask_epi8(bitmap128.m128i_m128i);}
	inline int toInt32() const {return _mm_cvtsi128_si32(bitmap128.m128i_m128i);}
	inline bool isBitSet(const int theBit) const {return equals(*this & bitSet[theBit].m128i_m128i, bitSet[theBit].m128i_m128i);};
	inline void setBit(const int theBit) {*this |= bitSet[theBit].m128i_m128i;};
	inline void clearBit(const int theBit) {bitmap128.m128i_m128i = _mm_andnot_si128(bitSet[theBit].m128i_m128i, bitmap128.m128i_m128i);};
	inline void clearBits(const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	inline void clearBits(const bm128& r, const bm128& r1) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, r1.bitmap128.m128i_m128i);};
	inline void clear() {bitmap128.m128i_m128i = _mm_setzero_si128();};
	inline bool isSubsetOf(const bm128 &s) const {return equals(s.bitmap128.m128i_m128i, _mm_or_si128(bitmap128.m128i_m128i, s.bitmap128.m128i_m128i));}
	inline bool clearsAll(const bm128 &s) const {return 1 == _mm_testc_si128(s.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}
	inline bool operator< (const bm128 &rhs) const {
		if(bitmap128.m128i_u64[1] < rhs.bitmap128.m128i_u64[1]) return true;
		if(bitmap128.m128i_u64[1] > rhs.bitmap128.m128i_u64[1]) return false;
		return bitmap128.m128i_u64[0] < rhs.bitmap128.m128i_u64[0];
	}
	inline void operator= (const bm128 &rhs) {bitmap128.m128i_m128i = rhs.bitmap128.m128i_m128i;};
	inline void loadUnaligned (const void *p) {bitmap128.m128i_m128i = _mm_loadu_si128((const __m128i *)p);};
	inline void invalidate() {bitmap128.m128i_m128i = maskffff.m128i_m128i;};
	//inline bool isInvalid() const {return equals(bitmap128.m128i_m128i, maskffff.m128i_m128i);};
	inline bool isInvalid() const {return 1 == _mm_test_all_ones(bitmap128.m128i_m128i);};
	inline static bool isZero(const __m128i &r) {return equals(r, _mm_setzero_si128());};
	inline bool isZero() const {return equals(bitmap128.m128i_m128i, _mm_setzero_si128());};
	inline static bm128* allocate(const int size) {return (bm128*)_mm_malloc(size * sizeof(bm128), 16);};
	inline static void deallocate(void *ptr) {_mm_free(ptr);};
	int toPseudoPuzzle(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, 0 : digits[i]; return n;}
	int toPuzzle(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, digits[i] : 0; return n;}
	int toPseudoPuzzleString(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, '.' : digits[i] + '0'; return n;}
	int toPuzzleString(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, digits[i] + '0' : '.'; return n;}
	void toMask81(char* r) const {for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? '1' : '.';}
	void toMask81(const char c, char* r) const {for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? c : '.';}
	void toMask128(char* r) const {for(int i = 0; i < 128; i++) r[i] = isBitSet(i) ? '1' : '.';}
	inline int popcount_128() const {
		//http://dalkescientific.blogspot.com/2008/06/molecular-fingerprints.html
		//see also http://bmagic.sourceforge.net/bmsse2opt.html
		const __m128i msk55 = _mm_set1_epi32(0x55555555);
		const __m128i msk33 = _mm_set1_epi32(0x33333333);
		const __m128i msk0F = _mm_set1_epi32(0x0F0F0F0F);
		const __m128i mul01 = _mm_set1_epi32(0x01010101);

		//xmm -= ((xmm >> 1) & 0x55555555);
		__m128i xmm = bitmap128.m128i_m128i;
		__m128i tmp = _mm_and_si128(_mm_srli_epi32(xmm,1), msk55);
		xmm = _mm_sub_epi32(xmm, tmp);
		//xmm = (xmm & 0x33333333) + ((xmm >> 2) & 0x33333333);
		tmp = _mm_and_si128(_mm_srli_epi32(xmm, 2), msk33);
		xmm = _mm_add_epi32(_mm_and_si128(xmm, msk33),tmp);
		//xmm = (xmm + (xmm >> 4)) & 0x0F0F0F0F;
		tmp = _mm_srli_epi32(xmm,4);
		xmm = _mm_and_si128(_mm_add_epi32(xmm,tmp),msk0F);
		// .. mix up
		tmp = _mm_shuffle_epi32(xmm, _MM_SHUFFLE(3,3,1,1));
		xmm = _mm_add_epi32(tmp,xmm);
		xmm = _mm_srli_epi64(_mm_mul_epu32(xmm,mul01), 24);
		return ((unsigned char*)&xmm)[0]+((unsigned char*)&xmm)[8];
	}
    inline unsigned int nonzeroOctets() const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, _mm_setzero_si128()));}
    inline unsigned int diffOctets(const bm128 &rhs) const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, rhs.bitmap128.m128i_m128i));}
	int getFirstBit1Index() const {
		unsigned int m = nonzeroOctets(); // 16-bit mask of octets having non-zero bits
		if(m) { //exit if no more octets with bits set
			int add8 = 0;
			if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
				m >>= 8;
				add8 = 8;
			}
			int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
			int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
			int octetValue = bitmap128.m128i_u8[octetIndex];
			int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
			return (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
		}
		return -1;
	}
	int getPositions(unsigned char *positions) const {
		int n = 0;
		//for(int i = 0; i < 81; i++)
		//	if(isBitSet(i))
		//		positions[n++] = (unsigned char)i;
		int m = nonzeroOctets(); // 16-bit mask of octets having non-zero bits
		int add8 = 0; //0 for lower 8 bits, 8 for higher 8 bits
		while(m) { //exit if no more octets with bits set
			if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
				m >>= 8;
				add8 = 8;
			}
			int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
			int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
			int octetValue = bitmap128.m128i_u8[octetIndex];
			do {
				int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
				int bitIndex = (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
				positions[n++] = bitIndex; //store
				octetValue ^= octetLSB; //clear the processed bit from the temporay copy
			} while(octetValue); //loop until all bits within this octed are processed
			m ^= octetIndexLSB; //clear the octet processed
		}
		return n;
	}
	inline void transposeSlice(const bm128 &src) //http://mischasan.wordpress.com/2011/07/24/what-is-sse-good-for-transposing-a-bit-matrix/
	{
		bm128 x(src);
		for (int i = 0; i < 8; i++) {
			bitmap128.m128i_u16[7-i] = _mm_movemask_epi8(x.bitmap128.m128i_m128i);
			x = _mm_slli_epi64(x.bitmap128.m128i_m128i, 1);
		}
	}
};
#endif
template <int maxElements> class bit_masks {
//    int __builtin_ia32_ptestc256 (v4di,v4di,ptest)
//    int __builtin_ia32_ptestnzc256 (v4di,v4di,ptest)
//    int __builtin_ia32_ptestz256 (v4di,v4di,ptest)

	bm128 aBits[maxElements / 128];

//	inline void clear() {
//		for(int i = 0; i < maxElements / 128; i++)
//			aBits[i].clear();
//	}
	inline void setAll() {
		for(int i = 0; i < maxElements / 128; i++)
			aBits[i] = maskffff;
	}
	void initSetMask(int numSets) {
		//clear();
//		int j = maxElements - numSets;
//		for(int i = maxElements / 128 - 1; j > 0 ; i--, j -= 128) {
//			if(j >= 128) {
//				aBits[i] |= maskffff.m128i_m128i;
//			}
//			else {
//				aBits[i] |= _mm_andnot_si128(maskLSB[128 - 1 - j].m128i_m128i, maskffff.m128i_m128i);
//			}
//		}
		setAll();
		int j = maxElements - numSets;
		for(int i = maxElements / 128 - 1; j > 0 ; i--, j -= 128) {
			if(j >= 128) {
				aBits[i].clear();
			}
			else {
				aBits[i].clearBits(_mm_andnot_si128(maskLSB[128 - 1 - j].m128i_m128i, maskffff.m128i_m128i));
			}
		}
	}
	void static fromBm128(int const srcRows, const bm128 * const src, bit_masks dest[81]) {
		//http://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/
		//http://mischasan.wordpress.com/2011/07/24/what-is-sse-good-for-transposing-a-bit-matrix/, mn
		//http://hackers-delight.org.ua/048.htm
		//https://www.google.bg/#q=bit+matrix+transpose
		bm128 ss;
		int nSlices = srcRows / 16 + 1;
		if(srcRows >= maxElements) {
			nSlices = maxElements / 128 * 8;
		}
		const bm128 * const s = src;
		for (int slice = 0; slice < nSlices; slice++) { //process 16 rows from the source simultaneously
			//process first 80 bits
			for (int srcCol = 0; srcCol < 10; srcCol++) { //process 8 bits per "column" simultaneously
				for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source
					assert(slice*16+srcSliceRow < maxElements);
					ss.bitmap128.m128i_u8[srcSliceRow] = s[slice*16+srcSliceRow].bitmap128.m128i_u8[srcCol];
				}
				ss.transposeSlice(ss); // 16 bits * 8 columns for the target
				for (int destRow = 0; destRow < 8; destRow++) {
					assert(srcCol * 8 + destRow < 80);
					assert(slice / 8 < maxElements / 128);
					dest[srcCol * 8 + destRow].aBits[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[destRow];
				}
			}
			//process 81-st bit
			for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source, only first bit is used
				assert(slice+srcSliceRow < maxElements);
				ss.bitmap128.m128i_u8[srcSliceRow] = s[slice+srcSliceRow].bitmap128.m128i_u8[10];
			}
			ss = _mm_slli_epi64(ss.bitmap128.m128i_m128i, 7); // move bit 0 to bit 7
			ss.bitmap128.m128i_u16[0] = _mm_movemask_epi8(ss.bitmap128.m128i_m128i);
			assert(slice / 8 < maxElements / 128);
			dest[80].aBits[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[0];
		}
	}
public:
	static const int maxSize = maxElements;
	inline void hitOnly(const bit_masks &s, const bit_masks &hittingMask) {
		for(int i = 0; i < maxElements / 128; i++) {
			aBits[i].bitmap128.m128i_u64[0] = s.aBits[i].bitmap128.m128i_u64[0] & ~hittingMask.aBits[i].bitmap128.m128i_u64[0];
			aBits[i].bitmap128.m128i_u64[1] = s.aBits[i].bitmap128.m128i_u64[1] & ~hittingMask.aBits[i].bitmap128.m128i_u64[1];
			//aBits[i].clearBits(hittingMask.aBits[i], s.aBits[i]);
		}
//		for(int i = 0; i < maxElements / 128; i += 2) {
//			__m256d d;
//			__m256d s1, s2;
//			s1 = _mm256_loadu_pd((const double *)&hittingMask.aBits[i]);
//			s2 = _mm256_loadu_pd((const double *)&s.aBits[i]);
//			d  = _mm256_andnot_pd(s1, s2);
//			_mm256_storeu_pd((double *) (&aBits[i]), d);
//		}
	}
	inline bool isHittingAll(const bit_masks &hittingMask) const {
		bm128 bm;
		for(int i = 0; i < maxElements / 128; i++) {
			//if(aBits[i].bitmap128.m128i_u64[0] & ~hittingMask.aBits[i].bitmap128.m128i_u64[0]) return false;
			//if(aBits[i].bitmap128.m128i_u64[1] & ~hittingMask.aBits[i].bitmap128.m128i_u64[1]) return false;
			if(!aBits[i].clearsAll(hittingMask.aBits[i]))
				return false;
		}
		return true;
	}
	void static bm128ToIndex(const bm128 *sets, int nsets, bit_masks &setMask, bit_masks hittingMasks[81]) {
		fromBm128(nsets, sets, hittingMasks);
		setMask.initSetMask(nsets);
	}
//	inline int hit(const bit_masks &s, const bit_masks &hittingMask) {
//		//set hittingMask bits in aBits and return the first unhit (i.e. zero bit) index
//		hitOnly(s, hittingMask);
//		unsigned int bAdd;
//		unsigned int bytePos;
//		for(int i = 0; i < maxElements / 128; i++) {
//			if(0xFFFF != (bytePos = _mm_movemask_epi8(_mm_cmpeq_epi8(aBits[i].bitmap128.m128i_m128i, maskffff.m128i_m128i)))) {
//				bAdd = i << 4; //*16
//				//3 alternatives for the following operation
//
//				//1) fast but uses Intel-specific intrinsic
//#if defined(__INTEL_COMPILER)
//				bytePos = _bit_scan_forward(~bytePos) + bAdd;
//#elif defined(__GNUC__)
//				bytePos = __builtin_ctz(~bytePos) + bAdd;
//#elif defined(_MSC_VER)
//				//2) uses Microsoft-specific intrinsic
//				using namespace std;
//				unsigned long bytePosL;
//		//#if UINT_MAX == 0xffff
//				_BitScanForward(&bytePosL,~bytePos);
//		//#else
//		//		_BitScanForward64(&bytePosL,~bytePos);
//		//#endif //UINT_MAX
//				bytePos = (unsigned int)bytePosL + bAdd;
//#else
//				//3) standard but slower
//				bytePos = (~bytePos);
//				if(bytePos & 0xff)
//					bytePos = lowestBit[(bytePos & 0xff) - 1] + bAdd;
//				else
//					bytePos = lowestBit[((bytePos >> 8)  & 0xff) - 1] + bAdd + 8;
//#endif //compiler
//
//
//				return (bytePos << 3) + lowestBit[((unsigned char*)&aBits[0])[bytePos]];
//				//return (bytePos << 3) + _bit_scan_forward(~((unsigned char*)&aBits[0])[bytePos]);
//				//_mm_prefetch(((char *)&newSetMask) + 0x60, _MM_HINT_T0); //unsignificant improvement
//			}
//		}
//		return INT_MAX;
//	}
	inline int firstUnhit() {
		for(int i = 0; i < maxElements / 128; i++) {
			if(aBits[i].bitmap128.m128i_u64[0]) {
				return i * 128 + __builtin_ctzll((aBits[i].bitmap128.m128i_u64[0]));
			}
			if(aBits[i].bitmap128.m128i_u64[1]) {
				return i * 128 + 64 + __builtin_ctzll((aBits[i].bitmap128.m128i_u64[1]));
			}
		}
		return INT_MAX;
	}
	inline int copyAlive(const sizedUset *original, sizedUset *target, int target_size, const bm128 &deadClues) const {
		int num_inserted = 0;
		for(int i = 0; i < maxElements / 128; i++) {
			for(int j = 0; j < 2; j++) {
				int base = i * 128 + j * 64;
				for(uint64_t bits = aBits[i].bitmap128.m128i_u64[j]; bits; bits &= (bits - 1)) {
					int offset = __builtin_ctzll(bits);
					sizedUset s = original[base + offset];
					s.clearBits(deadClues);
					s.setSize(); //calculate new size for the later reordering
//					if(s.getSize() == 0) {
//						//unhit UA within the dead clues
//						printf("unhit UA within the dead clues identified during the consolidation\n");
//						return false;
//					}
					target[num_inserted++] = s;
					if(num_inserted >= target_size || s.getSize() == 0) return num_inserted; //possibly with zero length UA
				}
			}
		}
		return num_inserted;
	}
};

struct BitMask768 : public bit_masks<768> {};

//struct UATable {
//	bm128 *rows;
//	int size;
//	UATable(const UATable &src, int hitClue) {
//		rows = bm128::allocate(src.size);
//		size = 0;
//		const bm128 &mask = bitSet[hitClue];
//		for(int r = 0; r < src.size; r++) {
//			if(mask.isDisjoint(src.rows[r])) {
//				rows[size++] = src.rows[r];
//			}
//		}
//	};
//	UATable(const UATable &src, const bm128 &mask) {
//		rows = bm128::allocate(src.size);
//		size = 0;
//		for(int r = 0; r < src.size; r++) {
//			if(mask.isDisjoint(src.rows[r])) {
//				rows[size++] = src.rows[r];
//			}
//		}
//	};
//	UATable() : rows(NULL), size(0) {};
//	~UATable() {
//		if(rows) bm128::deallocate(rows);
//	};
//	void setSize(int theSize) {
//		size = theSize;
//		if(rows) bm128::deallocate(rows);
//		rows = bm128::allocate(size);
//	}
//};
//
//struct fastUATable {
//	const UATable &table;
//	BitMask768 hittingMasks[81]; //one index per cell
//	BitMask768 validRowsIndex[81]; //stack, one index per clue, 1=valid, 0=invalid
//	fastUATable(const UATable &srcTable, int cellNumber) : table(srcTable) {
//		BitMask768 &theIndex = validRowsIndex[cellNumber];
//		for(int c = 0; c < 81; c++) {
//			hittingMasks[c].setAll();
//		}
//		theIndex.setAll(); //all rows are valid
//		int truncatedSize = table.size;
//		if(truncatedSize >= 768) {
//			truncatedSize = 768;
//		}
//		else { // we have empty bits at the end; clear them from the index then forget the size
//			int firstPartialBlock = (truncatedSize - 1) << 7;
//			int fillerSizeInPartialBlock = 127 - ((truncatedSize - 1) & 0x7f);
//			theIndex.aBits[firstPartialBlock].clearBits(maskLSB[fillerSizeInPartialBlock]);
//			for(int e = firstPartialBlock + 1; e < 6; e++) {
//				theIndex.aBits[e].clear();
//			}
//		}
//		//create the hitting masks (i.e. the bitmap indexes per cell position)
//		//todo: optimize for better cache hits, use small squares, or at least more reads but less writes
//		for(int r = 0; r < truncatedSize; r++) {
//			const bm128 &row = table.rows[r];
//			for(int c = 0; c < 81; c++) {
//				if(row.isBitSet(c)) {
//					hittingMasks[c].aBits[r << 7].clearBit(r & 0x7f);
//				}
//			}
//		}
//		//now we have indexes created and validRowsIndex[cellNumber] set to '1' up to the effective size and to '0' till the end
//	};
//};

//struct tresholds {
//	int positions[81]; //non-zero position means valid treshold
//	int minClueIndex;
//	int maxClueIndex;
//	UATable *flatTables[81][16]; //valid prior to switching to bitmap indexing
//	fastUATable *indexedTables[81][16]; //valid after swithing to bitmap indexing
//	tresholds() {
//		memset(positions, 0, sizeof(int) * 81); //invalidate positions
//		minClueIndex = maxClueIndex = 0;
//	}
//};

#endif //BITMASK768_H_INCLUDED
