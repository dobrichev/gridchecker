#ifndef T_128_H_INCLUDED

#define T_128_H_INCLUDED

//#include <emmintrin.h>
#include <smmintrin.h>
#include <limits.h>

#ifndef _MSC_VER
	//assume every compiler but MS is C99 compliant and has inttypes
	#include <inttypes.h>
#else
	#include <intrin.h>
   //typedef signed __int8     int8_t;
   //typedef signed __int16    int16_t;
   //typedef signed __int32    int32_t;
   typedef unsigned __int8   uint8_t;
   typedef unsigned __int16  uint16_t;
   typedef unsigned __int32  uint32_t;
   //typedef signed __int64       int64_t;
   typedef unsigned __int64     uint64_t;
#endif

#ifdef   _MSC_VER
	#define x_popcnt64(a) __popcnt64(a)
	#define x_popcnt32(a) __popcnt32(a)
	static inline int __builtin_ctzll(unsigned long long input_num) {
		unsigned long index;
		_BitScanForward64(&index, input_num);
		return index;
	}
	#define __restrict
#else
	#define x_popcnt64(a) __builtin_popcountll(a)
	#define x_popcnt32(a) __builtin_popcount(a)
#endif

typedef union t_128 {
    ////unsigned __int64    m128i_u64[2];
    uint64_t    m128i_u64[2];
    uint8_t     m128i_u8[16];
    uint16_t    m128i_u16[8];
    uint32_t    m128i_u32[4];
	__m128i				m128i_m128i;
    //__int64             m128i_i64[2];
	//__m128d				m128i_m128d;
} t_128;

extern const t_128 bitSet[128];
extern const t_128 maskLSB[129];
extern const t_128 maskffff;
static const unsigned char toPos[] = {
	0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8
};

struct bm128 {
private:
	inline static __m128i andnot(const __m128i l, const __m128i r) {return _mm_andnot_si128(l, r);};
	inline static bool equals(const __m128i l, const __m128i r) {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(l, r));};
