/***************************************************************************
A Fast Simple Sudoku Solver by Mladen Dobrichev

For commercial users the actions allowed to this file are limited to erasing.
No limitations to other users, but citing somewhere my name is welcome.
There are parts of code taken from bb_sudoku by Brian Turner. Thank you, Brian!

The entry point is
	int solve(char* in, char* out, int mode).

Compiler flags:
-c -O2 -Ob1 -Oi -Ot -Oy -GA -MT -GS- -GR- -Wp64 -Gr
-Qopenmp -Qftz -QxSSE2 -Qopenmp-link:static
Which means:
compile_only, optimize_for_speed, expands_only_functions_marked_as_inline,
use_intrinsic_functions, favor_fast_code, omit_frame_pointers, fast_access_to_tls_storage,
multithreaded_executable, no_buffer_security_check, disable_runtime_type_info,
detect_64-bit_portability_issues, use__fastcall_calling_convention,
generate_parallel_code, flush_denormal_fp_result_to_zero, use_SSE2_instructions,
link_statically_to_openmp

Some counterintuitive compilation hints:
- no global optimizations, only for speed
- no inlining
- no interprocedural optimizations
- no Profile Guided Optimizations

Therminology:
"Game" is the puzzle along with the whole context during processing.
Puzzle consist of 81 "Cells".
Each cell is member of three "Groups" - its "Row", "Column", and "Square".
The rest of the cells within the same row, column, and square are "Affected Cells".
Segment is a combination of first/second/third three rows or columns.
"Triplets" are first/second/third subsequent 3 cells within the rows and columns.
"Affected Triplets" for a given triplet are the rest of the triplets in the same row/col and square.
***************************************************************************/

//No global variables
//No runtime integer division or multiplication
//Runtime Library function calls are limited to memcpy (hidden, when assigning the game struct), setjmp, and longjmp.

//There is an Intel Compiler specific pragma "unroll" which causes significant improvement in the SetDigit function.
//It is critical this loop to be unrolled to obtain good performance. Splitting the loop into chunks
//may cause other compilers to unroll them. Last resort is manual unrolling.
//#define __INTEL_COMPILER

//#define USE_LOCKED_CANDIDATES //a bit slower

#ifndef __INTEL_COMPILER
#define MANUAL_UNROLL
#endif //__INTEL_COMPILER

#include <limits.h>
#include <memory.h>
#include "solver.h"
#include "tables.h"
//#include "grid.h" //for unavoidables
//#include "rowminlex.h" //for essentially different solutions

//Use constants and tabular functions whenever possible

