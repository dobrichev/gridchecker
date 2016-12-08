#include "uset.h"

#include <stdio.h>
#include <fstream>
#include <set>
#include <vector>
#include "tables.h"
#include "mm_allocator.h"
#include "t_128.h"
#include "solver.h"
#include "options.h"

using namespace std;

ratingByDisjoints::ratingByDisjoints() {
	for(int i = 0; i < 24; i++) {
		numDJ[i] = 0;
	}
	ratePlus = 0.0;
}
bool ratingByDisjoints::operator< (const ratingByDisjoints &r) const {
	for(int i = 0; i < 24; i++) {
		if(numDJ[i] < r.numDJ[i]) return true;
		if(numDJ[i] > r.numDJ[i]) return false;
	}
	if(ratePlus < r.ratePlus) return true;
	//if(ratePlus > r.ratePlus) return false;
	return false; //equal
}
void ratingByDisjoints::setRate(const int size, const int rate) {numDJ[24 - size] = rate;}
void ratingByDisjoints::setRate(const double rate) {ratePlus = rate;}

sizedUset::sizedUset() {};
sizedUset::sizedUset(const bm128 &bm) : bm128(bm) {setSize();};
sizedUset::sizedUset(const bm128 &bm, const int size) : bm128(bm) {bitmap128.m128i_u32[3] = (uint32_t)size;};
sizedUset::sizedUset(const sizedUset &bm) : bm128(bm) {};
int sizedUset::getSize() const {
	return bitmap128.m128i_u32[3];
}
void sizedUset::setSize() {
	bitmap128.m128i_u32[3] = 0; //don't count the previous size itself
	bitmap128.m128i_u32[3] = (uint32_t)popcount_128();
}
void sizedUset::setSize(int newSize) {
	bitmap128.m128i_u32[3] = (uint32_t)newSize;
}
//int sizedUset::decreaseSize() {
//	return --bitmap128.m128i_u8[15];
//}
int sizedUset::join(const sizedUset &rhs) {
	bm128 u(bitmap128);
	u &= maskLSB[81];
	if(u.isDisjoint(rhs.bitmap128)) {
		int size = getSize() + rhs.getSize();
		*this |= rhs.bitmap128;
		setSize(size);
		return size;
	}
	return 0;
}
//bool sizedUset::operator < (const sizedUset &rhs) const {
//	//int lc = popcount_128();
//	//int rc = rhs.popcount_128();
//	int lc = getSize();
//	int rc = rhs.getSize();
//	if(lc < rc) return true;
//	if(lc > rc) return false;
//	return (bm128::operator <(rhs));
//}
//void sizedUset::operator=(const bm128 &rhs) {
//	bitmap128 = rhs.bitmap128;
//	setSize();
//}
void sizedUset::operator=(const sizedUset &rhs) {bitmap128 = rhs.bitmap128;}

