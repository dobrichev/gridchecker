#ifndef T_128_H_INCLUDED

#define T_128_H_INCLUDED

#include <smmintrin.h>
//#include <emmintrin.h>
#ifndef _MSC_VER
	//assume every compiler but MS is C99 compliant and has inttypes
	#include <inttypes.h>
#else
   //typedef signed __int8     int8_t;
   //typedef signed __int16    int16_t;
   //typedef signed __int32    int32_t;
   typedef unsigned __int8   uint8_t;
   //typedef unsigned __int16  uint16_t;
   typedef unsigned __int32  uint32_t;
   //typedef signed __int64       int64_t;
   typedef unsigned __int64     uint64_t;
#endif
namespace skfr {

typedef union t_128 {
    uint64_t    m128i_u64[2];
    uint8_t     m128i_u8[16];
    //unsigned __int16    m128i_u16[8];
    uint32_t    m128i_u32[4];
	__m128i				m128i_m128i;
    //__int64             m128i_i64[2];
	//__m128d				m128i_m128d;
} t_128;

extern const t_128 bitSet[128];
extern const t_128 maskLSB[129];
extern const t_128 maskffff;
extern const t_128 false128;
extern const t_128 true128;

int popcount_128(__m128i xmm);

struct bm128 {
private:
//	inline static __m128i andnot(const __m128i l, const __m128i r) {return _mm_andnot_si128(l, r);};
	inline static bool equals(const __m128i l, const __m128i r) {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(l, r));};
//	inline __m128i operator& (const __m128i r) const {return _mm_and_si128(bitmap128.m128i_m128i, r);};
public:
	t_128 bitmap128;
	bm128() {};
	bm128(const bm128 &v) : bitmap128(v.bitmap128) {};
	bm128(const __m128i &v) {bitmap128.m128i_m128i = v;};
	bm128(const t_128 &v) {bitmap128.m128i_m128i = v.m128i_m128i;};
//	bm128(const t_128 v) {bitmap128.m128i_m128i = v.m128i_m128i;};
//	inline __m128i operator| (const bm128 &r) const {return _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline bool operator== (const bm128& r) const {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
//	inline bool operator!= (const bm128& r) const {return 0xFFFF != _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
//	inline __m128i operator& (const bm128 &r) const {return _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator&= (const bm128& r) {bitmap128.m128i_m128i = _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const bm128& r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const __m128i r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r);};
	inline void operator^= (const bm128& r) {bitmap128.m128i_m128i = _mm_xor_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	//inline void operator-= (const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, maskLSB[81].m128i_m128i);}; //complementary
	inline void operator-= (const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}; //andnot
//	inline void operator<<= (const int bits) {bitmap128.m128i_m128i = _mm_slli_epi16(bitmap128.m128i_m128i, bits);};
//	inline bool isDisjoint(const bm128& r) const {return equals(_mm_and_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i), _mm_setzero_si128());};
//	inline int mask8() const {return _mm_movemask_epi8(bitmap128.m128i_m128i);};
//	inline int toInt32() const {return _mm_cvtsi128_si32(bitmap128.m128i_m128i);};
	//inline bool isBitSet(const int theBit) const {return equals(*this & bitSet[theBit].m128i_m128i, bitSet[theBit].m128i_m128i);}; //SSE2
	inline bool isBitSet(const int theBit) const {return !_mm_testz_si128(this->bitmap128.m128i_m128i, bitSet[theBit].m128i_m128i);} //SSE4.1, faster than SSE2
	inline void setBit(const int theBit) {*this |= bitSet[theBit].m128i_m128i;};
	inline void clearBit(const int theBit) {bitmap128.m128i_m128i = _mm_andnot_si128(bitSet[theBit].m128i_m128i, bitmap128.m128i_m128i);};
//	inline void clearBits(const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	inline void clear() {bitmap128.m128i_m128i = _mm_setzero_si128();};
	inline bool isSubsetOf(const bm128 &s) const {return equals(s.bitmap128.m128i_m128i, _mm_or_si128(bitmap128.m128i_m128i, s.bitmap128.m128i_m128i));} //SSE2
	//inline bool isSubsetOf(const bm128 &s) const {return 0 != _mm_testc_si128(s.bitmap128.m128i_m128i, bitmap128.m128i_m128i);} //SSE4.1, slower than SSE2
//	inline bool operator< (const bm128 &rhs) const {
//		if(bitmap128.m128i_u64[1] < rhs.bitmap128.m128i_u64[1]) return true;
//		if(bitmap128.m128i_u64[1] > rhs.bitmap128.m128i_u64[1]) return false;
//		return bitmap128.m128i_u64[0] < rhs.bitmap128.m128i_u64[0];
//	}
	inline void operator= (const bm128 &rhs) {bitmap128.m128i_m128i = rhs.bitmap128.m128i_m128i;};
//	inline void invalidate() {bitmap128.m128i_m128i = maskffff.m128i_m128i;};
//	inline bool isInvalid() const {return equals(bitmap128.m128i_m128i, maskffff.m128i_m128i);};
//	inline static bool isZero(const __m128i &r) {return equals(r, _mm_setzero_si128());};
	inline bool isZero() const {return equals(bitmap128.m128i_m128i, _mm_setzero_si128());}; //SSE2
	//inline bool isZero() const {return _mm_testz_si128(this->bitmap128.m128i_m128i, this->bitmap128.m128i_m128i);}; //SSE4.1, slower than SSE2

	inline void alterOddEven(const bm128& w) {
		//*this = ((w & true128) << 1) | ((w & false128) >> 1); The bits lost in the shifting process are always 0 after applying the mask
		bitmap128.m128i_m128i = _mm_or_si128(
			_mm_slli_epi16(_mm_and_si128(w.bitmap128.m128i_m128i, true128.m128i_m128i), 1),
			_mm_srli_epi16(_mm_and_si128(w.bitmap128.m128i_m128i, false128.m128i_m128i), 1)
			);
	};

	inline int nonzeroOctets() const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, _mm_setzero_si128()));}

//	inline static bm128* allocate(const int size) {return (bm128*)_mm_malloc(size * sizeof(bm128), 16);};
//	inline static void deallocate(void *ptr) {_mm_free(ptr);};
//	int toPseudoPuzzle(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, 0 : digits[i]; return n;}
//	int toPseudoPuzzleString(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, '.' : digits[i] + '0'; return n;}
//	int toPuzzleString(const char* digits, char* r) const {int n = 0; for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? n++, digits[i] + '0' : '.'; return n;}
//	void toMask81(char* r) const {for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? '1' : '.';}
};

} //namespace skfr

#endif // T_128_H_INCLUDED