//Convert mask to a number (1 to 9) if only the appropriate bit is set.
//Convert zero to 9, allowing cellDigit to char conversion using the same table.
//Convert all the rest to zero.
extern const unsigned int Bitmap2Digit[512] =
{
        9,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

/* Zero based cell enumeration
 9 10 11  12 13 14  15 16 17    Group

00 01 02  03 04 05  06 07 08	0
09 10 11  12 13 14  15 16 17	1	18	19	20
18 19 20  21 22 23  24 25 26	2

27 28 29  30 31 32  33 34 35	3
36 37 38  39 40 41  42 43 44	4	21	22	23
45 46 47  48 49 50  51 52 53	5

54 55 56  57 58 59  60 61 62	6
63 64 65  66 67 68  69 70 71	7	24	25	26
72 73 74  75 76 77  78 79 80	8
*/

//#ifdef USE_LOCKED_CANDIDATES
/*
Segment enumeration
0 0 0     3 4 5
1 1 1     3 4 5
2 2 2     3 4 5

Triplets within a segment enumeration
000 111 222
333 444 555
666 777 888
*/

static const int affectedTriplets[9][4] =
  {{1,2,3,6},{0,2,4,7},{0,1,5,8},{4,5,0,6},{3,5,1,7},{3,4,2,8},{7,8,0,3},{6,8,1,4},{6,7,2,5}};

//6 segments * 9 triplets * 12 affected cells
static const int tripletAffectedCells[6][9][12] =
{
	{
	{ 3, 4, 5, 6, 7, 8, 9,10,11,18,19,20},{ 0, 1, 2, 6, 7, 8,12,13,14,21,22,23},{ 0, 1, 2, 3, 4, 5,15,16,17,24,25,26},
	{12,13,14,15,16,17, 0, 1, 2,18,19,20},{ 9,10,11,15,16,17, 3, 4, 5,21,22,23},{ 9,10,11,12,13,14, 6, 7, 8,24,25,26},
	{21,22,23,24,25,26, 0, 1, 2, 9,10,11},{18,19,20,24,25,26, 3, 4, 5,12,13,14},{18,19,20,21,22,23, 6, 7, 8,15,16,17}
	},{
	{30,31,32,33,34,35,36,37,38,45,46,47},{27,28,29,33,34,35,39,40,41,48,49,50},{27,28,29,30,31,32,42,43,44,51,52,53},
	{39,40,41,42,43,44,27,28,29,45,46,47},{36,37,38,42,43,44,30,31,32,48,49,50},{36,37,38,39,40,41,33,34,35,51,52,53},
	{48,49,50,51,52,53,27,28,29,36,37,38},{45,46,47,51,52,53,30,31,32,39,40,41},{45,46,47,48,49,50,33,34,35,42,43,44}
	},{
	{57,58,59,60,61,62,63,64,65,72,73,74},{54,55,56,60,61,62,66,67,68,75,76,77},{54,55,56,57,58,59,69,70,71,78,79,80},
	{66,67,68,69,70,71,54,55,56,72,73,74},{63,64,65,69,70,71,57,58,59,75,76,77},{63,64,65,66,67,68,60,61,62,78,79,80},
	{75,76,77,78,79,80,54,55,56,63,64,65},{72,73,74,78,79,80,57,58,59,66,67,68},{72,73,74,75,76,77,60,61,62,69,70,71}
	},{
	{27,36,45,54,63,72, 1,10,19, 2,11,20},{ 0, 9,18,54,63,72,28,37,46,29,38,47},{ 0, 9,18,27,36,45,55,64,73,56,65,74},
	{28,37,46,55,64,73, 0, 9,18, 2,11,20},{ 1,10,19,55,64,73,27,36,45,29,38,47},{ 1,10,19,28,37,46,54,63,72,56,65,74},
	{29,38,47,56,65,74, 0, 9,18, 1,10,19},{ 2,11,20,56,65,74,27,36,45,28,37,46},{ 2,11,20,29,38,47,54,63,72,55,64,73}
	},{
	{30,39,48,57,66,75, 4,13,22, 5,14,23},{ 3,12,21,57,66,75,31,40,49,32,41,50},{ 3,12,21,30,39,48,58,67,76,59,68,77},
	{31,40,49,58,67,76, 3,12,21, 5,14,23},{ 4,13,22,58,67,76,30,39,48,32,41,50},{ 4,13,22,31,40,49,57,66,75,59,68,77},
	{32,41,50,59,68,77, 3,12,21, 4,13,22},{ 5,14,23,59,68,77,30,39,48,31,40,49},{ 5,14,23,32,41,50,57,66,75,58,67,76}
	},{
	{33,42,51,60,69,78, 7,16,25, 8,17,26},{ 6,15,24,60,69,78,34,43,52,35,44,53},{ 6,15,24,33,42,51,61,70,79,62,71,80},
	{34,43,52,61,70,79, 6,15,24, 8,17,26},{ 7,16,25,61,70,79,33,42,51,35,44,53},{ 7,16,25,34,43,52,60,69,78,62,71,80},
	{35,44,53,62,71,80, 6,15,24, 7,16,25},{ 8,17,26,62,71,80,33,42,51,34,43,52},{ 8,17,26,35,44,53,60,69,78,61,70,79}
	}
};

//6 segments * 9 triplets * 3 cells in triplet
static const int tripletCells[6][9][3] =
{
	{{ 0, 1, 2},{ 3, 4, 5},{ 6, 7, 8},{ 9,10,11},{12,13,14},{15,16,17},{18,19,20},{21,22,23},{24,25,26}},
	{{27,28,29},{30,31,32},{33,34,35},{36,37,38},{39,40,41},{42,43,44},{45,46,47},{48,49,50},{51,52,53}},
	{{54,55,56},{57,58,59},{60,61,62},{63,64,65},{66,67,68},{69,70,71},{72,73,74},{75,76,77},{78,79,80}},
	{{ 0, 9,18},{27,36,45},{54,63,72},{ 1,10,19},{28,37,46},{55,64,73},{ 2,11,20},{29,38,47},{56,65,74}},
	{{ 3,12,21},{30,39,48},{57,66,75},{ 4,13,22},{31,40,49},{58,67,76},{ 5,14,23},{32,41,50},{59,68,77}},
	{{ 6,15,24},{33,42,51},{60,69,78},{ 7,16,25},{34,43,52},{61,70,79},{ 8,17,26},{35,44,53},{62,71,80}}
};
//#endif //USE_LOCKED_CANDIDATES

//game mode flags
#define MODE_SOLVING			0	//unused, keep solving
//#define MODE_STORE_SOLUTIONS	1	//initial 0
#define MODE_STOP_PROCESSING	4	//solved or errored
#define MODE_STOP_GUESSING		8	//necessary solutions found
#define MODE_EDIFFERENT			16	//stop at first essentially different solution

//The whole game context, cloned before guessing, and copied back after successful guess.
struct game
{
	bitmap groupKnownDigits[27];	//initial 0
	bitmap cellPossibilities[81];	//0==known, initial 511=0x01FF
	int mode;						//combination of the game mode flags, initial 0
	int cellsLeft;					//initial 81
	int lastGuess;					//initial somewhere in the middle
	//int nSolutions;
	unsigned long long nSolutions;
	//int maxSolutions;
	unsigned long long maxSolutions;
	//game *masterGame;				//sometimes we are jumping directly to g->masterGame->done
	cellDigit *cellDigits;
	char *results;					//external buffer for solutions
	//uaCollector *uaColl;			//callback class for processing the solutions one by one
	const int *knownSolution;		//pointer to a bitmapped solution to compare with or to guess from
	int *pencilmarks;				//pointer to a bitmapped possibilities for multisolution puzzle
	//int guessDepth;
	//int *maxGuessDepth;
};

//A template used for game structure initialization.
static const game defaultGame =
{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, //groupKnownDigits
	{ //cellPossibilities
		511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,
		511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,
		511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511,511
	},
	MODE_SOLVING,
	81, //cellsLeft
	40, //lastGuess
	0, //nSolutions
	2, //maxSolutions
	//NULL, //&masterGame
	NULL, //&cellDigits
	NULL, //&results
	//NULL, //&uaCollector
	NULL, //&knownSolution
	NULL, //&pencilmarks
	//0, //guessDepth
	//NULL //&maxGuessDepth
};

//extern int z0, z1;

void solutionFound(game &g) {
	if(g.pencilmarks) { //compose pencilmarks
		for(int j = 0; j < 81; j++) {
			if(g.cellDigits[j]) {
				g.pencilmarks[j] |= g.cellDigits[j];
			}
			else { //special case for 9
				g.pencilmarks[j] |= 256;
			}
		}
	}
//	else if(g.uaColl) {//collect UA sets
//		if (g.nSolutions) {//callback for each solution except the first
//			char sol[81];
//			for(int j = 0; j < 81; j++) {
//				sol[j] = Bitmap2Digit[g.cellDigits[j]];
//			}
//			if(g.uaColl->addSolution(sol)) { //limit reached
//				//g->nSolutions = g->maxSolutions;
//				g.mode |= (MODE_STOP_PROCESSING | MODE_STOP_GUESSING); //force exit
//				return;
//			}
//		}
//	}
	else if(g.results) {//add the solution to the external list
		for(int j = 0; j < 81; j++) {
			g.results[81 * g.nSolutions + j] = Bitmap2Digit[g.cellDigits[j]];
		}
	}
	g.nSolutions++;
	//if(g.nSolutions % 1000000 == 0) printf("."); //debug
	if(g.nSolutions >= g.maxSolutions) {//no more guesses, everything is done
		g.mode |= (MODE_STOP_PROCESSING | MODE_STOP_GUESSING);
		return;
	}
	g.mode |= MODE_STOP_PROCESSING;
}

//Set a known digit at a known cell.
//Update (clear) possibilities at affected cells and call recursively
//when a single possibility is found.
//Update some other context
static void setDigit(game &g, const cellIndex pi, const bitmap pbm)
{
	bitmap *const gkd = g.groupKnownDigits;
	bitmap *const cp = g.cellPossibilities;

	cellIndex qi[82];
	bitmap qbm[82];
	int qTop = 0;
	int n = 0;
	//qi[0] = pi;	qbm[0] = pbm;
	int i = pi;
	int bm = pbm;
	//setting the same digit for second time may corrupt the rest of the data
	//if(cp[pi] == 0) { //puzzle is overdetermined
	//	return;
	//}

//process_queue:
	do {
		if(cp[i] == 0) continue; //silently ignore setting of already solved cell
		//if((cp[i] & bm) == 0) goto ret_stop; //never happens
		const int *const ag = affectedGroups[i];

		//if the digit we are setting has been previously set for one of
		//the 3 groups the cell is member of, we are in a wrong way
		if(0 == (bm & (gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]]))) {
			;
		}
		else {
			goto ret_stop;
		}
		cp[i] = 0; //clear the mask to exclude this cell from further processing

		//if we want the final solution, store the digit for printing
		if(g.cellDigits) {
			g.cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.
		}

		if(--g.cellsLeft) {
			;
		}
		else {//solved
			solutionFound(g);
			return;
		}

		//set the digit as solved for 3 groups
		gkd[ag[0]] |= bm; gkd[ag[1]] |= bm; gkd[ag[2]] |= bm;

		//clear the known digit from the possibilities of all related cells
		const cellIndex *const ac = affectedCells[i];
//#define clp( ci ) \
//	{\
//		bitmap &bbm = cp[ac[ci]];\
//		if(0 == (bbm & bm)) {\
//			;\
//		}\
//		else {\
//			bbm ^= bm;\
//			if(bbm) {\
//				if(0 == Bitmap2Digit[bbm]) {\
//					;\
//				}\
//				else {\
//					qi[qTop] = ac[ci], qbm[qTop++] = bbm;\
//				}\
//			}\
//			else {\
//				goto ret_stop;\
//			}\
//		}\
//	}

#define CCELL( acci ) \
	{\
		bitmap &bbm = cp[acci];\
		if(0 == (bbm & bm)) {\
			;\
		}\
		else {\
			bbm ^= bm;\
			if(bbm) {\
				if(0 == Bitmap2Digit[bbm]) {\
					;\
				}\
				else {\
					qi[qTop] = acci, qbm[qTop++] = bbm;\
				}\
			}\
			else {\
				goto ret_stop;\
			}\
		}\
	}
#define clp( ci ) {CCELL( ac[ci] )}

		clp( 0);clp( 1);clp( 2);clp( 3);clp( 4);clp( 5);clp( 6);clp( 7);clp( 8);clp( 9);
		clp(10);clp(11);clp(12);clp(13);clp(14);clp(15);clp(16);clp(17);clp(18);clp(19);

		//switch(i)
		//{
		//case  0: CCELL(1);CCELL( 2);CCELL( 3);CCELL( 4);CCELL( 5);CCELL( 6);CCELL( 7);CCELL( 8);CCELL( 9);CCELL(10);CCELL(11);CCELL(18);CCELL(19);CCELL(20);CCELL(27);CCELL(36);CCELL(45);CCELL(54);CCELL(63);CCELL(72);break;
		//case  1: CCELL(0);CCELL( 2);CCELL( 3);CCELL( 4);CCELL( 5);CCELL( 6);CCELL( 7);CCELL( 8);CCELL( 9);CCELL(10);CCELL(11);CCELL(18);CCELL(19);CCELL(20);CCELL(28);CCELL(37);CCELL(46);CCELL(55);CCELL(64);CCELL(73);break;
		//case  2: CCELL(0);CCELL( 1);CCELL( 3);CCELL( 4);CCELL( 5);CCELL( 6);CCELL( 7);CCELL( 8);CCELL( 9);CCELL(10);CCELL(11);CCELL(18);CCELL(19);CCELL(20);CCELL(29);CCELL(38);CCELL(47);CCELL(56);CCELL(65);CCELL(74);break;
		//case  3: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 4);CCELL( 5);CCELL( 6);CCELL( 7);CCELL( 8);CCELL(12);CCELL(13);CCELL(14);CCELL(21);CCELL(22);CCELL(23);CCELL(30);CCELL(39);CCELL(48);CCELL(57);CCELL(66);CCELL(75);break;
		//case  4: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 3);CCELL( 5);CCELL( 6);CCELL( 7);CCELL( 8);CCELL(12);CCELL(13);CCELL(14);CCELL(21);CCELL(22);CCELL(23);CCELL(31);CCELL(40);CCELL(49);CCELL(58);CCELL(67);CCELL(76);break;
		//case  5: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 3);CCELL( 4);CCELL( 6);CCELL( 7);CCELL( 8);CCELL(12);CCELL(13);CCELL(14);CCELL(21);CCELL(22);CCELL(23);CCELL(32);CCELL(41);CCELL(50);CCELL(59);CCELL(68);CCELL(77);break;
		//case  6: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 3);CCELL( 4);CCELL( 5);CCELL( 7);CCELL( 8);CCELL(15);CCELL(16);CCELL(17);CCELL(24);CCELL(25);CCELL(26);CCELL(33);CCELL(42);CCELL(51);CCELL(60);CCELL(69);CCELL(78);break;
		//case  7: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 3);CCELL( 4);CCELL( 5);CCELL( 6);CCELL( 8);CCELL(15);CCELL(16);CCELL(17);CCELL(24);CCELL(25);CCELL(26);CCELL(34);CCELL(43);CCELL(52);CCELL(61);CCELL(70);CCELL(79);break;
		//case  8: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 3);CCELL( 4);CCELL( 5);CCELL( 6);CCELL( 7);CCELL(15);CCELL(16);CCELL(17);CCELL(24);CCELL(25);CCELL(26);CCELL(35);CCELL(44);CCELL(53);CCELL(62);CCELL(71);CCELL(80);break;
		//case  9: CCELL(0);CCELL( 1);CCELL( 2);CCELL(10);CCELL(11);CCELL(12);CCELL(13);CCELL(14);CCELL(15);CCELL(16);CCELL(17);CCELL(18);CCELL(19);CCELL(20);CCELL(27);CCELL(36);CCELL(45);CCELL(54);CCELL(63);CCELL(72);break;
		//case 10: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 9);CCELL(11);CCELL(12);CCELL(13);CCELL(14);CCELL(15);CCELL(16);CCELL(17);CCELL(18);CCELL(19);CCELL(20);CCELL(28);CCELL(37);CCELL(46);CCELL(55);CCELL(64);CCELL(73);break;
		//case 11: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 9);CCELL(10);CCELL(12);CCELL(13);CCELL(14);CCELL(15);CCELL(16);CCELL(17);CCELL(18);CCELL(19);CCELL(20);CCELL(29);CCELL(38);CCELL(47);CCELL(56);CCELL(65);CCELL(74);break;
		//case 12: CCELL(3);CCELL( 4);CCELL( 5);CCELL( 9);CCELL(10);CCELL(11);CCELL(13);CCELL(14);CCELL(15);CCELL(16);CCELL(17);CCELL(21);CCELL(22);CCELL(23);CCELL(30);CCELL(39);CCELL(48);CCELL(57);CCELL(66);CCELL(75);break;
		//case 13: CCELL(3);CCELL( 4);CCELL( 5);CCELL( 9);CCELL(10);CCELL(11);CCELL(12);CCELL(14);CCELL(15);CCELL(16);CCELL(17);CCELL(21);CCELL(22);CCELL(23);CCELL(31);CCELL(40);CCELL(49);CCELL(58);CCELL(67);CCELL(76);break;
		//case 14: CCELL(3);CCELL( 4);CCELL( 5);CCELL( 9);CCELL(10);CCELL(11);CCELL(12);CCELL(13);CCELL(15);CCELL(16);CCELL(17);CCELL(21);CCELL(22);CCELL(23);CCELL(32);CCELL(41);CCELL(50);CCELL(59);CCELL(68);CCELL(77);break;
		//case 15: CCELL(6);CCELL( 7);CCELL( 8);CCELL( 9);CCELL(10);CCELL(11);CCELL(12);CCELL(13);CCELL(14);CCELL(16);CCELL(17);CCELL(24);CCELL(25);CCELL(26);CCELL(33);CCELL(42);CCELL(51);CCELL(60);CCELL(69);CCELL(78);break;
		//case 16: CCELL(6);CCELL( 7);CCELL( 8);CCELL( 9);CCELL(10);CCELL(11);CCELL(12);CCELL(13);CCELL(14);CCELL(15);CCELL(17);CCELL(24);CCELL(25);CCELL(26);CCELL(34);CCELL(43);CCELL(52);CCELL(61);CCELL(70);CCELL(79);break;
		//case 17: CCELL(6);CCELL( 7);CCELL( 8);CCELL( 9);CCELL(10);CCELL(11);CCELL(12);CCELL(13);CCELL(14);CCELL(15);CCELL(16);CCELL(24);CCELL(25);CCELL(26);CCELL(35);CCELL(44);CCELL(53);CCELL(62);CCELL(71);CCELL(80);break;
		//case 18: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 9);CCELL(10);CCELL(11);CCELL(19);CCELL(20);CCELL(21);CCELL(22);CCELL(23);CCELL(24);CCELL(25);CCELL(26);CCELL(27);CCELL(36);CCELL(45);CCELL(54);CCELL(63);CCELL(72);break;
		//case 19: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 9);CCELL(10);CCELL(11);CCELL(18);CCELL(20);CCELL(21);CCELL(22);CCELL(23);CCELL(24);CCELL(25);CCELL(26);CCELL(28);CCELL(37);CCELL(46);CCELL(55);CCELL(64);CCELL(73);break;
		//case 20: CCELL(0);CCELL( 1);CCELL( 2);CCELL( 9);CCELL(10);CCELL(11);CCELL(18);CCELL(19);CCELL(21);CCELL(22);CCELL(23);CCELL(24);CCELL(25);CCELL(26);CCELL(29);CCELL(38);CCELL(47);CCELL(56);CCELL(65);CCELL(74);break;
		//case 21: CCELL(3);CCELL( 4);CCELL( 5);CCELL(12);CCELL(13);CCELL(14);CCELL(18);CCELL(19);CCELL(20);CCELL(22);CCELL(23);CCELL(24);CCELL(25);CCELL(26);CCELL(30);CCELL(39);CCELL(48);CCELL(57);CCELL(66);CCELL(75);break;
		//case 22: CCELL(3);CCELL( 4);CCELL( 5);CCELL(12);CCELL(13);CCELL(14);CCELL(18);CCELL(19);CCELL(20);CCELL(21);CCELL(23);CCELL(24);CCELL(25);CCELL(26);CCELL(31);CCELL(40);CCELL(49);CCELL(58);CCELL(67);CCELL(76);break;
		//case 23: CCELL(3);CCELL( 4);CCELL( 5);CCELL(12);CCELL(13);CCELL(14);CCELL(18);CCELL(19);CCELL(20);CCELL(21);CCELL(22);CCELL(24);CCELL(25);CCELL(26);CCELL(32);CCELL(41);CCELL(50);CCELL(59);CCELL(68);CCELL(77);break;
		//case 24: CCELL(6);CCELL( 7);CCELL( 8);CCELL(15);CCELL(16);CCELL(17);CCELL(18);CCELL(19);CCELL(20);CCELL(21);CCELL(22);CCELL(23);CCELL(25);CCELL(26);CCELL(33);CCELL(42);CCELL(51);CCELL(60);CCELL(69);CCELL(78);break;
		//case 25: CCELL(6);CCELL( 7);CCELL( 8);CCELL(15);CCELL(16);CCELL(17);CCELL(18);CCELL(19);CCELL(20);CCELL(21);CCELL(22);CCELL(23);CCELL(24);CCELL(26);CCELL(34);CCELL(43);CCELL(52);CCELL(61);CCELL(70);CCELL(79);break;
		//case 26: CCELL(6);CCELL( 7);CCELL( 8);CCELL(15);CCELL(16);CCELL(17);CCELL(18);CCELL(19);CCELL(20);CCELL(21);CCELL(22);CCELL(23);CCELL(24);CCELL(25);CCELL(35);CCELL(44);CCELL(53);CCELL(62);CCELL(71);CCELL(80);break;
		//case 27: CCELL(0);CCELL( 9);CCELL(18);CCELL(28);CCELL(29);CCELL(30);CCELL(31);CCELL(32);CCELL(33);CCELL(34);CCELL(35);CCELL(36);CCELL(37);CCELL(38);CCELL(45);CCELL(46);CCELL(47);CCELL(54);CCELL(63);CCELL(72);break;
		//case 28: CCELL(1);CCELL(10);CCELL(19);CCELL(27);CCELL(29);CCELL(30);CCELL(31);CCELL(32);CCELL(33);CCELL(34);CCELL(35);CCELL(36);CCELL(37);CCELL(38);CCELL(45);CCELL(46);CCELL(47);CCELL(55);CCELL(64);CCELL(73);break;
		//case 29: CCELL(2);CCELL(11);CCELL(20);CCELL(27);CCELL(28);CCELL(30);CCELL(31);CCELL(32);CCELL(33);CCELL(34);CCELL(35);CCELL(36);CCELL(37);CCELL(38);CCELL(45);CCELL(46);CCELL(47);CCELL(56);CCELL(65);CCELL(74);break;
		//case 30: CCELL(3);CCELL(12);CCELL(21);CCELL(27);CCELL(28);CCELL(29);CCELL(31);CCELL(32);CCELL(33);CCELL(34);CCELL(35);CCELL(39);CCELL(40);CCELL(41);CCELL(48);CCELL(49);CCELL(50);CCELL(57);CCELL(66);CCELL(75);break;
		//case 31: CCELL(4);CCELL(13);CCELL(22);CCELL(27);CCELL(28);CCELL(29);CCELL(30);CCELL(32);CCELL(33);CCELL(34);CCELL(35);CCELL(39);CCELL(40);CCELL(41);CCELL(48);CCELL(49);CCELL(50);CCELL(58);CCELL(67);CCELL(76);break;
		//case 32: CCELL(5);CCELL(14);CCELL(23);CCELL(27);CCELL(28);CCELL(29);CCELL(30);CCELL(31);CCELL(33);CCELL(34);CCELL(35);CCELL(39);CCELL(40);CCELL(41);CCELL(48);CCELL(49);CCELL(50);CCELL(59);CCELL(68);CCELL(77);break;
		//case 33: CCELL(6);CCELL(15);CCELL(24);CCELL(27);CCELL(28);CCELL(29);CCELL(30);CCELL(31);CCELL(32);CCELL(34);CCELL(35);CCELL(42);CCELL(43);CCELL(44);CCELL(51);CCELL(52);CCELL(53);CCELL(60);CCELL(69);CCELL(78);break;
		//case 34: CCELL(7);CCELL(16);CCELL(25);CCELL(27);CCELL(28);CCELL(29);CCELL(30);CCELL(31);CCELL(32);CCELL(33);CCELL(35);CCELL(42);CCELL(43);CCELL(44);CCELL(51);CCELL(52);CCELL(53);CCELL(61);CCELL(70);CCELL(79);break;
		//case 35: CCELL(8);CCELL(17);CCELL(26);CCELL(27);CCELL(28);CCELL(29);CCELL(30);CCELL(31);CCELL(32);CCELL(33);CCELL(34);CCELL(42);CCELL(43);CCELL(44);CCELL(51);CCELL(52);CCELL(53);CCELL(62);CCELL(71);CCELL(80);break;
		//case 36: CCELL(0);CCELL( 9);CCELL(18);CCELL(27);CCELL(28);CCELL(29);CCELL(37);CCELL(38);CCELL(39);CCELL(40);CCELL(41);CCELL(42);CCELL(43);CCELL(44);CCELL(45);CCELL(46);CCELL(47);CCELL(54);CCELL(63);CCELL(72);break;
		//case 37: CCELL(1);CCELL(10);CCELL(19);CCELL(27);CCELL(28);CCELL(29);CCELL(36);CCELL(38);CCELL(39);CCELL(40);CCELL(41);CCELL(42);CCELL(43);CCELL(44);CCELL(45);CCELL(46);CCELL(47);CCELL(55);CCELL(64);CCELL(73);break;
		//case 38: CCELL(2);CCELL(11);CCELL(20);CCELL(27);CCELL(28);CCELL(29);CCELL(36);CCELL(37);CCELL(39);CCELL(40);CCELL(41);CCELL(42);CCELL(43);CCELL(44);CCELL(45);CCELL(46);CCELL(47);CCELL(56);CCELL(65);CCELL(74);break;
		//case 39: CCELL(3);CCELL(12);CCELL(21);CCELL(30);CCELL(31);CCELL(32);CCELL(36);CCELL(37);CCELL(38);CCELL(40);CCELL(41);CCELL(42);CCELL(43);CCELL(44);CCELL(48);CCELL(49);CCELL(50);CCELL(57);CCELL(66);CCELL(75);break;
		//case 40: CCELL(4);CCELL(13);CCELL(22);CCELL(30);CCELL(31);CCELL(32);CCELL(36);CCELL(37);CCELL(38);CCELL(39);CCELL(41);CCELL(42);CCELL(43);CCELL(44);CCELL(48);CCELL(49);CCELL(50);CCELL(58);CCELL(67);CCELL(76);break;
		//case 41: CCELL(5);CCELL(14);CCELL(23);CCELL(30);CCELL(31);CCELL(32);CCELL(36);CCELL(37);CCELL(38);CCELL(39);CCELL(40);CCELL(42);CCELL(43);CCELL(44);CCELL(48);CCELL(49);CCELL(50);CCELL(59);CCELL(68);CCELL(77);break;
		//case 42: CCELL(6);CCELL(15);CCELL(24);CCELL(33);CCELL(34);CCELL(35);CCELL(36);CCELL(37);CCELL(38);CCELL(39);CCELL(40);CCELL(41);CCELL(43);CCELL(44);CCELL(51);CCELL(52);CCELL(53);CCELL(60);CCELL(69);CCELL(78);break;
		//case 43: CCELL(7);CCELL(16);CCELL(25);CCELL(33);CCELL(34);CCELL(35);CCELL(36);CCELL(37);CCELL(38);CCELL(39);CCELL(40);CCELL(41);CCELL(42);CCELL(44);CCELL(51);CCELL(52);CCELL(53);CCELL(61);CCELL(70);CCELL(79);break;
		//case 44: CCELL(8);CCELL(17);CCELL(26);CCELL(33);CCELL(34);CCELL(35);CCELL(36);CCELL(37);CCELL(38);CCELL(39);CCELL(40);CCELL(41);CCELL(42);CCELL(43);CCELL(51);CCELL(52);CCELL(53);CCELL(62);CCELL(71);CCELL(80);break;
		//case 45: CCELL(0);CCELL( 9);CCELL(18);CCELL(27);CCELL(28);CCELL(29);CCELL(36);CCELL(37);CCELL(38);CCELL(46);CCELL(47);CCELL(48);CCELL(49);CCELL(50);CCELL(51);CCELL(52);CCELL(53);CCELL(54);CCELL(63);CCELL(72);break;
		//case 46: CCELL(1);CCELL(10);CCELL(19);CCELL(27);CCELL(28);CCELL(29);CCELL(36);CCELL(37);CCELL(38);CCELL(45);CCELL(47);CCELL(48);CCELL(49);CCELL(50);CCELL(51);CCELL(52);CCELL(53);CCELL(55);CCELL(64);CCELL(73);break;
		//case 47: CCELL(2);CCELL(11);CCELL(20);CCELL(27);CCELL(28);CCELL(29);CCELL(36);CCELL(37);CCELL(38);CCELL(45);CCELL(46);CCELL(48);CCELL(49);CCELL(50);CCELL(51);CCELL(52);CCELL(53);CCELL(56);CCELL(65);CCELL(74);break;
		//case 48: CCELL(3);CCELL(12);CCELL(21);CCELL(30);CCELL(31);CCELL(32);CCELL(39);CCELL(40);CCELL(41);CCELL(45);CCELL(46);CCELL(47);CCELL(49);CCELL(50);CCELL(51);CCELL(52);CCELL(53);CCELL(57);CCELL(66);CCELL(75);break;
		//case 49: CCELL(4);CCELL(13);CCELL(22);CCELL(30);CCELL(31);CCELL(32);CCELL(39);CCELL(40);CCELL(41);CCELL(45);CCELL(46);CCELL(47);CCELL(48);CCELL(50);CCELL(51);CCELL(52);CCELL(53);CCELL(58);CCELL(67);CCELL(76);break;
		//case 50: CCELL(5);CCELL(14);CCELL(23);CCELL(30);CCELL(31);CCELL(32);CCELL(39);CCELL(40);CCELL(41);CCELL(45);CCELL(46);CCELL(47);CCELL(48);CCELL(49);CCELL(51);CCELL(52);CCELL(53);CCELL(59);CCELL(68);CCELL(77);break;
		//case 51: CCELL(6);CCELL(15);CCELL(24);CCELL(33);CCELL(34);CCELL(35);CCELL(42);CCELL(43);CCELL(44);CCELL(45);CCELL(46);CCELL(47);CCELL(48);CCELL(49);CCELL(50);CCELL(52);CCELL(53);CCELL(60);CCELL(69);CCELL(78);break;
		//case 52: CCELL(7);CCELL(16);CCELL(25);CCELL(33);CCELL(34);CCELL(35);CCELL(42);CCELL(43);CCELL(44);CCELL(45);CCELL(46);CCELL(47);CCELL(48);CCELL(49);CCELL(50);CCELL(51);CCELL(53);CCELL(61);CCELL(70);CCELL(79);break;
		//case 53: CCELL(8);CCELL(17);CCELL(26);CCELL(33);CCELL(34);CCELL(35);CCELL(42);CCELL(43);CCELL(44);CCELL(45);CCELL(46);CCELL(47);CCELL(48);CCELL(49);CCELL(50);CCELL(51);CCELL(52);CCELL(62);CCELL(71);CCELL(80);break;
		//case 54: CCELL(0);CCELL( 9);CCELL(18);CCELL(27);CCELL(36);CCELL(45);CCELL(55);CCELL(56);CCELL(57);CCELL(58);CCELL(59);CCELL(60);CCELL(61);CCELL(62);CCELL(63);CCELL(64);CCELL(65);CCELL(72);CCELL(73);CCELL(74);break;
		//case 55: CCELL(1);CCELL(10);CCELL(19);CCELL(28);CCELL(37);CCELL(46);CCELL(54);CCELL(56);CCELL(57);CCELL(58);CCELL(59);CCELL(60);CCELL(61);CCELL(62);CCELL(63);CCELL(64);CCELL(65);CCELL(72);CCELL(73);CCELL(74);break;
		//case 56: CCELL(2);CCELL(11);CCELL(20);CCELL(29);CCELL(38);CCELL(47);CCELL(54);CCELL(55);CCELL(57);CCELL(58);CCELL(59);CCELL(60);CCELL(61);CCELL(62);CCELL(63);CCELL(64);CCELL(65);CCELL(72);CCELL(73);CCELL(74);break;
		//case 57: CCELL(3);CCELL(12);CCELL(21);CCELL(30);CCELL(39);CCELL(48);CCELL(54);CCELL(55);CCELL(56);CCELL(58);CCELL(59);CCELL(60);CCELL(61);CCELL(62);CCELL(66);CCELL(67);CCELL(68);CCELL(75);CCELL(76);CCELL(77);break;
		//case 58: CCELL(4);CCELL(13);CCELL(22);CCELL(31);CCELL(40);CCELL(49);CCELL(54);CCELL(55);CCELL(56);CCELL(57);CCELL(59);CCELL(60);CCELL(61);CCELL(62);CCELL(66);CCELL(67);CCELL(68);CCELL(75);CCELL(76);CCELL(77);break;
		//case 59: CCELL(5);CCELL(14);CCELL(23);CCELL(32);CCELL(41);CCELL(50);CCELL(54);CCELL(55);CCELL(56);CCELL(57);CCELL(58);CCELL(60);CCELL(61);CCELL(62);CCELL(66);CCELL(67);CCELL(68);CCELL(75);CCELL(76);CCELL(77);break;
		//case 60: CCELL(6);CCELL(15);CCELL(24);CCELL(33);CCELL(42);CCELL(51);CCELL(54);CCELL(55);CCELL(56);CCELL(57);CCELL(58);CCELL(59);CCELL(61);CCELL(62);CCELL(69);CCELL(70);CCELL(71);CCELL(78);CCELL(79);CCELL(80);break;
		//case 61: CCELL(7);CCELL(16);CCELL(25);CCELL(34);CCELL(43);CCELL(52);CCELL(54);CCELL(55);CCELL(56);CCELL(57);CCELL(58);CCELL(59);CCELL(60);CCELL(62);CCELL(69);CCELL(70);CCELL(71);CCELL(78);CCELL(79);CCELL(80);break;
		//case 62: CCELL(8);CCELL(17);CCELL(26);CCELL(35);CCELL(44);CCELL(53);CCELL(54);CCELL(55);CCELL(56);CCELL(57);CCELL(58);CCELL(59);CCELL(60);CCELL(61);CCELL(69);CCELL(70);CCELL(71);CCELL(78);CCELL(79);CCELL(80);break;
		//case 63: CCELL(0);CCELL( 9);CCELL(18);CCELL(27);CCELL(36);CCELL(45);CCELL(54);CCELL(55);CCELL(56);CCELL(64);CCELL(65);CCELL(66);CCELL(67);CCELL(68);CCELL(69);CCELL(70);CCELL(71);CCELL(72);CCELL(73);CCELL(74);break;
		//case 64: CCELL(1);CCELL(10);CCELL(19);CCELL(28);CCELL(37);CCELL(46);CCELL(54);CCELL(55);CCELL(56);CCELL(63);CCELL(65);CCELL(66);CCELL(67);CCELL(68);CCELL(69);CCELL(70);CCELL(71);CCELL(72);CCELL(73);CCELL(74);break;
		//case 65: CCELL(2);CCELL(11);CCELL(20);CCELL(29);CCELL(38);CCELL(47);CCELL(54);CCELL(55);CCELL(56);CCELL(63);CCELL(64);CCELL(66);CCELL(67);CCELL(68);CCELL(69);CCELL(70);CCELL(71);CCELL(72);CCELL(73);CCELL(74);break;
		//case 66: CCELL(3);CCELL(12);CCELL(21);CCELL(30);CCELL(39);CCELL(48);CCELL(57);CCELL(58);CCELL(59);CCELL(63);CCELL(64);CCELL(65);CCELL(67);CCELL(68);CCELL(69);CCELL(70);CCELL(71);CCELL(75);CCELL(76);CCELL(77);break;
		//case 67: CCELL(4);CCELL(13);CCELL(22);CCELL(31);CCELL(40);CCELL(49);CCELL(57);CCELL(58);CCELL(59);CCELL(63);CCELL(64);CCELL(65);CCELL(66);CCELL(68);CCELL(69);CCELL(70);CCELL(71);CCELL(75);CCELL(76);CCELL(77);break;
		//case 68: CCELL(5);CCELL(14);CCELL(23);CCELL(32);CCELL(41);CCELL(50);CCELL(57);CCELL(58);CCELL(59);CCELL(63);CCELL(64);CCELL(65);CCELL(66);CCELL(67);CCELL(69);CCELL(70);CCELL(71);CCELL(75);CCELL(76);CCELL(77);break;
		//case 69: CCELL(6);CCELL(15);CCELL(24);CCELL(33);CCELL(42);CCELL(51);CCELL(60);CCELL(61);CCELL(62);CCELL(63);CCELL(64);CCELL(65);CCELL(66);CCELL(67);CCELL(68);CCELL(70);CCELL(71);CCELL(78);CCELL(79);CCELL(80);break;
		//case 70: CCELL(7);CCELL(16);CCELL(25);CCELL(34);CCELL(43);CCELL(52);CCELL(60);CCELL(61);CCELL(62);CCELL(63);CCELL(64);CCELL(65);CCELL(66);CCELL(67);CCELL(68);CCELL(69);CCELL(71);CCELL(78);CCELL(79);CCELL(80);break;
		//case 71: CCELL(8);CCELL(17);CCELL(26);CCELL(35);CCELL(44);CCELL(53);CCELL(60);CCELL(61);CCELL(62);CCELL(63);CCELL(64);CCELL(65);CCELL(66);CCELL(67);CCELL(68);CCELL(69);CCELL(70);CCELL(78);CCELL(79);CCELL(80);break;
		//case 72: CCELL(0);CCELL( 9);CCELL(18);CCELL(27);CCELL(36);CCELL(45);CCELL(54);CCELL(55);CCELL(56);CCELL(63);CCELL(64);CCELL(65);CCELL(73);CCELL(74);CCELL(75);CCELL(76);CCELL(77);CCELL(78);CCELL(79);CCELL(80);break;
		//case 73: CCELL(1);CCELL(10);CCELL(19);CCELL(28);CCELL(37);CCELL(46);CCELL(54);CCELL(55);CCELL(56);CCELL(63);CCELL(64);CCELL(65);CCELL(72);CCELL(74);CCELL(75);CCELL(76);CCELL(77);CCELL(78);CCELL(79);CCELL(80);break;
		//case 74: CCELL(2);CCELL(11);CCELL(20);CCELL(29);CCELL(38);CCELL(47);CCELL(54);CCELL(55);CCELL(56);CCELL(63);CCELL(64);CCELL(65);CCELL(72);CCELL(73);CCELL(75);CCELL(76);CCELL(77);CCELL(78);CCELL(79);CCELL(80);break;
		//case 75: CCELL(3);CCELL(12);CCELL(21);CCELL(30);CCELL(39);CCELL(48);CCELL(57);CCELL(58);CCELL(59);CCELL(66);CCELL(67);CCELL(68);CCELL(72);CCELL(73);CCELL(74);CCELL(76);CCELL(77);CCELL(78);CCELL(79);CCELL(80);break;
		//case 76: CCELL(4);CCELL(13);CCELL(22);CCELL(31);CCELL(40);CCELL(49);CCELL(57);CCELL(58);CCELL(59);CCELL(66);CCELL(67);CCELL(68);CCELL(72);CCELL(73);CCELL(74);CCELL(75);CCELL(77);CCELL(78);CCELL(79);CCELL(80);break;
		//case 77: CCELL(5);CCELL(14);CCELL(23);CCELL(32);CCELL(41);CCELL(50);CCELL(57);CCELL(58);CCELL(59);CCELL(66);CCELL(67);CCELL(68);CCELL(72);CCELL(73);CCELL(74);CCELL(75);CCELL(76);CCELL(78);CCELL(79);CCELL(80);break;
		//case 78: CCELL(6);CCELL(15);CCELL(24);CCELL(33);CCELL(42);CCELL(51);CCELL(60);CCELL(61);CCELL(62);CCELL(69);CCELL(70);CCELL(71);CCELL(72);CCELL(73);CCELL(74);CCELL(75);CCELL(76);CCELL(77);CCELL(79);CCELL(80);break;
		//case 79: CCELL(7);CCELL(16);CCELL(25);CCELL(34);CCELL(43);CCELL(52);CCELL(60);CCELL(61);CCELL(62);CCELL(69);CCELL(70);CCELL(71);CCELL(72);CCELL(73);CCELL(74);CCELL(75);CCELL(76);CCELL(77);CCELL(78);CCELL(80);break;
		//case 80: CCELL(8);CCELL(17);CCELL(26);CCELL(35);CCELL(44);CCELL(53);CCELL(60);CCELL(61);CCELL(62);CCELL(69);CCELL(70);CCELL(71);CCELL(72);CCELL(73);CCELL(74);CCELL(75);CCELL(76);CCELL(77);CCELL(78);CCELL(79);break;
		//}

#ifdef SOLVER_BY_GROUPS
		while(qTop - n > 3) {
			for(int x = n; x < qTop; x++) {
				i = qi[x], bm = qbm[x];
				if(cp[i] == 0) continue; //silently ignore setting of already solved cell
				//if((cp[i] & bm) == 0) goto ret_stop; //never happens
				const int *const ag = affectedGroups[i];
				const cellIndex *const ac = affectedCells[i];

				//if the digit we are setting has been previously set for one of
				//the 3 groups the cell is member of, we are in a wrong way
				if(bm & (gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]])) {
					goto ret_stop;
				}
				cp[i] = 0; //clear the mask to exclude this cell from further processing

				//if we want the final solution, store the digit for printing
				if(g.cellDigits) {
					g.cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.
				}

				if(0 == --g.cellsLeft) {//solved
					solutionFound(g);
					return;
				}

				//set the digit as solved for 3 groups
				gkd[ag[0]] |= bm; gkd[ag[1]] |= bm; gkd[ag[2]] |= bm;
			}
			//empty the queue
			n = qTop = 0;
			//now cleanup affected cells by groups
			for(int i = 0; i < 81; i++) {
				bitmap &bm = cp[i];
				const int *ag = affectedGroups[i];
				bitmap knowns = gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]];
				if(bm & knowns) { //there are possibilities to be cleared
					bm &= ~knowns;
					if(bm == 0) { //no possibilities for this cell
						g.mode |= MODE_STOP_PROCESSING;
						return;
					}
					if(Bitmap2Digit[bm]) { //single digit?
						//setDigit(g, i, bm);
						//if(g.mode & MODE_STOP_PROCESSING) return;
						qi[qTop] = i, qbm[qTop++] = bm;
					}
				}
			}
		}