uset::uset() : nbits(0) {};
uset::uset(const bm128 &bm) : bm128(bm), nbits(0) {};
bool uset::operator < (const uset &rhs) const {
	if(positions[0] > rhs.positions[0]) return true;
	if(positions[0] < rhs.positions[0]) return false;
	if(nbits < rhs.nbits) return true;
	if(nbits > rhs.nbits) return false;
	return (bm128::operator <(rhs));
	//return memcmp(positions, rhs.positions, nbits <= rhs.nbits ? nbits : rhs.nbits) > 0;
}
void uset::positionsByBitmap() {
	//int n = 0;
	////for(int i = 0; i < 81; i++)
	////	if(isBitSet(i))
	////		positions[n++] = (unsigned char)i;
	//int m = nonzeroOctets(); // 16-bit mask of octets having non-zero bits
	//int add8 = 0; //0 for lower 8 bits, 8 for higher 8 bits
	//while(m) { //exit if no more octets with bits set
	//	if((m & 0xFF) == 0) { //lower 8 bits of the mask (== lower 64 bits of the field) are zero, switch to higher bits
	//		m >>= 8;
	//		add8 = 8;
	//	}
	//	int octetIndexLSB = m & -m; //the rightmost octet having nonzero bit
	//	int octetIndex = toPos[octetIndexLSB] + add8 - 1; //zero based index of this octet within the field
	//	int octetValue = bitmap128.m128i_u8[octetIndex];
	//	do {
	//		int octetLSB = octetValue & -octetValue; //the rightmost bit set within the value
	//		int bitIndex = (octetIndex * 8) + (toPos[octetLSB] - 1); //convert to zero based index within the fields
	//		positions[n++] = bitIndex; //store
	//		octetValue ^= octetLSB; //clear the processed bit from the temporay copy
	//	} while(octetValue); //loop until all bits within this octed are processed
	//	m ^= octetIndexLSB; //clear the octet processed
	//}
	//nbits = n;
	nbits = getPositions(positions);
}
void uset::bitmapByPositions() {
	//bitmap128.m128i_m128i = _mm_setzero_si128 ();
	clear();
	for(int i = 0; i < nbits; i++)
		setBit(positions[i]);
}
void uset::bitmapByIntegerPos(const int *pos, const int size) {
	clear();
	nbits = size;
	for(int i = 0; i < nbits; i++)
		setBit(pos[i]);
}
void uset::toString(char *r) const {
	*r++ = '{';
	for(int i = 0; i < nbits; i++) {
		*r++ = (char)('1' + positions[i] / 9);
		*r++ = (char)('1' + positions[i] % 9);
		*r++ = ',';
	}
	*r = 0;
	*(r - 1) = '}';
}
void uset::toString1(char *r) const {
	*r++ = '(';
	for(int i = 0; i < nbits - 1; i++) {
		r += sprintf(r, "%2.2u,", (unsigned int)positions[i]);
	}
	sprintf(r, "%2.2u)", (unsigned int)positions[nbits - 1]);
}
void uset::fromString(const char* s) {
	const char *p;
	int x, y;

	clear();
	nbits = 0;
	if(*s != '{') return;
	for(p = s + 1; *p <= '9' && *p > '0'; p+=3) {
		x = *p - '1';
		y = p[1] - '1';
		if(y < 0 || y > 8) goto error;
		x = 9 * x + y;
		setBit(x);
		positions[nbits++] = (unsigned char)x;
	}
	return;
error:
	nbits = 0;
	return;
}
void uset::fromPuzzle(const char* s) {
	clear();
	nbits = 0;
	for(int i = 0; i < 81; i++) {
		if(s[i]) {
			setBit(i);
			positions[nbits++] = i;
		}
	}
}
void uset::operator=(const bm128 &rhs) {bitmap128 = rhs.bitmap128;};

int lightweightUsetList::find2of3() const {
	int n = 0;
	for(lightweightUsetList::const_iterator s1 = begin(); s1 != end(); s1++) {
		for(lightweightUsetList::const_iterator s2 = upper_bound(*s1); s2 != end(); s2++) {
			bm128 s3 = *s1;
			s3 ^= *s2;
			if(count(s3)) {
				n++;
			}
		}
	}
	return n;
}

bool compareUsetBySize::operator()(const uset& _Left, const uset& _Right) const {
	if(_Left.nbits < _Right.nbits) return true;
	if(_Left.nbits > _Right.nbits) return false;
	return memcmp(_Left.positions, _Right.positions, _Left.nbits <= _Right.nbits ? _Left.nbits : _Right.nbits) < 0;
}

//bool compareLightweightUsetBySize::operator()(const bm128& _Left, const bm128& _Right) const {
//	int lc = _Left.popcount_128();
//	int rc = _Left.popcount_128();
//	if(lc < rc) return true;
//	if(lc > rc) return false;
//	return _Left < _Right;
//}

