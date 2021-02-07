#include "t_128.h"

const t_128 bitSet[128] =
{
	{{0x0000000000000001,0x0}}, {{0x0000000000000002,0x0}}, {{0x0000000000000004,0x0}}, {{0x0000000000000008,0x0}},
	{{0x0000000000000010,0x0}}, {{0x0000000000000020,0x0}}, {{0x0000000000000040,0x0}}, {{0x0000000000000080,0x0}},
	{{0x0000000000000100,0x0}}, {{0x0000000000000200,0x0}}, {{0x0000000000000400,0x0}}, {{0x0000000000000800,0x0}},
	{{0x0000000000001000,0x0}}, {{0x0000000000002000,0x0}}, {{0x0000000000004000,0x0}}, {{0x0000000000008000,0x0}},
	{{0x0000000000010000,0x0}}, {{0x0000000000020000,0x0}}, {{0x0000000000040000,0x0}}, {{0x0000000000080000,0x0}},
	{{0x0000000000100000,0x0}}, {{0x0000000000200000,0x0}}, {{0x0000000000400000,0x0}}, {{0x0000000000800000,0x0}},
	{{0x0000000001000000,0x0}}, {{0x0000000002000000,0x0}}, {{0x0000000004000000,0x0}}, {{0x0000000008000000,0x0}},
	{{0x0000000010000000,0x0}}, {{0x0000000020000000,0x0}}, {{0x0000000040000000,0x0}}, {{0x0000000080000000,0x0}},
	{{0x0000000100000000,0x0}}, {{0x0000000200000000,0x0}}, {{0x0000000400000000,0x0}}, {{0x0000000800000000,0x0}},
	{{0x0000001000000000,0x0}}, {{0x0000002000000000,0x0}}, {{0x0000004000000000,0x0}}, {{0x0000008000000000,0x0}},
	{{0x0000010000000000,0x0}}, {{0x0000020000000000,0x0}}, {{0x0000040000000000,0x0}}, {{0x0000080000000000,0x0}},
	{{0x0000100000000000,0x0}}, {{0x0000200000000000,0x0}}, {{0x0000400000000000,0x0}}, {{0x0000800000000000,0x0}},
	{{0x0001000000000000,0x0}}, {{0x0002000000000000,0x0}}, {{0x0004000000000000,0x0}}, {{0x0008000000000000,0x0}},
	{{0x0010000000000000,0x0}}, {{0x0020000000000000,0x0}}, {{0x0040000000000000,0x0}}, {{0x0080000000000000,0x0}},
	{{0x0100000000000000,0x0}}, {{0x0200000000000000,0x0}}, {{0x0400000000000000,0x0}}, {{0x0800000000000000,0x0}},
	{{0x1000000000000000,0x0}}, {{0x2000000000000000,0x0}}, {{0x4000000000000000,0x0}}, {{0x8000000000000000,0x0}},
	{{0x0,0x0000000000000001}}, {{0x0,0x0000000000000002}}, {{0x0,0x0000000000000004}}, {{0x0,0x0000000000000008}},
	{{0x0,0x0000000000000010}}, {{0x0,0x0000000000000020}}, {{0x0,0x0000000000000040}}, {{0x0,0x0000000000000080}},
	{{0x0,0x0000000000000100}}, {{0x0,0x0000000000000200}}, {{0x0,0x0000000000000400}}, {{0x0,0x0000000000000800}},
	{{0x0,0x0000000000001000}}, {{0x0,0x0000000000002000}}, {{0x0,0x0000000000004000}}, {{0x0,0x0000000000008000}},
	{{0x0,0x0000000000010000}}, {{0x0,0x0000000000020000}}, {{0x0,0x0000000000040000}}, {{0x0,0x0000000000080000}},
	{{0x0,0x0000000000100000}}, {{0x0,0x0000000000200000}}, {{0x0,0x0000000000400000}}, {{0x0,0x0000000000800000}},
	{{0x0,0x0000000001000000}}, {{0x0,0x0000000002000000}}, {{0x0,0x0000000004000000}}, {{0x0,0x0000000008000000}},
	{{0x0,0x0000000010000000}}, {{0x0,0x0000000020000000}}, {{0x0,0x0000000040000000}}, {{0x0,0x0000000080000000}},
	{{0x0,0x0000000100000000}}, {{0x0,0x0000000200000000}}, {{0x0,0x0000000400000000}}, {{0x0,0x0000000800000000}},
	{{0x0,0x0000001000000000}}, {{0x0,0x0000002000000000}}, {{0x0,0x0000004000000000}}, {{0x0,0x0000008000000000}},
	{{0x0,0x0000010000000000}}, {{0x0,0x0000020000000000}}, {{0x0,0x0000040000000000}}, {{0x0,0x0000080000000000}},
	{{0x0,0x0000100000000000}}, {{0x0,0x0000200000000000}}, {{0x0,0x0000400000000000}}, {{0x0,0x0000800000000000}},
	{{0x0,0x0001000000000000}}, {{0x0,0x0002000000000000}}, {{0x0,0x0004000000000000}}, {{0x0,0x0008000000000000}},
	{{0x0,0x0010000000000000}}, {{0x0,0x0020000000000000}}, {{0x0,0x0040000000000000}}, {{0x0,0x0080000000000000}},
	{{0x0,0x0100000000000000}}, {{0x0,0x0200000000000000}}, {{0x0,0x0400000000000000}}, {{0x0,0x0800000000000000}},
	{{0x0,0x1000000000000000}}, {{0x0,0x2000000000000000}}, {{0x0,0x4000000000000000}}, {{0x0,0x8000000000000000}}
};