#endif

	} while (i = qi[n],	bm = qbm[n++], n <= qTop);
	return;
ret_stop:
	g.mode |= MODE_STOP_PROCESSING;
	return;
}

//Loop trough the groups, find the only cell containing particular digit in the group if any,
//then set the digit found.
//Since setting the digit changes the context, repeat until nothing found.
//On exit, the slower algorithms are expected to be run (i.e. guessing). 
static void /*__declspec(noinline)*/ checkForLastOccurenceInGroup(game& g)
{
	//checks for group having some posible digit in only one cell
	const bitmap *const cp = g.cellPossibilities;
	const bitmap *const gkd = g.groupKnownDigits;
restart:
	for(int gi = 0; gi < 27; gi++) {
		//if(gkd[gi] == 511) continue;
		//if(BitCount[gkd[gi]] > 6) continue;
		int groupPoss = 0;
		int duplicates = 0;
		const cellIndex *const gc = cellsInGroup[gi];
#ifdef __INTEL_COMPILER
#pragma unroll(9)
#endif //__INTEL_COMPILER
		for(int ci = 0; ci < 9; ci++) {
			const int cellPoss = cp[gc[ci]];
			duplicates |= (groupPoss & cellPoss); //this tricky code is taken from bb_sudoku by Brian Turner
			groupPoss |= cellPoss;
		}
		if((groupPoss ^ gkd[gi]) != 511) { //no place for some of the unknown digits
			goto ret_err;
		}
		int uniques = groupPoss ^ duplicates;
		if(uniques == 0) continue;
		//clear the unique possibilities from the group and process the unique cells
		for(int ci = 0; ci < 9; ci++) {
			const int newPoss = cp[gc[ci]] & uniques;
			if(newPoss == 0) continue;
			//one of the cells of interest found
			if(Bitmap2Digit[newPoss]) {
				setDigit(g, gc[ci], newPoss);
				if(g.mode & MODE_STOP_PROCESSING) return;
				goto restart; //usually the rest uniques are directly eliminated in setDigit
				////the benefit from the following optimization is questionable
				//uniques ^= newPoss; //clear the already processed bit
				//if(uniques == 0) goto restart; //no more bits
			}
			else {
				//error: the same cell introduced > 1 unique digits in the group
				goto ret_err;
			}
		}
		//goto restart;
	} //group loop
	////TODO: search for pairs/triplets/quads
	//if(checkForSubsets(g))
	//	goto restart;
	return;
ret_err:
	g.mode |= MODE_STOP_PROCESSING;
	return;
}