void usetListBySize::insertNoSuperset(const uset &us) {
	if(count(us))
		return; //do nothing with duplicates
	for(const_iterator s = begin(); s != upper_bound(us); s++) { //search within first (smaller) elements
		if(s->isSubsetOf(us))
			return;
	}
	insert(us);
}
void usetListBySize::removeSupersets() {
	for(const_iterator us = begin(); us != end(); us++) { //scan entire set
		for(iterator s = lower_bound(*us); s != end();) { //search within next (larger) elements
			if((us->nbits < s->nbits) && us->isSubsetOf(*s)) {
				//s = erase(s);
                iterator ss = s;
                s++;
				erase(ss);
            }
			else
				s++;
		}
	}
}
//void usetListBySize::unused_setNumDisjionts () {
//	for(iterator us = begin(); us != end(); us++) {
//		us->numDisjoints = 0;
//	}
//	for(iterator us = begin(); us != end(); us++) {
//		for(iterator s = lower_bound(*us); s != end(); s++) {
//			if(us->isDisjoint(*s)) {
//				us->numDisjoints++;
//				s->numDisjoints++;
//			}
//		}
//	}
//}
void usetListBySize::getRate(const int maxSize, const bm128 &mask, ratingByDisjoints &rating) const {
	if(maxSize) { //for maxSize == 0 rate only by getNumDisjoints()
		//max real disjoint sets are 16, but forcing non-givens complicates the things
		//2 UA with common clue are disjoint after forcing the common clue to non-given
		lightweightUsetList dj[20];
		for(const_iterator ua = begin(); ua != end(); ua++) {
			if(ua->nbits == 1)
				continue;
			if(ua->nbits > maxSize)
				break;
			if(ua->isDisjoint(mask))
				dj[1].insert(*ua);
		}
		int s = 0, s1;
		do {
			s++;
			s1 = s + 1;
			for(lightweightUsetList::const_iterator u1 = dj[1].begin(); u1 != dj[1].end(); u1++) {
				for(lightweightUsetList::const_iterator us = dj[s].begin(); us != dj[s].end(); us++) {
					if(us->isDisjoint(*u1)) {
						bm128 t = (*us) | (*u1);
						dj[s1].insert(t);
					}
				}
			}
			if(opt.verbose) {
				printf("%d=%d,", s, static_cast <int>(dj[s].size()));
			}
			rating.setRate(s, static_cast <int>(dj[s].size()));
			if(s != 1)
				dj[s].clear();
		} while(!dj[s1].empty());
		dj[1].clear();
	}
	double addrate = getNumDisjoints(mask);
	rating.setRate(addrate);
	if(opt.verbose) {
		printf(" (%f)\n", addrate);
	}
}
void usetListBySize::setDistributions() {
	for(int i = 0; i < 88; i++) {
		distributionBySize[i] = 0;
	}
	for(const_iterator ua = begin(); ua != end(); ua++) {
		distributionBySize[ua->nbits]++;
	}
}
double usetListBySize::getNumDisjoints(const bm128 &mask) const {
	double i = 0;
	for(const_iterator ua = begin(); ua != end(); ua++) {
		if(ua->isDisjoint(mask))
			i += (1.0 / (ua->nbits * ua->nbits));
	}
	return i;
}
int usetListBySize::ReadFromFile(char const *filename) {
	ifstream file(filename);
	if (! file)
		return -1;
	char buf[1000];
	iterator hint = begin();
	while(file.getline(buf, sizeof(buf))) {
		uset u;
		u.fromString(buf);
		////g.updateDigitMask(u);
		hint = insert(hint, u); //expecting the UA are sorted, give a hint to insert after the last UA
	}
	if(opt.verbose) {
		printf("%d UA sets loaded.\n", (int)size());
	}
	return 0;
}

cliqueMember::cliqueMember() : numDisjoints(0) {};
cliqueMember::cliqueMember(const bm128 &bm) : uset(bm), numDisjoints(0) {};
void cliqueMember::setNumDisjoints(const bm128 &bm, const usetListBySize &ul) {
	numDisjoints = 0;
	for(usetListBySize::const_iterator us = ul.begin(); us != ul.end(); us++) {
		if(bm.isDisjoint(*us)) {
			numDisjoints++;
		}
	}
}