public:
	t_128 bitmap128;
	bm128() {};
	bm128(const bm128 &v) : bitmap128(v.bitmap128) {};
	bm128(const __m128i &v) {bitmap128.m128i_m128i = v;};
	bm128(const t_128 &v) {bitmap128.m128i_m128i = v.m128i_m128i;};
	//bm128(const t_128 v) {bitmap128.m128i_m128i = v.m128i_m128i;};
	inline __m128i operator| (const bm128 &r) const {return _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline bool operator== (const bm128& r) const {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
	inline bool operator!= (const bm128& r) const {return 0xFFFF != _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
	inline __m128i operator& (const bm128 &r) const {return _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	//inline __m128i operator& (const __m128i r) const {return _mm_and_si128(bitmap128.m128i_m128i, r);};
	inline void operator&= (const bm128& r) {bitmap128.m128i_m128i = _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const bm128& r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const __m128i r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r);};
	inline void operator^= (const bm128& r) {bitmap128.m128i_m128i = _mm_xor_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator-= (const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, maskLSB[81].m128i_m128i);}; //81-complementary
	inline void operator<<= (const int bits) {bitmap128.m128i_m128i = _mm_slli_epi16(bitmap128.m128i_m128i, bits);};
	inline bool isDisjoint(const bm128& r) const {return 0 != _mm_testz_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}
	//inline bool isDisjoint(const bm128& r) const {return equals(andnot(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i), bitmap128.m128i_m128i);};
	//inline bool slow isDisjoint(const bm128& r) const {return equals(_mm_and_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i), _mm_setzero_si128());};
	inline int mask8() const {return _mm_movemask_epi8(bitmap128.m128i_m128i);}
	inline uint64_t toInt64() const {return _mm_cvtsi128_si64(bitmap128.m128i_m128i);}
	inline uint64_t toInt64_1() const {return _mm_extract_epi64(bitmap128.m128i_m128i, 1);}
	inline int toInt32() const {return _mm_cvtsi128_si32(bitmap128.m128i_m128i);}
	inline int toInt8_15() const {return _mm_extract_epi8(bitmap128.m128i_m128i, 15);}
	inline void setInt8_15(int i) {bitmap128.m128i_m128i = _mm_insert_epi8(bitmap128.m128i_m128i, i, 15);}
	//inline bool isBitSet(const int theBit) const {return equals(*this & bitSet[theBit].m128i_m128i, bitSet[theBit].m128i_m128i);};
	inline bool isBitSet(const int theBit) const {return !_mm_testz_si128(this->bitmap128.m128i_m128i, bitSet[theBit].m128i_m128i);}
	inline void setBit(const int theBit) {*this |= bitSet[theBit].m128i_m128i;};
	inline void clearBit(const int theBit) {bitmap128.m128i_m128i = _mm_andnot_si128(bitSet[theBit].m128i_m128i, bitmap128.m128i_m128i);};
	inline void clearBits(const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	inline void clearBits(const bm128& r, const bm128& r1) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, r1.bitmap128.m128i_m128i);};
	inline void clear() {bitmap128.m128i_m128i = _mm_setzero_si128();};
	inline bool isSubsetOf(const bm128 &s) const {return 1 == _mm_testc_si128(s.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}
	//inline bool clearsAll(const bm128 &s) const {return 1 == _mm_testc_si128(s.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}
	inline bool operator< (const bm128 &rhs) const {
		if(bitmap128.m128i_u64[1] < rhs.bitmap128.m128i_u64[1]) return true;
		if(bitmap128.m128i_u64[1] > rhs.bitmap128.m128i_u64[1]) return false;
		return bitmap128.m128i_u64[0] < rhs.bitmap128.m128i_u64[0];
	}
	inline void operator= (const bm128 &rhs) {bitmap128.m128i_m128i = rhs.bitmap128.m128i_m128i;};
	//inline void operator= (const bm128 rhs) {bitmap128.m128i_m128i = rhs.bitmap128.m128i_m128i;};
	inline void loadUnaligned (const void *p) {bitmap128.m128i_m128i = _mm_loadu_si128((const __m128i *)p);};
	inline void invalidate() {bitmap128.m128i_m128i = maskffff.m128i_m128i;};
	//inline bool isInvalid() const {return equals(bitmap128.m128i_m128i, maskffff.m128i_m128i);};
	inline bool isInvalid() const {return 1 == _mm_test_all_ones(bitmap128.m128i_m128i);};
	inline static bool isZero(const __m128i &r) {return equals(r, _mm_setzero_si128());};
	inline bool isZero() const {return equals(bitmap128.m128i_m128i, _mm_setzero_si128());};
	//inline bool isZero() const {return _mm_testz_si128(bitmap128.m128i_m128i, bitmap128.m128i_m128i);}; //slower on Ivy bridge
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
		return (int)(x_popcnt64(this->toInt64()) + x_popcnt64(this->toInt64_1()));
//		//http://dalkescientific.blogspot.com/2008/06/molecular-fingerprints.html
//		//see also http://bmagic.sourceforge.net/bmsse2opt.html
//		const __m128i msk55 = _mm_set1_epi32(0x55555555);
//		const __m128i msk33 = _mm_set1_epi32(0x33333333);
//		const __m128i msk0F = _mm_set1_epi32(0x0F0F0F0F);
//		const __m128i mul01 = _mm_set1_epi32(0x01010101);
//
//		//xmm -= ((xmm >> 1) & 0x55555555);
//		__m128i xmm = bitmap128.m128i_m128i;
//		__m128i tmp = _mm_and_si128(_mm_srli_epi32(xmm,1), msk55);
//		xmm = _mm_sub_epi32(xmm, tmp);
//		//xmm = (xmm & 0x33333333) + ((xmm >> 2) & 0x33333333);
//		tmp = _mm_and_si128(_mm_srli_epi32(xmm, 2), msk33);
//		xmm = _mm_add_epi32(_mm_and_si128(xmm, msk33),tmp);
//		//xmm = (xmm + (xmm >> 4)) & 0x0F0F0F0F;
//		tmp = _mm_srli_epi32(xmm,4);
//		xmm = _mm_and_si128(_mm_add_epi32(xmm,tmp),msk0F);
//		// .. mix up
//		tmp = _mm_shuffle_epi32(xmm, _MM_SHUFFLE(3,3,1,1));
//		xmm = _mm_add_epi32(tmp,xmm);
//		xmm = _mm_srli_epi64(_mm_mul_epu32(xmm,mul01), 24);
//		return ((unsigned char*)&xmm)[0]+((unsigned char*)&xmm)[8];
	}
    inline unsigned int nonzeroOctets() const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, _mm_setzero_si128()));}
    inline unsigned int diffOctets(const bm128 &rhs) const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, rhs.bitmap128.m128i_m128i));}
	//static int getBit0Indexes(const bm128 *bitmaps, int bitmapsSize, int *positions) {
	//	//find indices of all clear bits in bitmaps[0..bitmapsSize], store them into positions[], and return number of positions
	//	int n = 0;
	//	//for(int k = 0; k < bitmapsSize; k++)
	//	//	for(int i = 0; i < 128; i++)
	//	//		if(isBitSet(i))
	//	//			positions[n++] = 128 * k + i;
	//	static const unsigned char toPos[] = { //TODO: move to the global lookups
	//		0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
	//		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
	//		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8
	//	};
	//	for(int bmNum = 0; bmNum < bitmapsSize; bmNum++) {
	//		//const bm128 &u = bitmaps[bmNum]; //that is we index 1s
	//		bm128 u(maskffff);
	//		u.clearBits(bitmaps[bmNum]); //that is we index 0s
	//		int m = u.nonzeroOctets(); // 16-bit mask of octets having non-zero bits
	//		int add8 = 0; //0 for lower 8 bits, 8 for higher 8 bits
	//		while(m) { //exit if no more octets with bits set
	//			if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
	//				m >>= 8;
	//				add8 = 8;
	//			}
	//			int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
	//			int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
	//			int octetValue = u.bitmap128.m128i_u8[octetIndex];
	//			do {
	//				int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
	//				int bitIndex = (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
	//				positions[n++] = 128 * bmNum + bitIndex; //store
	//				octetValue ^= octetLSB; //clear the processed bit from the temporay copy
	//			} while(octetValue); //loop until all bits within this octed are processed
	//			m ^= octetIndexLSB; //clear the octet processed
	//		}
	//	}
	//	return n;
	//}

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