//#ifdef USE_LOCKED_CANDIDATES
//Based on bb_sudoku by Brian Turner.
static void FindLockedCandidates (game &g)
{
	bitmap *const gcp = g.cellPossibilities;
	bitmap tripletPossibilities[9];
restart:
	//int found = 0;
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < 9; j++)
			tripletPossibilities[j] = gcp[tripletCells[i][j][0]] | gcp[tripletCells[i][j][1]] | gcp[tripletCells[i][j][2]];
		for(int j = 0; j < 9; j++) {
			const int b = (//found in the current triplet, and found in exactly one of the affected triplet pairs
				((tripletPossibilities[affectedTriplets[j][0]] | tripletPossibilities[affectedTriplets[j][1]]) ^ //row or column pair
				 (tripletPossibilities[affectedTriplets[j][2]] | tripletPossibilities[affectedTriplets[j][3]]))  //square pair
				& tripletPossibilities[j]);
			if(b == 0) continue;
			//don't care which bit where came from
			for(int k = 0; k < 12; k++) { //6 from the row/col + 6 from the square
				const int ci = tripletAffectedCells[i][j][k];
				bitmap &cp = gcp[ci];
				if(0 == (cp & b)) continue;
				//there is something to clear
				cp = (bitmap)(cp & ~b);
				if(cp) {
					if(0 == Bitmap2Digit[cp]) {
						//found = 1;
						continue;
					}
					//single possibility remains
					setDigit(g, ci, cp);
					if(g.mode & MODE_STOP_PROCESSING) return;
					checkForLastOccurenceInGroup(g);
					if(g.mode & MODE_STOP_PROCESSING) return;
					goto restart;
				}
				else {
					//no any possibility for this cell
					g.mode |= MODE_STOP_PROCESSING;
					return;
				}
			}
		} //for j
	} //for i

	//if(found) {
	//	checkForLastOccurenceInGroup(g);
	//	//goto restart;
	//}

	////experimental: search for pairs/triplets/quads
	//checkForLastOccurenceInGroup(g);
	//if(checkForSubsets(g)) {
	//	if(g.mode & MODE_STOP_PROCESSING) return;
	//	checkForLastOccurenceInGroup(g);
	//	if(g.mode & MODE_STOP_PROCESSING) return;
	//	goto restart;
	//}
	return;
}
//#endif // USE_LOCKED_CANDIDATES