void cliqueMember::rateIt(const usetListBySize &ul, const bm128 &bm) {
	//int maxSize = 8;
	int maxSize = 11;
	//patch for MC & similar grids which require 100GB
	//Pt(20) is slowed down from 487 to 541 seconds
	if(ul.distributionBySize[6] > 54 || ul.distributionBySize[3] || ul.distributionBySize[2] || ul.distributionBySize[1]) {
		maxSize = 0; //quietly rate only by number of DJ regardless of cliques
	}
	ul.getRate(maxSize, bm, rating);
}

void cliqueMember::rateCells(const usetListBySize &ul) {
	ratingByDisjoints cr[81];
	//pass 1: clear cells, one by one, and get rating
	int maxSize = 8;
	//patch for MC & similar grids which require 100GB
	//Pt(20) is slowed down from 487 to 541 seconds
	if(ul.distributionBySize[6] > 54 || ul.distributionBySize[3] || ul.distributionBySize[2] || ul.distributionBySize[1]) {
		maxSize = 0; //quietly rate only by number of DJ regardless of cliques
	}
	for(int i = 0; i < nbits; i++) {
		bm128 bm;
		bm = accumulatedBM;
		bm.clearBit(positions[i]);
		ul.getRate(maxSize, bm, cr[i]);
		mappedTo[i] = -1; //invalidate desired position
	}
	//pass 2: order cells by rating, storing the desired positions in mappedTo
	//cells with high rating are placed at left
	int nUnorderedMembers = nbits;
	while(nUnorderedMembers > 1) {
		int bestIndex = -1;
		for(int p = 0; p < nbits; p++) {
			if(mappedTo[p] == -1) {
				if(bestIndex == -1)
					bestIndex = p;
				if((cr[bestIndex] < cr[p])) {
				//if(!(cr[bestIndex] < cr[p])) {
					bestIndex = p; //best = lower rating = less DJ for the rest of clues in this UA = leftmost
				}
			}
		}
		nUnorderedMembers--;
		mappedTo[bestIndex] = (char)nUnorderedMembers;
	}
	//set the last position w/o checkings
	for(int p = 0; p < nbits; p++) {
		if(mappedTo[p] == -1) {
			mappedTo[p] = 0;
			break;
		}
	}
}

