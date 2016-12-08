#ifndef BITMASK768_H_INCLUDED

#define BITMASK768_H_INCLUDED

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "tables.h"
#include "t_128.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <intrin.h> //MS _BitScanForward
#pragma intrinsic(_BitScanForward)
#endif //compiler

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
	const int maxSize = maxElements;
	inline void hitOnly(const bit_masks &s, const bit_masks &hittingMask) {
		for(int i = 0; i < maxElements / 128; i++) {
//			aBits[i] = s.aBits[i] | hittingMask.aBits[i];
//			//aBits[i].bitmap128.m128i_u64[0] = s.aBits[i].bitmap128.m128i_u64[0] | hittingMask.aBits[i].bitmap128.m128i_u64[0];
//			//aBits[i].bitmap128.m128i_u64[1] = s.aBits[i].bitmap128.m128i_u64[1] | hittingMask.aBits[i].bitmap128.m128i_u64[1];
			aBits[i].clearBits(hittingMask.aBits[i], s.aBits[i]);
		}
	}
	inline bool isHittingAll(const bit_masks &hittingMask) const {
		bm128 bm;
		for(int i = 0; i < maxElements / 128; i++) {
//			bm = aBits[i] | hittingMask.aBits[i];
//			if(!bm.isInvalid()) return false;
//			//if((aBits[i].bitmap128.m128i_u64[0] | hittingMask.aBits[i].bitmap128.m128i_u64[0]) != 0xffffffffffffffff) return false;
//			//if((aBits[i].bitmap128.m128i_u64[1] | hittingMask.aBits[i].bitmap128.m128i_u64[1]) != 0xffffffffffffffff) return false;
			if(!aBits[i].clearsAll(hittingMask.aBits[i])) return false;
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
//			if(aBits[i].bitmap128.m128i_u64[0] != 0xffffffffffffffff) {
//				return i * 128 + __builtin_ctzll(~(aBits[i].bitmap128.m128i_u64[0]));
//			}
//			if(aBits[i].bitmap128.m128i_u64[1] != 0xffffffffffffffff) {
//				return i * 128 + 64 + __builtin_ctzll(~(aBits[i].bitmap128.m128i_u64[1]));
//			}
			if(aBits[i].bitmap128.m128i_u64[0]) {
				return i * 128 + __builtin_ctzll((aBits[i].bitmap128.m128i_u64[0]));
			}
			if(aBits[i].bitmap128.m128i_u64[1]) {
				return i * 128 + 64 + __builtin_ctzll((aBits[i].bitmap128.m128i_u64[1]));
			}
		}
		return INT_MAX;
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