const t_128 maskLSB[129] =
{
	{{0,0}},
	{{0x00000001,0}}, {{0x00000003,0}}, {{0x00000007,0}}, {{0x0000000F,0}},
	{{0x0000001F,0}}, {{0x0000003F,0}}, {{0x0000007F,0}}, {{0x000000FF,0}},
	{{0x000001FF,0}}, {{0x000003FF,0}}, {{0x000007FF,0}}, {{0x00000FFF,0}},
	{{0x00001FFF,0}}, {{0x00003FFF,0}}, {{0x00007FFF,0}}, {{0x0000FFFF,0}},
	{{0x0001FFFF,0}}, {{0x0003FFFF,0}}, {{0x0007FFFF,0}}, {{0x000FFFFF,0}},
	{{0x001FFFFF,0}}, {{0x003FFFFF,0}}, {{0x007FFFFF,0}}, {{0x00FFFFFF,0}},
	{{0x01FFFFFF,0}}, {{0x03FFFFFF,0}}, {{0x07FFFFFF,0}}, {{0x0FFFFFFF,0}},
	{{0x1FFFFFFF,0}}, {{0x3FFFFFFF,0}}, {{0x7FFFFFFF,0}}, {{0xFFFFFFFF,0}},
	{{0x00000001FFFFFFFF,0}}, {{0x00000003FFFFFFFF,0}}, {{0x00000007FFFFFFFF,0}}, {{0x0000000FFFFFFFFF,0}},
	{{0x0000001FFFFFFFFF,0}}, {{0x0000003FFFFFFFFF,0}}, {{0x0000007FFFFFFFFF,0}}, {{0x000000FFFFFFFFFF,0}},
	{{0x000001FFFFFFFFFF,0}}, {{0x000003FFFFFFFFFF,0}}, {{0x000007FFFFFFFFFF,0}}, {{0x00000FFFFFFFFFFF,0}},
	{{0x00001FFFFFFFFFFF,0}}, {{0x00003FFFFFFFFFFF,0}}, {{0x00007FFFFFFFFFFF,0}}, {{0x0000FFFFFFFFFFFF,0}},
	{{0x0001FFFFFFFFFFFF,0}}, {{0x0003FFFFFFFFFFFF,0}}, {{0x0007FFFFFFFFFFFF,0}}, {{0x000FFFFFFFFFFFFF,0}},
	{{0x001FFFFFFFFFFFFF,0}}, {{0x003FFFFFFFFFFFFF,0}}, {{0x007FFFFFFFFFFFFF,0}}, {{0x00FFFFFFFFFFFFFF,0}},
	{{0x01FFFFFFFFFFFFFF,0}}, {{0x03FFFFFFFFFFFFFF,0}}, {{0x07FFFFFFFFFFFFFF,0}}, {{0x0FFFFFFFFFFFFFFF,0}},
	{{0x1FFFFFFFFFFFFFFF,0}}, {{0x3FFFFFFFFFFFFFFF,0}}, {{0x7FFFFFFFFFFFFFFF,0}}, {{0xFFFFFFFFFFFFFFFF,0}},
	{{0xFFFFFFFFFFFFFFFF,0x00000001}}, {{0xFFFFFFFFFFFFFFFF,0x00000003}}, {{0xFFFFFFFFFFFFFFFF,0x00000007}}, {{0xFFFFFFFFFFFFFFFF,0x0000000F}},
	{{0xFFFFFFFFFFFFFFFF,0x0000001F}}, {{0xFFFFFFFFFFFFFFFF,0x0000003F}}, {{0xFFFFFFFFFFFFFFFF,0x0000007F}}, {{0xFFFFFFFFFFFFFFFF,0x000000FF}},
	{{0xFFFFFFFFFFFFFFFF,0x000001FF}}, {{0xFFFFFFFFFFFFFFFF,0x000003FF}}, {{0xFFFFFFFFFFFFFFFF,0x000007FF}}, {{0xFFFFFFFFFFFFFFFF,0x00000FFF}},
	{{0xFFFFFFFFFFFFFFFF,0x00001FFF}}, {{0xFFFFFFFFFFFFFFFF,0x00003FFF}}, {{0xFFFFFFFFFFFFFFFF,0x00007FFF}}, {{0xFFFFFFFFFFFFFFFF,0x0000FFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x0001FFFF}}, {{0xFFFFFFFFFFFFFFFF,0x0003FFFF}}, {{0xFFFFFFFFFFFFFFFF,0x0007FFFF}}, {{0xFFFFFFFFFFFFFFFF,0x000FFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x001FFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x003FFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x007FFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x00FFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x01FFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x03FFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x07FFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x0FFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFF}},
	{{0xFFFFFFFFFFFFFFFF,0x1FFFFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x3FFFFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFF}}, {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF}},
};