//apply techniques prior to guessing
//useful for reducing the pencilmarks for pseudopuzzles with < 16 givens
static void fastEliminations(game &g) {
	do {
		checkForLastOccurenceInGroup(g); //the first algorithm performs internal repeating
		if(g.mode & MODE_STOP_PROCESSING) return;
	}
#ifdef USE_LOCKED_CANDIDATES
	while(FindLockedCandidates(g), (g.mode & MODE_STOP_PROCESSING));
#else
	while(0);
#endif // USE_LOCKED_CANDIDATES
}

struct pmWeight {
	int cell;
	int digit;
};

struct pmWeights {
	pmWeight w[81 * 9];	//max possible cell/values
	int size;	//the actual size with solved removed
	void init(const game &g) {
		size = 0;
		//compose a cache digit distribution within the houses
		int boxCount[9][9];
		int rowCount[9][9]; //row, digit
		int colCount[9][9];
		//clear
		for(int c = 0; c < 9; c++) {
			for(int d = 0; d < 9; d++) {
				rowCount[c][d] = colCount[c][d] = boxCount[c][d] = 0;
			}
		}
		//count
		for(int c = 0; c < 81; c++) {
			if(g.cellPossibilities[c]) { //skip solved cells
				for(int d = 0; d < 9; d++) {
					int bm = 1 << d;
					if(g.cellPossibilities[c] & bm) { //unsolved digit in the cell
						boxCount[boxByCellIndex[c]][d]++;
						rowCount[rowByCellIndex[c]][d]++;
						colCount[colByCellIndex[c]][d]++;
					}
				}
			}
		}
		//populate the non-zero weight reciprocals
		int minWeightReciprocal = 9*9*9*9*100; //worst case in empty grid
		for(int c = 0; c < 81; c++) {
			if(g.cellPossibilities[c]) { //unsolved cell
				for(int d = 0; d < 9; d++) {
					int bm = 1 << d;
					if(g.cellPossibilities[c] & bm) { //unsolved digit in the cell
						pmWeight ww;
						//pmWeight &ww = w[size];
						ww.cell = c;
						ww.digit = d;
						int weightReciprocal = BitCount[g.cellPossibilities[c]] * boxCount[boxByCellIndex[c]][d] * rowCount[rowByCellIndex[c]][d] * colCount[colByCellIndex[c]][d];
						//int weightReciprocal = 100 * (BitCount[g.cellPossibilities[c]] * boxCount[boxByCellIndex[c]][d] * rowCount[rowByCellIndex[c]][d] * colCount[colByCellIndex[c]][d])
						//	+ 10 * boxCount[boxByCellIndex[c]][d]
						//	+ BitCount[g.cellPossibilities[c]];
						if(weightReciprocal == 0 || weightReciprocal > minWeightReciprocal) {
							//ignore
							continue;
						}
						if(weightReciprocal < minWeightReciprocal) { //reset potential candidates to this
							minWeightReciprocal = weightReciprocal;
							w[0] = ww; //structure copy
							size = 1;
						}
						else {	//add this to potential candidates
							w[size++] = ww; //structure copy
						}
					}
				}
			}
		}
		//at this stage we know the minWeightReciprocal and the list of pencilmarks for that weight
		if(size > 1) { //further reduce
			//reduce to those candidates with more visible pencilmarks
			int maxAffected = 0;
			for(int i = 0; i < size; i++) {
				int nAffected = 0;
				int c = w[i].cell;
				const cellIndex * const affected = affectedCells[c];
				for(int a = 0; a < 20; a++) {
					if(g.cellPossibilities[affected[a]]) { //unsolved
						nAffected += BitCount[g.cellPossibilities[affected[a]]];
					}
				}
				if(nAffected > maxAffected) {
					maxAffected = nAffected;
					for(int j = 0; j < i; j++) { //mark all above as invalid
						w[j].cell = 99;
					}
				}
				else if(nAffected < maxAffected) {
						w[i].cell = 99;
				}
			}
			int j = 0;
			for(int i = 0; i < size; i++) {
				if(w[i].cell != 99) {
					if(i != j) {
						w[j] = w[i]; //structure copy
					}
					j++;
				}
			}
			size = j;
		}
	}
};