//		if(isZero()) return -1; //on Ivy bridge all variations of the code below are slower than the code above
//		register uint64_t x = toInt64();
//		if(x)
//			return __builtin_ctzll(x);
//		return 64 + __builtin_ctzll(toInt64_1());

		return -1;
	}
	inline int getPositions(unsigned char *positions) const {
//		int n = 0;
//		//for(int i = 0; i < 81; i++)
//		//	if(isBitSet(i))
//		//		positions[n++] = (unsigned char)i;
//		int m = nonzeroOctets(); // 16-bit mask of octets having non-zero bits
//		int add8 = 0; //0 for lower 8 bits, 8 for higher 8 bits
//		while(m) { //exit if no more octets with bits set
//			if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
//				m >>= 8;
//				add8 = 8;
//			}
//			int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
//			int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
//			int octetValue = bitmap128.m128i_u8[octetIndex];
//			do {
//				int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
//				int bitIndex = (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
//				positions[n++] = bitIndex; //store
//				octetValue ^= octetLSB; //clear the processed bit from the temporay copy
//			} while(octetValue); //loop until all bits within this octed are processed
//			m ^= octetIndexLSB; //clear the octet processed
//		}
//		return n;

//		int n = 0;
//		for(int j = 0; j < 2; j++) {
//			for(uint64_t bits = bitmap128.m128i_u64[j]; bits; bits &= (bits - 1)) {
//				int offset = __builtin_ctzll(bits);
//				positions[n++] = j * 64 + offset;
//			}
//		}
//		return n;

		int n = 0;
		for(uint64_t bits = toInt64(); bits; bits &= (bits - 1)) {
			int offset = __builtin_ctzll(bits);
			positions[n++] = offset;
		}
		//for(uint64_t bits = toInt64_1(); bits; bits &= (bits - 1)) {
		for(uint64_t bits = bitmap128.m128i_u64[1]; bits; bits &= (bits - 1)) {
			int offset = __builtin_ctzll(bits);
			positions[n++] = 64 + offset;
		}
		return n;

//		int n = 0;
//		uint64_t offset;
//		for(uint64_t bits = toInt64(); bits; bits &= ~(((uint64_t)1) << offset)) { //slower
//			offset = __builtin_ctzll(bits);
//			positions[n++] = offset;
//		}
//		for(uint64_t bits = toInt64_1(); bits; bits &= ~(((uint64_t)1) << offset)) {
//			offset = __builtin_ctzll(bits);
//			positions[n++] = 64 + offset;
//		}
//		return n;

//		//_bittestandreset64(bits, offset)
//		int n = 0;
//		uint64_t offset;
//		for(uint64_t bits = toInt64(); bits;) {
//			offset = __builtin_ctzll(bits);
//			positions[n++] = offset;
//			asm("btr %1,%0" : "+r"(bits) : "r"(offset)); //slow
//		}
//		for(uint64_t bits = toInt64_1(); bits;) {
//			offset = __builtin_ctzll(bits);
//			positions[n++] = 64 + offset;
//			asm("btr %1,%0" : "+r"(bits) : "r"(offset));
//		}
//		return n;
	}
	inline void transposeSlice(const bm128 &src) //http://mischasan.wordpress.com/2011/07/24/what-is-sse-good-for-transposing-a-bit-matrix/
	{
		bm128 x(src);
		for (int i = 0; i < 8; i++) {
			bitmap128.m128i_u16[7-i] = _mm_movemask_epi8(x.bitmap128.m128i_m128i);
			x = _mm_slli_epi64(x.bitmap128.m128i_m128i, 1);
		}
	}
