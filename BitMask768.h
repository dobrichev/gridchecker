#ifndef BITMASK768_H_INCLUDED

#define BITMASK768_H_INCLUDED

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "tables.h"
#include "t_128.h"
#include <immintrin.h>

class dclues : public bm128 {
public:
//	inline void setBit(const int theBit) {
//		if(theBit > 63) {
//			bitmap128.m128i_u64[1] |= ((uint64_t)1 << (theBit & 63));
//		}
//		else {
//			bitmap128.m128i_u64[0] |= ((uint64_t)1 << theBit);
//		}
//	}
//	inline bool isBitSet(const int theBit) const {
//		//return bm128::isBitSet(theBit);
//		if(theBit > 63) {
//			return 0 != (bitmap128.m128i_u64[1] & (((uint64_t)1) << (theBit & 63)));
//		}
//		else {
//			return 0 != (bitmap128.m128i_u64[0] & (((uint64_t)1) << theBit));
//		}
//	}
};

//typedef bm128 dead_clues_type;
typedef dclues dead_clues_type;


//#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
//#include <intrin.h> //MS _BitScanForward
//#pragma intrinsic(_BitScanForward)
//#endif //compiler

typedef union {
	__m256i b256;
	//__m256d b256d;
	__m128i b128[2];
	uint64_t u64[4];
	uint16_t u16[16];
} bm256;

template <int maxElements> class bit_masks {
//    int __builtin_ia32_ptestc256 (v4di,v4di,ptest)
//    int __builtin_ia32_ptestnzc256 (v4di,v4di,ptest)
//    int __builtin_ia32_ptestz256 (v4di,v4di,ptest)
	static const int maxWords = maxElements / 256;

	bm256 aBits[maxElements / 256];
	int actualWords;

	inline int getNumWords() const {
		if(maxWords > 3) return actualWords; //should be resolved at compile time
		return maxWords;
	}

//	inline void clear() {
//		for(int i = 0; i < maxElements / 128; i++)
//			aBits[i].clear();
//	}
	inline void setAll() {
		__m256i all1 = _mm256_set1_epi64x(-1);
		for(int i = 0; i < maxWords; i++) {
			aBits[i].b256 = all1;
			//aBits[i].b128[0] = maskffff.m128i_m128i;
			//aBits[i].b128[1] = maskffff.m128i_m128i;
		}
	}
	void initSetMask(int numSets) {
		actualWords = (numSets + 255) / 256;
		if(actualWords > maxWords) actualWords = maxWords;
		setAll();
		int j = maxElements - numSets;
//		for(int i = maxElements / 128 - 1; j > 0 ; i--, j -= 128) {
//			if(j >= 128) {
//				aBits[i].clear();
//			}
//			else {
//				aBits[i].clearBits(_mm_andnot_si128(maskLSB[128 - 1 - j].m128i_m128i, maskffff.m128i_m128i));
//			}
//		}
		for(int i = maxWords - 1; j > 0 ; i--, j -= 256) {
			if(j >= 256) {
				aBits[i].b256 = _mm256_setzero_si256();
				//aBits[i].b128[0] = _mm_setzero_si128();
				//aBits[i].b128[1] = _mm_setzero_si128();
			}
			else {
				if(j >= 128) {
					aBits[i].b128[1] = _mm_setzero_si128();
					j -= 128;
					aBits[i].b128[0] = maskLSB[128 - j].m128i_m128i;
				}
				else {
					aBits[i].b128[1] = maskLSB[128 - j].m128i_m128i;
				}
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
			nSlices = maxElements / 16;
		}
		const bm128 * const s = src;
		for (int slice = 0; slice < nSlices; slice++) { //process 16 rows from the source simultaneously
			//process first 80 bits
			for (int srcCol = 0; srcCol < 10; srcCol++) { //process 8 bits per "column" simultaneously
				for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source
					ss.bitmap128.m128i_u8[srcSliceRow] = s[slice*16+srcSliceRow].bitmap128.m128i_u8[srcCol];
				}
				ss.transposeSlice(ss); // 16 bits * 8 columns for the target
				for (int destRow = 0; destRow < 8; destRow++) {
					//dest[srcCol * 8 + destRow].aBits[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[destRow];
					dest[srcCol * 8 + destRow].aBits[slice / 16].u16[slice % 16] = ss.bitmap128.m128i_u16[destRow];
				}
			}
			//process 81-st bit
			for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source, only first bit is used
				assert(slice+srcSliceRow < maxElements);
				ss.bitmap128.m128i_u8[srcSliceRow] = s[slice*16+srcSliceRow].bitmap128.m128i_u8[10];
			}
			ss = _mm_slli_epi64(ss.bitmap128.m128i_m128i, 7); // move bit 0 to bit 7
			ss.bitmap128.m128i_u16[0] = _mm_movemask_epi8(ss.bitmap128.m128i_m128i);
			assert(slice / 8 < maxElements / 128);
			//dest[80].aBits[slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[0];
			dest[80].aBits[slice / 16].u16[slice % 16] = ss.bitmap128.m128i_u16[0];
		}
	}