//Perform any solving algorithms.
//Finally make a guess and call recursively until success (solved) or failure (no solution).
static void attempt(game &g)
{
	game gg;
	cellIndex chosenCell = 0;
	int ci, nGuesses, chosenValue, cp, bc;
restart:
	checkForLastOccurenceInGroup(g); //the first algorithm performs internal repeating
	if(g.mode & MODE_STOP_PROCESSING) return;
#ifdef USE_LOCKED_CANDIDATES
	//if(g.cellsLeft < 25)
	if(g.maxSolutions != 1)
	{
		FindLockedCandidates(g); //bb_sudoku by Brian Turner
		if(g.mode & MODE_STOP_PROCESSING) return;
	}
#endif // USE_LOCKED_CANDIDATES
	//Prepare a guess

	//Findout an unsolved cell with less possibilities
	nGuesses = 10;
	//Randomizing the guess sequence causes 9.3% false improvement.
	bool fastGuess = g.knownSolution && (g.nSolutions == 0);
	ci = g.lastGuess;
	do {
		ci++;
		if(ci > 80)
			ci = 0;
		cp = g.cellPossibilities[ci];
		if(0 == cp) continue; //skip solved cells
		if((bc = BitCount[cp]) < nGuesses) {
			chosenCell = ci;
			if(2 == (nGuesses = bc) || fastGuess) //guessing a cell with 2 possibilities is OK
				break;
		}
	} while(ci != g.lastGuess);
	g.lastGuess = chosenCell;

	int restValues = g.cellPossibilities[chosenCell];
	if(fastGuess) {
		//start from the right guess
		chosenValue = g.knownSolution[chosenCell];
		if(0 == (chosenValue & restValues)) { //wrong way
			//chosenValue = restValues & -restValues;
			g.mode |= MODE_STOP_PROCESSING;
			return;
		}
	}
	else {
		//start from the rightmost bit
		chosenValue = restValues & -restValues;
	}
	for(; --nGuesses; chosenValue = restValues & -restValues) {
		restValues ^= chosenValue; //rest of the possibilities
		gg = g; //copy the solution context

		//gg.guessDepth++;
		//if(gg.maxGuessDepth && gg.guessDepth > *gg.maxGuessDepth)
		//	*gg.maxGuessDepth = gg.guessDepth;

		setDigit(gg, chosenCell, (bitmap)chosenValue);
		if(0 == (gg.mode & MODE_STOP_PROCESSING)) {
			attempt(gg);
		}
		if(gg.mode & MODE_STOP_GUESSING) {
			g.mode = gg.mode;
			g.nSolutions = gg.nSolutions;
			return;
		}
		g.nSolutions = gg.nSolutions;
		if(nGuesses == 1) { //stop working with copies of the context
			setDigit(g, chosenCell, restValues);
			if(g.mode & MODE_STOP_PROCESSING) return;
			goto restart;
		}
		//Note that the next guess will destroy the content of g->cellDigits.
	}
}

////Perform any solving algorithms.
////Finally make a guess and call recursively until success (solved) or failure (no solution).

//Set the initially known digits.
//The context is updated on 2 passes.
static inline void init(game &g, const char* in)
{
	bitmap *gkd = g.groupKnownDigits;
	bitmap *cp = g.cellPossibilities;
	//First pass: set the digits w/o updating the affected cells.
	for(int i = 0; i < 81; i++) {
		if(in[i] == 0) continue;
		//...additional input checking here...
		//setDigit(g, i, Digit2Bitmap[in[i]]);
		//...check for error here if this is the goal...

		bitmap bm = Digit2Bitmap[(unsigned int)in[i]];

		const int *ag = affectedGroups[i];
		//if the digit we are setting has been previously set for one of
		//the 3 groups the cell is member of, we are in wrong way
		if(bm & (gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]])) {
			g.mode |= MODE_STOP_PROCESSING;
			return;
		}
		cp[i] = 0; //clear the mask to exclude this cell from further processing

		if(g.cellDigits) {
			g.cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.
		}

		if(0 == --g.cellsLeft) {//lucky, 81 givens
			//g.mode |= MODE_STOP_PROCESSING;
			solutionFound(g);
			//g.nSolutions = 1;
			return;
		}

		//set the digit as solved for all 3 groups
		gkd[ag[0]] |= bm; gkd[ag[1]] |= bm; gkd[ag[2]] |= bm;
	}
	//Second pass: update the affected cells.
	//Overall performance improvement is about 9% compared to setDigit calls.
	//checkBoard(g, gkd, cp);
	int n;
	do {
		n = 0;
		for(int i = 0; i < 81; i++) {
			bitmap &bm = cp[i];
			const int *ag = affectedGroups[i];
			bitmap knowns = gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]];
			if(bm & knowns) { //there are possibilities to be cleared
				bm &= ~knowns;
				if(bm == 0) { //no possibilities for this cell
					g.mode |= MODE_STOP_PROCESSING;
					return;
				}
				if(Bitmap2Digit[bm]) { //single digit?
					gkd[ag[0]] |= bm, gkd[ag[1]] |= bm, gkd[ag[2]] |= bm;
					if(g.cellDigits) {
						g.cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.
					}
					if(0 == --g.cellsLeft) {//lucky
						solutionFound(g);
						return;
					}
					bm = 0;
					n = 1;
					//setDigit(g, i, bm);
					//if(g.mode & MODE_STOP_PROCESSING) return;
				}
			}
		}
	} while(n);
	g.lastGuess = 81 - g.cellsLeft; //randomize guessing positions
}

//Set the initially known pencilmarks.
//The context is updated on 2 passes.
static inline void init(game &g, const int* in)
{
	bitmap *gkd = g.groupKnownDigits;
	bitmap *cp = g.cellPossibilities;
	//First pass: set the digits w/o updating the affected cells.
	for(int i = 0; i < 81; i++) {
		bitmap bm = in[i] & 511;

		if(bm == 511) continue; //non-given

		if(bm == 0) { //cell w/o possibilities imediately results in invalid pseudopuzzle
			g.mode |= MODE_STOP_PROCESSING;
			return;
		}

		if(Bitmap2Digit[bm]) { //uniform given
			const int *ag = affectedGroups[i];
			//if the digit we are setting has been previously set for one of
			//the 3 groups the cell is member of, we are in wrong way
			if(bm & (gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]])) {
				g.mode |= MODE_STOP_PROCESSING;
				return;
			}
			cp[i] = 0; //clear the mask to exclude this cell from further processing

			if(g.cellDigits) {
				g.cellDigits[i] = (cellDigit)bm; //set the digit. Bit 8 is lost, translating 9 to 0.
			}

			if(0 == --g.cellsLeft) {//lucky, 81 givens
				solutionFound(g);
				return;
			}

			//set the digit as solved for all 3 groups
			gkd[ag[0]] |= bm; gkd[ag[1]] |= bm; gkd[ag[2]] |= bm;
		}
		else {
			cp[i] = bm;
		}
	}
	//Second pass: update the affected cells.
	//Overall performance improvement is about 9% compared to setDigit calls.
	//checkBoard(g, gkd, cp);
	for(int i = 0; i < 81; i++) {
		bitmap &bm = cp[i];
		const int *ag = affectedGroups[i];
		bitmap knowns = gkd[ag[0]] | gkd[ag[1]] | gkd[ag[2]];
		if(bm & knowns) { //there are possibilities to be cleared
			bm &= ~knowns;
			if(bm == 0) { //no possibilities for this cell
				g.mode |= MODE_STOP_PROCESSING;
				return;
			}
			if(Bitmap2Digit[bm]) { //single digit?
				setDigit(g, i, bm);
				if(g.mode & MODE_STOP_PROCESSING) return;
			}
		}
	}
	g.lastGuess = 81 - g.cellsLeft; //randomize guessing positions
}

extern unsigned long long solve(const char* in, const unsigned long long maxSolutions, char* out)
{
	cellDigit cellDigits[81];
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = maxSolutions;
	g.results = out;
	g.cellDigits = cellDigits;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING)) {
		attempt(g); //do the job
	}
	return g.nSolutions;
}

extern unsigned long long solve(const int* gridBM, const char* in, const unsigned long long maxSolutions)
{
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = maxSolutions;
	g.knownSolution = gridBM;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING)) {
		attempt(g); //do the job
	}
	return g.nSolutions;
}

extern unsigned long long solve(const int* gridBM, const char* in, const unsigned long long maxSolutions, char* out)
{
	cellDigit cellDigits[81];
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = maxSolutions;
	g.knownSolution = gridBM;
	g.results = out;
	g.cellDigits = cellDigits;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING)) {
		attempt(g); //do the job
	}
	return g.nSolutions;
}

extern unsigned long long solve(const char* in, const unsigned long long maxSolutions)
{
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = maxSolutions;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING)) {
		attempt(g); //do the job
	}
	return g.nSolutions;
}

unsigned long long checkNoSingles(const char* in)
{
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = 1;
	init(g, in); //set all known digits
	int unknowns = g.cellsLeft;
	if(g.mode & MODE_STOP_PROCESSING) //invalid or solved by direct eliminations
		return 0;
	//game gg = g; //copy pencilmarks
	checkForLastOccurenceInGroup(g); //the first algorithm performs internal repeating
	if(g.mode & MODE_STOP_PROCESSING) //invalid or solved by singles
		return 0;
	if(unknowns != g.cellsLeft)
		return 0;
	//if(memcmp(g.cellPossibilities, gg.cellPossibilities, sizeof(g.cellPossibilities) * sizeof(g.cellPossibilities[0]))) //elimination w/o guessing
	//	return 0;
	FindLockedCandidates(g);
	if(g.mode & MODE_STOP_PROCESSING) //invalid or solved w/o guessing
		return 0;
	if(unknowns != g.cellsLeft)
		return 0;
	//if(memcmp(g.cellPossibilities, gg.cellPossibilities, sizeof(g.cellPossibilities) * sizeof(g.cellPossibilities[0]))) //elimination w/o guessing
	//	return 0;

	return 1; //remove to make this function universal. Currently it is called after obtaining the only solution.
}

