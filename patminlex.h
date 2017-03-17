#include <stdio.h>
#include <set>
#include <cstring>

struct bestTripletPermutation {
	int bestResult; //three bit best allowed composition for the givens within a triplet
	int resultMask; //6-bits mask of permutation indexes with 1 for the allowed permutations and zero otherwise
	int resultNumBits; //number of the possible permutations in the result mask
};

class patminlex {
	struct gridPattern {
		int rows[9];
		int digits[9][9];
	};

	//CAND_LIST_SIZE worst case is 15552 = 2 (transpose) * 6 (band permutations) * 6*6*6 (rows in a band perm) * 6 (stack perm)
	#define CAND_LIST_SIZE 15552

	struct candidate {
		char isTransposed;		//0=original source, 1=transposed source
		char mapRowsForward[9]; //source row N goes to target row mapRowsForward[N]
		char mapRowsBackward[9]; //target row M comes from source row mapRowsBackward[M]
		char stacksPerm;
		unsigned char colsPermMask[3];	//bitmask of size 6 with still allowed permutations that don't affect the upper part of the result

		static const int perm[6][3];	//all 6 possible mappings for 3 values. Used for stacks and columns within a stack reordering by a given permutation index[0..5]
		static const candidate defaultCandidate;

		void init(int transpose, int topRow) {
			*this = defaultCandidate;
			isTransposed = transpose;
			mapRowsForward[topRow] = 0;
			mapRowsBackward[0] = topRow;
		}
		void expandStacks(const gridPattern * const pair, int topKey, candidate *results, int &nResults);
	};
	struct mapper {
		//the map is composed for transformation of the canonicalized sub-grid to the original one, so that
		//originalGrid[i] = label[canonicalGrid[cell[i]]]
		char cell[81];
		char label[10];
		inline bool operator< (const mapper& rhs) const {
			return std::memcmp(this, &rhs, sizeof(mapper)) < 0;
		}
	};

	static const int minCanNineBits[512]; //a precomputed minlexed recomposition of the bit triplets for any 9-bits input

	class mappers : public std::set<mapper> {};
	mappers theMaps;

	int fromString(const char *txt, gridPattern& normal, gridPattern& transposed);
	static int bestTopRowScore(gridPattern &p);
public:
	enum action {
		findMinLex, //return minlex in result, default
		findMinPattern, //return minlex pattern in result
		findMinPatternLE, //return minlex pattern in result only if it is less or equal to data, return resLT or resEQ or resGT in res.
	};
	enum res {
		resLT,
		resEQ,
		resGT
	};
	patminlex(const char *source, char *result, action requestedAction = action::findMinLex, res* res = NULL, const void* const data = NULL); //canonicalize & collect maps
	void map(const char* src, char* results) const; //map back in all possible ways
	int size() const; //the number of automorphs = the number of ways an input can be mapped to the same output and vice versa
};