//	void static transpose(int const srcRows, const bm128 * const src, bm128 *dest) {
//		//union {char b[16]; __m128i m; } s;
//		bm128 ss;
//		int srcSize128 = srcRows / 128;
//		for (int slice = 0; slice < srcRows; slice += 16) { //process 16 rows from the source simultaneously
//			//process first 80 bits
//			for (int srcCol = 0; srcCol < 10; srcCol++) { //process 8 bits per "column" simultaneously
//				for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source
//					//s.b[srcSliceRow] = src[slice+srcSliceRow].bitmap128.m128i_u8[col];
//					ss.bitmap128.m128i_u8[srcSliceRow] = src[slice+srcSliceRow].bitmap128.m128i_u8[srcCol];
//				}
//				//s.m = transposeSlice(s.m); // 16 bits * 8 columns for the target
//				ss.transposeSlice(ss); // 16 bits * 8 columns for the target
//				for (int destRow = 0; destRow < 8; destRow++) {
//					//Out[srcCol][slice+srcSliceRow] = s.b[srcSliceRow];
//					dest[((srcCol * 8) + destRow) * srcSize128 + slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[destRow];
//				}
//			}
//			//process 81-th bit
//			for (int srcSliceRow = 0; srcSliceRow < 16; srcSliceRow++) { //fetch 8 bits * 16 rows from source, only first bit is used
//				ss.bitmap128.m128i_u8[srcSliceRow] = src[slice+srcSliceRow].bitmap128.m128i_u8[10];
//			}
//			ss = _mm_slli_epi64(ss.bitmap128.m128i_m128i, 7); // move bit 0 to bit 7
//			ss.bitmap128.m128i_u16[0] = _mm_movemask_epi8(ss.bitmap128.m128i_m128i);
//			dest[80 * srcSize128 + slice / 8].bitmap128.m128i_u16[slice % 8] = ss.bitmap128.m128i_u16[0];
//		}
//	}
	inline static bm128 getFFFF() {
		__m128i x = _mm_undefined_si128();
		//__m128i x = x;
		return _mm_cmpeq_epi8(x, x);
	}
};

#endif // T_128_H_INCLUDED