extern bool isDifficultD(const char *puzzle) { //return whether any given constrains any non-given
	char puz[81];
	memcpy(puz, puzzle, 81);
	char sol[81];
	if(1 != solve(puz, 1, sol))
		return false; //invalid puzzle
	//if a given can be replaced with another given and that lead to valid solution(s) then it is not redundant
	for(int testGivenPosition = 0; testGivenPosition < 81; testGivenPosition++) { //loop over givens
		//reduce the givens by one and compose partially solved puzzle with given at testGivenPosition removed
		if(puzzle[testGivenPosition] == 0)
			continue; //not a given
		puz[testGivenPosition] = 0; //remove the given
		game g = defaultGame; //start from a copy of the empty game
		g.maxSolutions = 1;
		init(g, puz); //set all known digits
		if(g.mode & MODE_STOP_PROCESSING) //invalid or solved by direct eliminations
			return false; //puzzle solves even with this given removed
		fastEliminations(g);
		if(g.mode & MODE_STOP_PROCESSING) //invalid or solved by singles
			return false; //puzzle solves even with this given removed
		if(0 == g.cellPossibilities[testGivenPosition] & ~Digit2Bitmap[(int)puzzle[testGivenPosition]])
			return false; //the removed given is solved (i.e. it is redundant)
		//test whether some of the non-givens is resolved
		for(int testNonGivenCell = 0; testNonGivenCell < 81; testNonGivenCell++) { //loop over non-given cells
			if(puzzle[testNonGivenCell] != 0)
				continue; //not a non-given
			if(0 == g.cellPossibilities[testNonGivenCell]) //a solved cell from the non-givens
				return false;
		}
		//test whether alternative solution exists for each of the non-givens
		for(int testNonGivenCell = 0; testNonGivenCell < 81; testNonGivenCell++) { //loop over non-given cells
			if(puzzle[testNonGivenCell] != 0)
				continue; //not a non-given
			game gg = g; //clone the game
			gg.cellPossibilities[testNonGivenCell] &= ~Digit2Bitmap[(int)sol[testNonGivenCell]]; //remove the known solution possibility
			if(1 == BitCount[gg.cellPossibilities[testNonGivenCell]]) {
				//after removal of the known solution possibility, only one other remains
				setDigit(gg, testNonGivenCell, gg.cellPossibilities[testNonGivenCell]);
			}
			if((gg.mode & MODE_STOP_PROCESSING) == 0) {
				attempt(gg); //check for any solution
			}
			if(0 == gg.nSolutions) {
				//with removed given at position testGivenPosition, a solution with value at position testNonGivenCell different than the original solution doesn't exist. Filter doesn't match.
				return false;
			}
			//the removed given constrains the tested cell, even indirectly. Continue with next cell.
		}
		puz[testGivenPosition] = puzzle[testGivenPosition]; //restore the given
	}
	//for any given removed, any of the non-givens could be resolved to alternative value. Filter matches.
	return true;
}

extern int solverIsIrreducibleByProbing(char *puz) { //return nSol for irreducible puzzles, else 0
	//if a given can be replaced with another given and that lead to valid solution(s) then it is not redundant
	for(int red = 0; red < 81; red++) {
		if(puz[red] == 0)
			continue; //not a given
		int redValue = puz[red];
		//reduce the givens by composing partially solved puzzle with given at red removed
		puz[red] = 0;

		game g = defaultGame; //start from a copy of the empty game
		g.maxSolutions = 1;
		init(g, puz); //set all known digits
		if(g.mode & MODE_STOP_PROCESSING) //invalid or solved by direct eliminations
			//return (int)g.nSolutions;
			goto redundantFound; //with this given removed, the puzzle solves w/o guessing.
		fastEliminations(g);
		if(g.mode & MODE_STOP_PROCESSING) //invalid or solved by singles
			//return (int)g.nSolutions;
			goto redundantFound; //with this given removed, the puzzle solves w/o guessing.
		{
		int pm = g.cellPossibilities[red] & ~Digit2Bitmap[redValue];
		if(pm) {
			//unsolved cell
			int valueBM;
			while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
				pm ^= valueBM; //clear this bit from the cloned pencilmarks
				game gg = g; //clone the game
				//set the new given
				setDigit(gg, red, valueBM);
				if((gg.mode & MODE_STOP_PROCESSING) == 0) {
					attempt(gg); //check for any solution
				}
				if(gg.nSolutions) {
					//not redundant, don't probe with more values
					goto nextRedundant;
				}
			}
		}
		}
redundantFound:
		//given at position red is redundant
		puz[red] = redValue;
		return 0;
nextRedundant:
		puz[red] = redValue;
	}
	return 1;


//	for(int red = 0; red < 81; red++) {
//		if(puz[red] == 0)
//			continue; //not a given
//		int redValue = puz[red];
//		//TODO: reduce the givens by composing partially solved puzzle with given at red removed (done but doesn't help)
//		puz[red] = 0;
//
//		for(int probeRedValue = 1; probeRedValue < 10; probeRedValue++) {
//			if(probeRedValue == redValue) {
//				continue;
//			}
//			puz[red] = probeRedValue;
//			if(solve(puz, 1)) {
//				//not redundant, don't probe with more values
//				goto nextRedundant;
//			}
//		}
//		//given at position red is redundant
//		puz[red] = redValue;
//		return 0;
//nextRedundant:
//		puz[red] = redValue;
//	}
//	return 1;
}

extern int solverIsIrreducible(char *puz) {
	for(int red = 0; red < 81; red++) {
		if(puz[red] == 0)
			continue; //not a given
		int redValue = puz[red];
		puz[red] = 0;
		if(solve(puz, 2) == 1) {
			//given at position red is redundant
			puz[red] = redValue;
			return 0;
		}
		puz[red] = redValue;
	}
	return 1;
}

extern int solverPlus1(const char* in, char* out, const bool redundancyCheck, const bool unique) { //apply fast {+1} to in and store all to out
	int ret = 0;
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = 1;
	if(unique)
		g.maxSolutions = 2;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING))
		fastEliminations(g);
	if(g.mode & MODE_STOP_PROCESSING) {
		if(g.nSolutions) {
			//solved with simple techniques
			return 0;
			//if(redundancyCheck) {
			//	//any additional clue is redundant
			//	return 0;
			//}
			//else {
			//	//return original puzzle (it is matter of definition whether to return original or add all valid additional clues, one per position)
			//	memcpy(&out[0], in, 81);
			//	return 1;
			//}
		}
		//no solution
		return 0;
	}
	for(int pos = 0; pos < 81; pos++) {
		int pm = g.cellPossibilities[pos];
		if(pm) {
			//unsolved cell
			int valueBM;
			while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
				pm ^= valueBM; //clear this bit from the pencilmarks
				game gg = g; //clone the game
				//set the new given
				setDigit(gg, pos, valueBM);
				if((gg.mode & MODE_STOP_PROCESSING) == 0) {
					attempt(gg); //check for any solution
				}
				if(gg.nSolutions == 0) {
					continue; //check the next value for this cell
				}
				if(unique && gg.nSolutions > 1) {
					continue;
				}
				//validity & unique tests passed. Now add the puzzle to out.
				char *res = &out[ret * 81];
				memcpy(res, in, 81);
				res[pos] = Bitmap2Digit[valueBM];
				if(redundancyCheck) {
					if(solverIsIrreducibleByProbing(&out[ret * 81])) { //irreducible
						ret++;
					}
				}
				else {
					ret++;
				}
			}
		}
	}
	return ret;
}

extern int solverPlus2(char* in, char* out) { //apply fast {+2} to in and store uniques (possibly non-minimals) to out
	int ret = 0;
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = 2;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING))
		fastEliminations(g);
	if(g.mode & MODE_STOP_PROCESSING) {
		//solved with +0 clues or no solution
		return 0;
	}
	for(int pos = 0; pos < 81 - 1; pos++) {
		int pm = g.cellPossibilities[pos];
		if(pm) {
			//unsolved cell
			int valueBM;
			while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
				pm ^= valueBM; //clear this bit from the pencilmarks
				game gg = g; //clone the game
				//set the new given
				setDigit(gg, pos, valueBM);
				if((gg.mode & MODE_STOP_PROCESSING) == 0) {
					//perform additional eliminations
					fastEliminations(gg);
				}
				if(gg.mode & MODE_STOP_PROCESSING) {
					//solved at +1 stage, therefore +2 is redundant, or invalid +1
					continue; //check the next value for this cell
				}
				game g2 = gg;
				if((gg.mode & MODE_STOP_PROCESSING) == 0) {
					attempt(gg); //check for any solution
				}
				if(gg.nSolutions < 2) {
					//invalid or unique after +1
					continue; //check the next value for this cell
				}
				//+1 tests passed. Now add the next clue.
				for(int pos2 = pos + 1; pos2 < 81; pos2++) {
					int pm2 = g2.cellPossibilities[pos2];
					if(pm2) {
						//unsolved cell
						int valueBM2;
						while((valueBM2 = (pm2 & -pm2))) { //take the rightmost nonzero bit
							pm2 ^= valueBM2; //clear this bit from the pencilmarks
							game gg2 = g2; //clone the game
							//set the new given
							setDigit(gg2, pos2, valueBM2);
							if((gg2.mode & MODE_STOP_PROCESSING) == 0) {
								attempt(gg2); //check for any solution
							}
							if(gg2.nSolutions != 1) {
								//zero or multiple solutions at +2 stage
								continue; //check the next value for this cell
							}
							//+2 tests passed. Now add the puzzle to out.
							memcpy(&out[ret * 81], in, 81);
							out[ret * 81 + pos] = Bitmap2Digit[valueBM];	//+1
							out[ret * 81 + pos2] = Bitmap2Digit[valueBM2];	//+2
							ret++;
						}
					}
				}
			}
		}
	}
	return ret;
}