public:
	static const int maxSize = maxElements;
	inline void hitOnly(const bit_masks &s, const bit_masks &hittingMask) {
		actualWords = s.actualWords;
//		for(int i = 0; i < maxElements / 128; i++) {
//			//aBits[i].bitmap128.m128i_u64[0] = s.aBits[i].bitmap128.m128i_u64[0] & ~hittingMask.aBits[i].bitmap128.m128i_u64[0];
//			//aBits[i].bitmap128.m128i_u64[1] = s.aBits[i].bitmap128.m128i_u64[1] & ~hittingMask.aBits[i].bitmap128.m128i_u64[1];
//			aBits[i].clearBits(hittingMask.aBits[i], s.aBits[i]);
//		}
		//for(int i = 0; i < maxWords; i++) {
		for(int i = 0; i < getNumWords(); i++) {
			//aBits[i].b256d = _mm256_andnot_pd(hittingMask.aBits[i].b256d, s.aBits[i].b256d);
			aBits[i].b256 = _mm256_castpd_si256(_mm256_andnot_pd(_mm256_castsi256_pd(hittingMask.aBits[i].b256), _mm256_castsi256_pd(s.aBits[i].b256)));
		}
//		for(int i = 0; i < getNumWords(); i++) {
//			aBits[i].b128[0] = _mm_andnot_si128(hittingMask.aBits[i].b128[0], s.aBits[i].b128[0]);
//			aBits[i].b128[1] = _mm_andnot_si128(hittingMask.aBits[i].b128[1], s.aBits[i].b128[1]);
//		}
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
//		for(int i = 0; i < maxElements / 128; i++) {
//			//if(aBits[i].bitmap128.m128i_u64[0] & ~hittingMask.aBits[i].bitmap128.m128i_u64[0]) return false;
//			//if(aBits[i].bitmap128.m128i_u64[1] & ~hittingMask.aBits[i].bitmap128.m128i_u64[1]) return false;
//			if(!aBits[i].clearsAll(hittingMask.aBits[i]))
//				return false;
//		}
//		return true;
//		for(int i = 0; i < maxElements / 128; i += 2) {
//			__m256i s1;
//			__m256i s2;
//			s1 = _mm256_loadu_si256((__m256i*)&hittingMask.aBits[i]);
//			s2 = _mm256_loadu_si256((__m256i*)&aBits[i]);
//			if(0 == _mm256_testc_si256(s1, s2))
//				return false;
//		}
//		return true;
		//for(int i = 0; i < maxWords; i++) {
		for(int i = 0; i < getNumWords(); i++) {
			if(0 == _mm256_testc_si256(hittingMask.aBits[i].b256, aBits[i].b256))
				return false;
		}
		return true;
//		for(int i = 0; i < maxElements / 256; i++) {
//			if(0 == _mm_testc_si128(hittingMask.aBits[i].b128[0], aBits[i].b128[0]))
//				return false;
//			if(0 == _mm_testc_si128(hittingMask.aBits[i].b128[1], aBits[i].b128[1]))
//				return false;
//		}
//		return true;
	}
	static bool isHitting(const __m256i hittingMask, const __m256i setMask) {
		return _mm256_testc_si256(hittingMask, setMask);
	}
	static __m256i hitWord(const __m256i hittingMask, const __m256i setMask) {
		return _mm256_castpd_si256(_mm256_andnot_pd(_mm256_castsi256_pd(hittingMask), _mm256_castsi256_pd(setMask)));
	}
	__m256i getTopWord() const {
		return aBits[0].b256;
	}
	static int firstUnhitWord(const __m256i setMask) {
		uint64_t bits;
		if((bits = _mm256_extract_epi64(setMask, 0))) {
			return __builtin_ctzll(bits); //almost always this is the only test
		}
		if((bits = _mm256_extract_epi64(setMask, 1))) {
			return 64 + __builtin_ctzll(bits);
		}
		if((bits = _mm256_extract_epi64(setMask, 2))) {
			return 128 + __builtin_ctzll(bits);
		}
		if((bits = _mm256_extract_epi64(setMask, 3))) {
			return 192 + __builtin_ctzll(bits);
		}
		return INT_MAX;
	}
	void static bm128ToIndex(const bm128 *sets, int nsets, bit_masks &setMask, bit_masks hittingMasks[81]) {
		fromBm128(nsets, sets, hittingMasks);
		setMask.initSetMask(nsets);
	}

	inline int firstUnhit() {
//		for(int i = 0; i < maxElements / 128; i++) {
//			if(aBits[i].bitmap128.m128i_u64[0]) {
//				return i * 128 + __builtin_ctzll((aBits[i].bitmap128.m128i_u64[0]));
//			}
//			if(aBits[i].bitmap128.m128i_u64[1]) {
//				return i * 128 + 64 + __builtin_ctzll((aBits[i].bitmap128.m128i_u64[1]));
//			}
//		}

//		for(int i = 0; i < getNumWords(); i++) { //slow
//			uint64_t bits;
//			if((bits = _mm256_extract_epi64(aBits[i].b256, 0))) {
//				return i * 256 + __builtin_ctzll(bits); //almost always this is the only test
//			}
//			if((bits = _mm256_extract_epi64(aBits[i].b256, 1))) {
//				return i * 256 + 64 + __builtin_ctzll(bits);
//			}
//			if((bits = _mm256_extract_epi64(aBits[i].b256, 2))) {
//				return i * 256 + 128 + __builtin_ctzll(bits);
//			}
//			if((bits = _mm256_extract_epi64(aBits[i].b256, 3))) {
//				return i * 256 + 192 + __builtin_ctzll(bits);
//			}
//		}
//		return INT_MAX;

		//for(int i = 0; i < maxWords; i++) {
		for(int i = 0; i < getNumWords(); i++) {
			if(aBits[i].u64[0]) {
				return i * 256 + __builtin_ctzll(aBits[i].u64[0]); //almost always this is the only test
			}
			if(aBits[i].u64[1]) {
				return i * 256 + 64 + __builtin_ctzll(aBits[i].u64[1]);
			}
			if(aBits[i].u64[2]) {
				return i * 256 + 128 + __builtin_ctzll(aBits[i].u64[2]);
			}
			if(aBits[i].u64[3]) {
				return i * 256 + 192 + __builtin_ctzll(aBits[i].u64[3]);
			}
		}
		return INT_MAX;
	}
	inline int copyAlive(const sizedUset *original, sizedUset *target, int target_size, const bm128 &deadClues) const {
		int num_inserted = 0;
//		for(int i = 0; i < maxElements / 128; i++) {
//			for(int j = 0; j < 2; j++) {
//				int base = i * 128 + j * 64;
//				for(uint64_t bits = aBits[i].bitmap128.m128i_u64[j]; bits; bits &= (bits - 1)) {
//					int offset = __builtin_ctzll(bits);
//					sizedUset s = original[base + offset];
//					s.clearBits(deadClues);
//					s.setSize(); //calculate new size for the later reordering
////					if(s.getSize() == 0) {
////						//unhit UA within the dead clues
////						printf("unhit UA within the dead clues identified during the consolidation\n");
////						return false;
////					}
//					target[num_inserted++] = s;
//					if(num_inserted >= target_size || s.getSize() == 0) return num_inserted; //possibly with zero length UA
//				}
//			}
//		}
		//for(int i = 0; i < maxWords; i++) {
		for(int i = 0; i < getNumWords(); i++) {
			for(int j = 0; j < 4; j++) {
				int base = i * 256 + j * 64;
				for(uint64_t bits = aBits[i].u64[j]; bits; bits &= (bits - 1)) {
					int offset = __builtin_ctzll(bits);
					sizedUset s = original[base + offset];
					s.clearBits(deadClues);
					s.setSize(); //calculate new size for the later reordering
					if(s.getSize() == 0) {
						target[0] = s;
						return 1; //a zero length UA
					}
					target[num_inserted++] = s;
					if(num_inserted >= target_size)
						return target_size;
				}
			}
		}
		return num_inserted;
	}