const t_128 maskffff = {{0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF}};

/**
inline unsigned int findRightmost1(const __m128i a) {
	__m128i x;
	x = _mm_cmpeq_epi8 (a, _mm_setzero_si128 ());
	unsigned int i = _mm_movemask_epi8 (x);
	i = ~i;
	unsigned int offs = lsb11bit[i & 0xfff]; //[0..12]
	unsigned int lsb = (unsigned int)((unsigned char*)&a)[offs];
	lsb = lsb11bit[lsb];
	lsb += (offs << 3);
	return lsb;
}

//http://dalkescientific.blogspot.com/2008/06/molecular-fingerprints.html
static inline int popcount_128(__m128i xmm)
{
	const __m128i msk55 = _mm_set1_epi32(0x55555555);
	const __m128i msk33 = _mm_set1_epi32(0x33333333);
	const __m128i msk0F = _mm_set1_epi32(0x0F0F0F0F);
	const __m128i mul01 = _mm_set1_epi32(0x01010101);

	//xmm -= ((xmm >> 1) & 0x55555555);
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

//code inspired by "Exploiting 64-Bit Parallelism" by Ron Gutman (DDJ, September 2000)
//http://www.drdobbs.com/architect/184405995
//inline int popcount_64(long long b)
//{
//	b = (b & 0x5555555555555555LU) + (b >> 1 & 0x5555555555555555LU);
//	b = (b & 0x3333333333333333LU) + (b >> 2 & 0x3333333333333333LU);
//	b = b + (b >> 4) & 0x0F0F0F0F0F0F0F0FLU;
//	b = b + (b >> 8);
//	b = b + (b >> 16);
//	b = b + (b >> 32) & 0x0000007F;
//	return (int) b;
//}

//debug
inline void to_bits(const __m128i b) {
	t_128 t;
	t.m128i_m128i = b;

	int v = t.m128i_u32[2];
	for(int j=0;j<17;j++) {
		printf("%c", (v & 0x10000) ? '1' : j == 10 ? '_' : '.');
		v <<= 1;
	}
	v = t.m128i_u32[1];
	for(int j=0;j<32;j++) {
		printf("%c", (v & 0x80000000) ? '1' : '.');
		v <<= 1;
	}
	v = t.m128i_u32[0];
	for(int j=0;j<32;j++) {
		printf("%c", (v & 0x80000000) ? '1' : '.');
		v <<= 1;
	}
	printf("\n");
}

//Set the given bit [0..80], preserving the bitcount.
__declspec(noinline) int bitset(__m128i &cs, const int b) {
	//Initially clues were
	// 000...000all bits
	//with tendency to go to
	//all bits000...000
	
	//int hi, lo; 
	int bcLSB;
	int rightmost1, rightmost0;
	t_128 t;

	if(b == -1) goto increment;

	//The current clues are like
	// ...MSB... b ...LSB...
	//The goal is to set bit b, minimally increasing the value of the
	//whole clueset. This means
	//a)moving any bit to left must be followed by resseting the right side ones to the rightmost
	//b)moving rightmost bit to right must be followed by minimal increasing in positions of the
	//leftside bits.
	//If there are any LSB, we are moving leftmost of them to
	//position b and then resseting the rest LSB to the rightmost.
	t.m128i_m128i = _mm_and_si128(cs, maskLSB[b+1].m128i_m128i);
	if(0xFFFF == _mm_movemask_epi8 (_mm_cmpeq_epi8 (t.m128i_m128i, _mm_setzero_si128 ()))) goto increment;

	//b is not the less significant bit
	bcLSB = popcount_128(t.m128i_m128i); //perform the slower popcount
	cs = _mm_andnot_si128 (maskLSB[b+1].m128i_m128i, cs); //clear the bits [b..0]
	cs = _mm_or_si128 (cs, bitSet[b].m128i_m128i); //set bit b
	cs = _mm_or_si128 (cs, maskLSB[bcLSB - 1].m128i_m128i); //put the rest cleared at right if any
	// ...same MSB... b 000...000rest LSB
	z0++;
	return b;

increment:
	z1++;
	//Else we are NOT placing the rightmost bit r on b and shifting left
	//the next left to him, because this may rearrange the MSB to hit some
	//other bit in the same constrain, in which case the new place of bit r for minimal
	//clueset is somewhere right to b.
	//WRONG: ...subset of MSB.. b 000...000
	//What we do is just increasing the clueset and rechecking.
	// ...subset of MSB.. ? 000...LSB

	//work with MSB, ignore b
	rightmost1 = findRightmost1(cs);

	//set the bits [rightmost1..0]
	t.m128i_m128i = _mm_or_si128 (cs, maskLSB[rightmost1+1].m128i_m128i);
	//t ~= t
	t.m128i_m128i = _mm_andnot_si128 (t.m128i_m128i, maskffff.m128i_m128i);
	//find the rightmost nonzero in t, which in cs is the rightmost zero leftside of the rightmost1
	rightmost0 = findRightmost1(t.m128i_m128i);

	if(rightmost0 >= 81) { //overflow, finished
		return -1;
	}
	bcLSB = rightmost0 - rightmost1 - 1;
	cs = _mm_andnot_si128 (maskLSB[rightmost0+1].m128i_m128i, cs); //clear the bits [rightmost0..0]
	cs = _mm_or_si128 (cs, bitSet[rightmost0].m128i_m128i); //set the former rightmost0 bit
	cs = _mm_or_si128 (cs, maskLSB[bcLSB].m128i_m128i); //put the rest cleared at right if any
	return rightmost0;
}
***/