clique::clique() : size(0), nPartitions(0) {fixedClues.clear(); fixedClues.nbits = 0; fixedNonClues.clear(); fixedNonClues.nbits = 0;};
void clique::insert(cliqueMember &m) {ua[size++] = m;}
void clique::clear() {size = 0;}
void clique::sortMembers(const usetListBySize &ul) {
	if(opt.verbose) {
		printf("\nReordering the UA sets and cells according to the selected clique.\n");
	}
	//first sort the members by size, ascending
	sortBySize();
	//group members of equal size in partitions
	createPartitions();
	//the order is
	//<rest cells><partition n with max UA sizes>...<partition 0 with min UA sizes>
	//sort the members within partitions
	for(int finalPartition = nPartitions - 1; finalPartition >= 0 && partitions[finalPartition].memberSizes > 1; finalPartition--) { //start from largest members
		int nUnorderedMembers = partitions[finalPartition].numMembers;
		if(nUnorderedMembers > 1) { //there is nothing to do with a single member
			if(opt.verbose) {
				printf("Reordering %d UA of size %d within partition %d.\n", partitions[finalPartition].numMembers, partitions[finalPartition].memberSizes, finalPartition);
			}
			for(int partitionMember = 0; partitionMember < nUnorderedMembers; partitionMember++) {
				bm128 mask = ua[partitions[finalPartition].firstCliqueMember + partitionMember].bitmap128;
				ua[partitions[finalPartition].firstCliqueMember + partitionMember].rateIt(ul, mask);
			}
			while(nUnorderedMembers > 1) { //there is nothing to do with a single member
				int bestIndex = 0;
				for(int partitionMember = 0; partitionMember < nUnorderedMembers; partitionMember++) {
					if(!(ua[partitions[finalPartition].firstCliqueMember + bestIndex].rating < ua[partitions[finalPartition].firstCliqueMember + partitionMember].rating)) {
					//if((ua[partitions[finalPartition].firstCliqueMember + bestIndex].rating < ua[partitions[finalPartition].firstCliqueMember + partitionMember].rating)) {
						bestIndex = partitionMember; //best = lower rating = more DJ for the rest of patrition = leftmost
					}
				}
				//place the best member at leftmost
				nUnorderedMembers--;
				if(bestIndex != nUnorderedMembers) {
					swap(partitions[finalPartition].firstCliqueMember + bestIndex, partitions[finalPartition].firstCliqueMember + nUnorderedMembers);
				}
			}
		}
	}
	ua[0].accumulatedBM = ua[0];
	for(int m = 1; m <= size; m++) { //clique member
		ua[m].accumulatedBM = ua[m - 1].accumulatedBM | ua[m];
	}
	//order the positions within each member
	if(opt.verbose) {
		printf("\nReordering the cells within UA.\n");
	}
	for(int m = 0; m < size; m++) { //clique member
		if(opt.verbose) {
			printf("Reordering %d cells within member %d.\n", ua[m].nbits, m);
		}
		ua[m].rateCells(ul);
	}
	//remap intramember positions to desired grid positions
	int offset = 0;
	for(int partition = 0; partition < nPartitions; partition++) {
		for(int m = 0; m < partitions[partition].numMembers; m++) {
			for(int pos = 0; pos < partitions[partition].memberSizes; pos++) {
				ua[partitions[partition].firstCliqueMember + m].mappedTo[pos] += (char)offset;
			}
			offset += partitions[partition].memberSizes;
		}
	}
	//remap rest of the cells if any
	if(offset < 81 - fixedClues.nbits - fixedNonClues.nbits) {
		if(opt.verbose) {
			printf("\nReordering the rest %d cells.\n", ua[size].nbits);
		}
		for(int pos = 0; pos < ua[size].nbits; pos++) {
			ua[size].mappedTo[pos] = (char)(offset + pos);
		}
	}
	//fixed clues and non-clues will be ramapped later regardless of mappedTo property
}
void clique::toString(char *r) const {
	*r = 0;
	for(int i = 0; i < size; i++) {
		char buf[300];
		ua[i].toString(buf);
		sprintf(r, "%s%s\n", r, buf);
	}
}
void clique::swap(const int m1, const int m2) {
	cliqueMember t = ua[m1];
	ua[m1] = ua[m2];
	ua[m2] = t;
}
void clique::sortBySize() {
	bool again = true;
	while(again) {
		again = false;
		for(int i = 0; i < size - 1; i++) {
			if(ua[i].nbits > ua[i + 1].nbits) {
			//if(ua[i].nbits < ua[i + 1].nbits) {
				swap(i, i + 1);
				again = true;
			}
		}
	}
}
void clique::createPartitions() {
	nPartitions = -1;
	int ps = -1;
	bm128 bm;
	bm.clear();
	for(int i = 0; i < size; i++) {
		ua[i].numDisjoints = 0;
		if(ua[i].nbits != ps) { //create a new partition
			partitions[++nPartitions].memberSizes = ps = ua[i].nbits;
			partitions[nPartitions].numMembers = 0;
			partitions[nPartitions].firstCliqueMember = i;
			partitions[nPartitions].bm.clear();
		}
		//add to the last partition
		partitions[nPartitions].numMembers++;
		partitions[nPartitions].bm |= ua[i];
		bm |= ua[i];
	}
	ua[size] -= bm; //rest of the positions
	ua[size].clearBits(fixedClues);
	ua[size].clearBits(fixedNonClues);
	ua[size].positionsByBitmap();
	//ua[size].accumulatedBM = ua[size] | bm; //should be all 81 bits but this will give const num DJ = 0 for each cell removed
	ua[size].accumulatedBM = ua[size]; //by exception only rest cells
	nPartitions++;
}