//	void static debug_check_hitting_masks(int const srcRows, const bm128 * const src, bit_masks dest[81]) {
//		for(int i = 0; i < srcRows; i++) {
//			for(int c = 0; c < 81; c++) {
//				bool srcBit = src[i].isBitSet(c);
//				//bool maskBit = dest[c].aBits[i / 256].u64[(i % 256) / 4] & (1UL << (i % 64));
//				bm128 d = dest[c].aBits[i / 256].b128[(i % 256) / 128];
//				bool maskBit = d.isBitSet(i % 128);
//				if(srcBit != maskBit) {
//					printf("debug_check_hitting_masks: row %d cell %d mismatch. Ua has %d\n", i, c, srcBit ? 1 : 0);
//				}
//			}
//		}
//	}
};

/**
 *  @brief A bitmap container for masking and retrieving bits in linear time
 *
 *  @tparam _Key  The maximal boolean elements to hold.
*/
template <int maxElements> class bit_mask {
	static const int maxWords = maxElements / 256;

	bm256 aBits[maxElements / 256];
	int actualWords;

    /**
     *  @brief  gets the actual number of used words in runtime, or
     *  maximal words for smaller instances which is resolved at compile time.
     *  @return actualWords <= return <= maxWords
     */
	inline int getNumWords() const {
		if(maxWords > 1) return actualWords; //should be resolved at compile time
		return maxWords;
	}
public:
	static const int maxSize = maxElements;
	inline bit_mask() = default;
   /**
     *  @brief  Creates a %bit_mask with bits from the given mask cleared by the bits of the second mask.
     *
     *  @param  source_mask Source mask.
     *  @param  clear_mask  Clearing mask.
     */
	inline bit_mask(const bit_mask &source_mask, const bit_mask &clear_mask) : actualWords(source_mask.actualWords) {
		for(int i = 0; i < getNumWords(); i++) {
			aBits[i].b256 = _mm256_castpd_si256(_mm256_andnot_pd(_mm256_castsi256_pd(clear_mask.aBits[i].b256), _mm256_castsi256_pd(source_mask.aBits[i].b256)));
		}
	}
    /**
     *  @brief  Creates a %bit_mask with bits set up to the given size.
     *  Also populates a family of 81 bit_masks by transposing
     *  the given 81-bit list.
     *
     *  @param  usets Source bits to store in clearIndexes.
     *  @param  nUsets Size of the source and respectively actual size of the created bit_mask.
     *  @param  clearIndexes[81] Target bit_masks with transposed bits for later clearing.
     */
	inline bit_mask(const bm128 *usets, int nUsets, bit_mask clearIndexes[81]) : actualWords(min((nUsets + 255) / 256, maxWords)){
		//populate the target clearIndexes family
		//http://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/
		//http://mischasan.wordpress.com/2011/07/24/what-is-sse-good-for-transposing-a-bit-matrix/, mn
		//http://hackers-delight.org.ua/048.htm
		bm128 ss;
		if(nUsets >= maxElements) {
			nUsets = maxElements;
		}
		int nSlices = (nUsets + 15) / 16;
		const bm128 * const s = usets;
		for (int slice = 0; slice < nSlices; slice++) { //process 16 rows slice from the source at once
			//process first 80 bits
			for (int srcCol = 0; srcCol < 10; srcCol++) { //process 8 bits at once, repeat 10 times
				for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source
					ss.bitmap128.m128i_u8[srcSliceRow] = s[slice*16+srcSliceRow].bitmap128.m128i_u8[srcCol];
				}
				ss.transposeSlice(ss); // 16 bits * 8 columns for the target
				for (int destRow = 0; destRow < 8; destRow++) {
					clearIndexes[srcCol * 8 + destRow].aBits[slice / 16].u16[slice % 16] = ss.bitmap128.m128i_u16[destRow];
				}
			}
			//process the 81-st bit
			for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source, only first bit is used
				ss.bitmap128.m128i_u8[srcSliceRow] = s[slice*16+srcSliceRow].bitmap128.m128i_u8[10];
			}
			ss = _mm_slli_epi64(ss.bitmap128.m128i_m128i, 7); // move bit 0 to bit 7
			ss.bitmap128.m128i_u16[0] = _mm_movemask_epi8(ss.bitmap128.m128i_m128i);
			clearIndexes[80].aBits[slice / 16].u16[slice % 16] = ss.bitmap128.m128i_u16[0];
		}
		//populate all valid bits for this instance with 1, and all beyond the actual size with 0
		//actualWords = (nUsets + 255) / 256;
		//if(actualWords > maxWords) actualWords = maxWords;
		__m256i all1 = _mm256_set1_epi64x(-1);
		for(int i = 0; i < maxWords; i++) {
			if(i < actualWords - 1) {
				aBits[i].b256 = all1;
			}
			else if(i == actualWords - 1) {
				//the transitional word between all-ones and all-zeroes
				int j = 256 * actualWords - nUsets; //how many upper bits in this word must be cleared
				union { //in attempt to leave the class data pure __m256i
					__m256i b256;
					__m128i b128[2];
				} tr;
				tr.b256 = all1;
				if(j >= 128) {
					//clear the upper 128 bits
					tr.b128[1] = _mm_setzero_si128();
					j -= 128;
					//clear the higher part of the lower bits
					tr.b128[0] = maskLSB[128 - j].m128i_m128i;
				}
				else {
					//leave the lower 128 bits set
					//clear the part of the upper bits
					tr.b128[1] = maskLSB[128 - j].m128i_m128i;
				}
				aBits[i].b256 = tr.b256;
			}
			else {
				aBits[i].b256 = _mm256_setzero_si256();
			}
		}
	}
    /**
     *  @brief  Get the index of the first bit set or INT_MAX if all are zero.
     *
     *  @return A zero based index up to the size, or INT_MAX for empty bit_mask
     */
	inline int getMinIndex() const {

		for(int i = 0; i < getNumWords(); i++) { //slow
			uint64_t bits;
			if((bits = _mm256_extract_epi64(aBits[i].b256, 0))) {
				return i * 256 + __builtin_ctzll(bits); //almost always this is the only test
			}
			if((bits = _mm256_extract_epi64(aBits[i].b256, 1))) {
				return i * 256 + 64 + __builtin_ctzll(bits);
			}
			if((bits = _mm256_extract_epi64(aBits[i].b256, 2))) {
				return i * 256 + 128 + __builtin_ctzll(bits);
			}
			if((bits = _mm256_extract_epi64(aBits[i].b256, 3))) {
				return i * 256 + 192 + __builtin_ctzll(bits);
			}
		}
		return INT_MAX;

//		for(int i = 0; i < getNumWords(); i++) {
//			if(aBits[i].u64[0]) {
//				return i * 256 + __builtin_ctzll(aBits[i].u64[0]); //almost always this is the only test
//			}
//			if(aBits[i].u64[1]) {
//				return i * 256 + 64 + __builtin_ctzll(aBits[i].u64[1]);
//			}
//			if(aBits[i].u64[2]) {
//				return i * 256 + 128 + __builtin_ctzll(aBits[i].u64[2]);
//			}
//			if(aBits[i].u64[3]) {
//				return i * 256 + 192 + __builtin_ctzll(aBits[i].u64[3]);
//			}
//		}
//		return INT_MAX;
	}
    /**
     *  @brief  Checks whether the given mask covers all or the bits.
     *
     *  @param  clear_mask The mask to be applied.
     *  @return True if the bits in the mask are subset of the bits in the clear_mask
     */
	inline bool isSubsetOf(const bit_mask & clear_mask) const {
		for(int i = 0; i < getNumWords(); i++) {
			if(0 == _mm256_testc_si256(clear_mask.aBits[i].b256, aBits[i].b256))
				return false;
//			if(!bm128(aBits[i].b128[0]).isSubsetOf(clear_mask.aBits[i].b128[0])) return false;
//			if(!bm128(aBits[i].b128[1]).isSubsetOf(clear_mask.aBits[i].b128[1])) return false;
		}
		return true;
	}
	int copyAlive(const sizedUset *original, sizedUset *target, int target_size, const dead_clues_type &deadClues) const {
		int num_inserted = 0;
		for(int i = 0; i < getNumWords(); i++) {
			for(int j = 0; j < 4; j++) {
				int base = i * 256 + j * 64;
				for(uint64_t bits = aBits[i].u64[j]; bits; bits &= (bits - 1)) {
					int offset = __builtin_ctzll(bits);
					sizedUset s = original[base + offset];
					s.clearBits(deadClues);
					s.setSize(); //calculate new size for the later reordering
					if(s.getSize() == 0) {
						target[0] = s;
						//printf("unhit UA within the dead clues identified during the consolidation\n");
						return 1; //a zero length UA
					}
					target[num_inserted++] = s;
					if(num_inserted >= target_size)
						return target_size;
				}
			}
		}
		return num_inserted;
	}
};

struct BitMask768 : public bit_masks<768> {};

#endif //BITMASK768_H_INCLUDED