extern int solverPlus1Unique(char* in, char* out) { //apply fast {+1} to in and store all to out
	int ret = 0;
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = 1;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING))
		checkForLastOccurenceInGroup(g);
	if(0 == (g.mode & MODE_STOP_PROCESSING))
		FindLockedCandidates(g);
	if((g.mode & MODE_STOP_PROCESSING) || g.nSolutions)
		return 0; //solved
	for(int pos = 0; pos < 81; pos++) {
		int pm = g.cellPossibilities[pos];
		if(pm) {
			//unsolved cell
			int valueBM;
			while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
				pm ^= valueBM; //clear this bit from the pencilmarks
				game gg = g; //clone the game
				gg.maxSolutions = 2;
				//set the new given
				setDigit(gg, pos, valueBM);
				if((gg.mode & MODE_STOP_PROCESSING) == 0) {
					attempt(gg); //check for any solution
				}
				if(gg.nSolutions != 1) {
					continue; //check the next value for this cell
				}
				//uniqueness test passed
				//compose the puzzle candidate
				char *res = &out[ret * 81];
				memcpy(res, in, 81);
				res[pos] = Bitmap2Digit[valueBM];
				//check for redundancy
				for(int red = 0; red < 81; red++) {
					if(res[red] == 0)
						continue; //not a given
					int redValue = res[red];
					res[red] = 0;
					if(2 != solve(res, 2))
						goto nextValue; //still unique solution after clue at red removal
					res[red] = redValue;
				}
				//tests passed. Now add the puzzle to out.
				ret++;
nextValue:
				;
			}
		}
	}
	return ret;
}

struct solverRelab {
	int patternSize;
	int patternPositions[81];
	char digits[81];
	bool minimals;
	bool unique;
	bool nosingles;
	const char *in;
	int (*callBack)(const char *puz, void *context);
	void *context;
};

static int solvePattern(solverRelab &rd, const game &g, const int maxDiff, const int clueNumber);
static int solvePattern(solverRelab &rd, const game &g, const int maxDiff, const int clueNumber, const int valueBM) {
	int nextClueNumber = clueNumber + 1;
	game gg = g;
	if(g.cellPossibilities[rd.patternPositions[clueNumber]] == 0)
		return 0; //solved => redundant
	if(g.cellPossibilities[rd.patternPositions[clueNumber]] == valueBM)
		return 0; //single possibility => redundant
	gg = g; //clone the game
	setDigit(gg, rd.patternPositions[clueNumber], valueBM);
	//if(nextClueNumber < patternSize) { //reduce the possibilities for the next iterations
	//	if((gg.mode & MODE_STOP_PROCESSING) == 0)
	//		checkForLastOccurenceInGroup(gg);
	//	//if(0 == (gg.mode & MODE_STOP_PROCESSING) /*&& gg.cellsLeft < 60*/)
	//	//	FindLockedCandidates(gg);
	//}
	if((gg.mode & MODE_STOP_PROCESSING) == 0) { //no contradiction
		game ggg = gg;
		ggg.maxSolutions = 1;
		attempt(ggg); //check for any solution
		if(ggg.nSolutions == 0)
			return 0; //no solution
	}
	else { //solved or contradiction
		if(gg.nSolutions == 0) // contradiction
			return 0;
		if(rd.minimals && gg.nSolutions == 1 && nextClueNumber < rd.patternSize) // rest of the clues are redundant
			return 0;
	}
	rd.digits[rd.patternPositions[clueNumber]] = Bitmap2Digit[valueBM];
	if(maxDiff == 0) {
		//no more recursive calls will be done
		//rest of the clues must be the same as in the template
		//populate rest of the clues from the template
		bool requiresDoubleCheck = false;
		for(int i = nextClueNumber; i < rd.patternSize; i++) {
			if(gg.cellPossibilities[rd.patternPositions[i]] == 0) { //solved cell
				if(rd.minimals)
					return 0; //redundant if the same value, or no solution if different value
				//must be solved from scratch to see whether the solved clue matches the given
				requiresDoubleCheck = true;
			}
			setDigit(gg, rd.patternPositions[i], Digit2Bitmap[(unsigned int)rd.in[rd.patternPositions[i]]]);
			if((gg.mode & MODE_STOP_PROCESSING) && gg.nSolutions == 0)
				return 0; //no solution
			rd.digits[rd.patternPositions[i]] = rd.in[rd.patternPositions[i]];
		}
		if((gg.mode & MODE_STOP_PROCESSING) == 0)
			attempt(gg); //check for any solution
		if(gg.nSolutions == 0)
			return 0; //no solution
		if(rd.unique && gg.nSolutions > 1) //???
			return 0; //multiple solutions
		nextClueNumber = rd.patternSize;
		if(requiresDoubleCheck) {
			//the puzzle isn't minimal but we don't know if the redundant (solved) values match the givens
			if(0 == solve(rd.digits, 1))
				return 0;
		}
	}
	if(nextClueNumber == rd.patternSize) { //no more clues
		if(rd.unique) { //check for uniqueness
			if(gg.nSolutions > 1)
				return 0;
			if(gg.nSolutions == 1) {
				gg.mode = MODE_SOLVING;
			}
			attempt(gg);
			if(gg.nSolutions != 1)
				return 0; //multiple or no solutions
			//check for redundancy
			if(rd.minimals) {
				for(int i = 0; i < nextClueNumber; i++) {
					int oldValue = rd.digits[rd.patternPositions[i]];
					bool found = false;
					//TODO: init a game instance with clue at patternPositions[i] cleared and check whether setdigit()+attempt() is faster then solve()
					//trying all 8 possibilities works faster than clearing the given and checking for a second solution
					for(int v = 1; v <= 9; v++) {
						if(v == oldValue)
							continue;
						rd.digits[rd.patternPositions[i]] = v;
						if(solve(rd.digits, 1)) {
							found = true;
							break;
						}
					}
					rd.digits[rd.patternPositions[i]] = oldValue;
					if(!found)
						return 0; //redundant clue digits[patternPositions[i]]
				}
			}
		}
		char out[81];
		for(int i = 0; i < 81; i++) {
			out[i] = 0;
		}
		for(int i = 0; i < rd.patternSize; i++) {
			out[rd.patternPositions[i]] = rd.digits[rd.patternPositions[i]];
		}
		if(rd.nosingles && checkNoSingles(out) == 0) //filter out simple puzzles
			return 0;
		rd.callBack(out, rd.context);
		return 1; //last clue, valid choice
	}
	//recurse
	return solvePattern(rd, gg, maxDiff, nextClueNumber);
}
static int solvePattern(solverRelab &rd, const game &g, const int maxDiff, const int clueNumber) {
	int pm = g.cellPossibilities[rd.patternPositions[clueNumber]];
	if(pm == 0) return 0; //previously solved => redundant
	//traverse trough all possibilities for the clueNumber
	int valueBM;
	int ret = 0;
	if(maxDiff) {
		while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
			pm ^= valueBM; //clear this bit from the pencilmarks
			int md = maxDiff;
			if((int)Bitmap2Digit[valueBM] != rd.in[rd.patternPositions[clueNumber]]) {
				md--; //one more difference
			}
			ret += solvePattern(rd, g, md, clueNumber, valueBM);
		}
	}
	else {
		return solvePattern(rd, g, 0, clueNumber, Digit2Bitmap[(unsigned int)rd.in[rd.patternPositions[clueNumber]]]);
	}
	return ret;
}
///populate the given pattern in all possible ways up to maxDiff differences
extern int solverRelabel(const char* in, const int maxDiff, const bool minimals, const bool unique, const bool nosingles, int (*callBack)(const char *puz, void *context), void *context) {
	//int ret = 0;
	solverRelab rd;
	rd.minimals = minimals;
	rd.unique = unique;
	rd.nosingles = nosingles;
	rd.in = in;
	rd.callBack = callBack;
	rd.context = context;
	game g = defaultGame; //start from a copy of the empty game
	if(!rd.unique) {
		g.maxSolutions = 1; //else default 2
	}
	rd.patternSize = 0;
	for(int i = 0; i < 81; i++) {
		rd.digits[i] = 0;
		if(rd.in[i]) {
			rd.patternPositions[rd.patternSize] = i;
			rd.patternSize++;
		}
	}
	//traverse trough all possibilities for the first clue
	return solvePattern(rd, g, maxDiff, 0);
}

static int solverPattern(game &g, char* in, const int nClues, const int* cluePositions, int (*puzFound)(void *context, const char* puz)) {
	if(0 == (g.mode & MODE_STOP_PROCESSING)) {
		checkForLastOccurenceInGroup(g);
		if(0 == (g.mode & MODE_STOP_PROCESSING)) {
			FindLockedCandidates(g);
			if(0 == (g.mode & MODE_STOP_PROCESSING)) {
				if(g.nSolutions) {
					return 0; //discard puzzles with less givens
					//(*puzFound)(NULL, in); //notify for unique solution with less givens than in template
					//return 1;
				}
				//valid clue combination, continue with next clue
			}
			else {
				return 0; //no solution
			}
		}
		else {
			return 0; //no solution
		}
	}
	else {
		return 0; //no solution
	}
	int pos = *cluePositions;
	int pm = g.cellPossibilities[pos];
	int ret = 0;
	if(pm) {
		//unsolved cell
		int valueBM;
		while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
			pm ^= valueBM; //clear this bit from the pencilmarks
			game gg = g; //clone the game
			//set the new given
			setDigit(gg, pos, valueBM);
			if(nClues - 1) {
				in[pos] = Bitmap2Digit[valueBM];
				ret += solverPattern(gg, in, nClues - 1, cluePositions + 1, puzFound);
			}
			else {
				gg.maxSolutions = 2;
				attempt(gg); //check for single solution at the last step
				if(gg.nSolutions != 1) {
					continue;
				}
				in[pos] = Bitmap2Digit[valueBM];
				(*puzFound)(NULL, in); //notify for the solution
				ret++;
			}
		}
	}
	in[pos] = 0;
	return ret;
}

extern int solverPattern(char* in, const int nClues, const int* cluePositions, int (*puzFound)(void *context, const char* puz)) {
	game g = defaultGame; //start from a copy of the empty game
	g.maxSolutions = 1;
	init(g, in); //set all known digits
	if(0 == (g.mode & MODE_STOP_PROCESSING))
		checkForLastOccurenceInGroup(g);
	if(0 == (g.mode & MODE_STOP_PROCESSING))
		FindLockedCandidates(g);
	if(0 == (g.mode & MODE_STOP_PROCESSING)) {
		if(g.nSolutions) {
			(*puzFound)(NULL, in); //notify for the solution
			return 1;
		}
	}
	else {
		return 0; //no solution
	}
	int pos = *cluePositions;
	int pm = g.cellPossibilities[pos];
	int ret = 0;
	if(pm) {
		//unsolved cell
		int valueBM;
		while((valueBM = (pm & -pm))) { //take the rightmost nonzero bit
			pm ^= valueBM; //clear this bit from the pencilmarks
			game gg = g; //clone the game
			//set the new given
			setDigit(gg, pos, valueBM);
			in[pos] = Bitmap2Digit[valueBM];
			ret += solverPattern(gg, in, nClues - 1, cluePositions + 1, puzFound);
		}
	}
	in[pos] = 0;
	return ret;
}

