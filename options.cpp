#define _CRT_SECURE_NO_DEPRECATE

#include <time.h>
#include "options.h"

options opt = options();

options::options() {
	scanOpt = NULL;
	solveOpt = NULL;
	nhbOpt = NULL;
	uaOpt = NULL;
	similarOpt = NULL;
	patternOpt = NULL;
	templateOpt = NULL;
	verbose = false;

	//SET PREFERENCES
	//opt->noPOSIX(); //do not check for POSIX style character options
	//anyopt.setVerbose(); // print warnings about unknown options
	//anyopt.autoUsagePrint(true); // print usage for bad options

	//SET THE USAGE/HELP and THE OPTION STRINGS/CHARACTERS
	anyopt.addUsage("");
	anyopt.addUsage("Usage:");
	anyopt.addUsage("" );
	anyopt.addUsage(" -h  --help          Print this help");
	anyopt.setCommandFlag("help", 'h');
	anyopt.addUsage(" -v  --version       Print version");
	anyopt.setCommandFlag("version", 'v');
	anyopt.addUsage(" --verbose           Add plenty of info during execution");
	anyopt.setFlag("verbose");

	anyopt.addUsage(" --scan              Scan grid(s) for minimal puzzles");
	anyopt.setCommandFlag("scan");
	anyopt.addUsage("   --gridlist <file>     Batch scan the grids in <file>");
	anyopt.setOption("gridlist");
	anyopt.addUsage("   --subgridlist <file>  Scan each subgrid in <file>");
	anyopt.setOption("subgridlist");
	anyopt.addUsage("   --isubgridlist <file> Scan each subgrid completion in <file>");
	anyopt.setOption("isubgridlist");
	anyopt.addUsage("   --gridfile <file>     Scan single grid from <file>.txt");
	anyopt.setOption("gridfile");
	anyopt.addUsage("   --batch               Batch scan the grids from stdin");
	anyopt.setFlag("batch");
	anyopt.addUsage("   --numclues <n>        Search for puzzles with up to n clues (17)");
	anyopt.setOption("numclues");
	anyopt.addUsage("   --stopatfirst         Stop after the first puzzle is found");
	anyopt.setFlag("stopatfirst");
	anyopt.addUsage("   --minimals            Skip non-minimal puzzles");
	anyopt.setFlag("minimals");
	anyopt.addUsage("   --shorter             Don't expand puzzles with < n givens to non-minimals (not impl.)");
	anyopt.setFlag("shorter");
	anyopt.addUsage("   --cluemask <mask>     81-char mask forcing 0=non-given, 1=given");
	anyopt.setOption("cluemask");
	anyopt.addUsage("   --fixband             Fix one band at a time (experimental)");
	anyopt.setFlag("fixband");
	anyopt.addUsage("   --fix1digit           Fix one digit at a time (experimental)");
	anyopt.setFlag("fix1digit");
	anyopt.addUsage("   --fix2digits          Fix 2 digits at a time");
	anyopt.setFlag("fix2digits");
	anyopt.addUsage("   --fix3digits          Fix 3 digits at a time (experimental)");
	anyopt.setFlag("fix3digits");
	anyopt.addUsage("   --fix4digits          Fix 4 + 5 digits (experimental)");
	anyopt.setFlag("fix4digits");
	anyopt.addUsage("   --fix1box             Fix one box at a time (experimental)");
	anyopt.setFlag("fix1box");
	anyopt.addUsage("   --fix2boxes           Fix 2 boxes at a time (experimental)");
	anyopt.setFlag("fix2boxes");
	anyopt.addUsage("   --fixauto             Auto fix or perform an exhaustive scan (not impl.)");
	anyopt.setFlag("fixauto");
	anyopt.addUsage("   --exhaustive          Perform an exhaustive scan, default");
	anyopt.setFlag("exhaustive");
	anyopt.addUsage("   --storepseudos        Store pseudopuzzles with fixed clues");
	anyopt.setFlag("storepseudos");
	anyopt.addUsage("   --fast17              Batch speculative scan for 17s (99%)");
	anyopt.setFlag("fast17");
	anyopt.addUsage("   --veryfast17          Batch speculative scan for 17s (87%)");
	anyopt.setFlag("veryfast17");
	//anyopt.addUsage("   --knownpuzzles <file> Allow comparison to already known puzzles");
	//anyopt.setOption("knownpuzzles");
	//anyopt.addUsage("   --skipknowns          Known puzzles are excluded from the output");
	//anyopt.setFlag("skipknowns");
	//anyopt.addUsage("   --appendunknowns      Appended newly found to the file with knowns");
	//anyopt.setFlag("appendunknowns");
	anyopt.addUsage("   --mcnuasizelimit <n>  Use UA of size up to <n> for MCN (13)");
	anyopt.setOption("mcnuasizelimit");
	anyopt.addUsage("   --mcnnoautoua         Prevent auto UA generation calculating MCN");
	anyopt.setFlag("mcnnoautoua");
	anyopt.addUsage("   --progressseconds <n> Prints progress at every <n> seconds (30)");
	anyopt.setOption("progressseconds");
	anyopt.addUsage("   --bandcompletions     Prints 416 band minimal clue completions");
	anyopt.setFlag("bandcompletions");
	anyopt.addUsage("   --scanfixedbandstack  Scans grid given by puzzle for alternative givens of the same size with one band+stack fixed");
	anyopt.setFlag("scanfixedbandstack");
	anyopt.addUsage("   --scanunav            Scans for puzzles large UA sets given by completion");
	anyopt.setFlag("scanunav");
	anyopt.addUsage("   --fastscan            Scans stdin for puzzles using McGuire's method");
	anyopt.setFlag("fastscan");

	anyopt.addUsage(" --solve             Solve puzzles");
	anyopt.setCommandFlag("solve");
	//anyopt.addUsage("   --puzzles <file>      Puzzle list");
	//anyopt.setOption("puzzles");
	anyopt.addUsage("   --groupbygrid         Normalized puzzles grouped by solution");
	anyopt.setCommandFlag("groupbygrid");
	anyopt.addUsage("     --gridsonly             Prints ordered solution grids w/o duplicates");
	anyopt.setFlag("gridsonly");
	anyopt.addUsage("   --minimals            Check puzzles for minimality");
	//anyopt.setFlag("minimals");
	anyopt.addUsage("   --count               Print only the solution count");
	anyopt.setFlag("count");
	anyopt.addUsage("     --maxsolutioncount <n>  Solves up to <n>-th solution (INT_MAX)");
	anyopt.setOption("maxsolutioncount");
	anyopt.addUsage("   --rate                Print the rating");
	anyopt.setFlag("rate");
	anyopt.addUsage("   --backdoor            Print backdoors & level");
	anyopt.setFlag("backdoor");

	anyopt.addUsage(" --nhb               Process neighbour grids");
	anyopt.setCommandFlag("nhb");
	anyopt.addUsage("   --gridfile <file>     Base grid <file>.txt");
	//anyopt.setOption("gridfile");
	anyopt.addUsage("   --knownpuzzles <file> Allow comparison to already known puzzles (not impl.)");
	anyopt.setOption("knownpuzzles");
	anyopt.addUsage("   --clusterize          Clusterize at num UA distance");
	anyopt.setFlag("clusterize");

	anyopt.addUsage(" --unav              Process unavoidable sets");
	anyopt.setCommandFlag("unav");
	anyopt.addUsage("   --gridfile <file>     Base grid <file>.txt");
	//anyopt.setOption("gridfile");
	anyopt.addUsage("   --unav4               Find 4-digit unavoidables");
	anyopt.setFlag("unav4");
	anyopt.addUsage("   --unav5               Find 5-digit unavoidables");
	anyopt.setFlag("unav5");
	anyopt.addUsage("   --unav12              Find up to 12-digit McGuire's unavoidables");
	anyopt.setFlag("unav12");
	//anyopt.addUsage("   --unavmorph           Find unavoidables which premute to same grid");
	//anyopt.setFlag("unavmorph");
	anyopt.addUsage("   --unavrandom          Find from random pseudo puzzles");
	anyopt.setFlag("unavrandom");
	anyopt.addUsage("     --attempts <n>          Attempt <n> pseudo puzzles (10000)");
	anyopt.setOption("attempts");
	anyopt.addUsage("     --unknowns <n>          Use pseudo puzzles with <n> unknowns (54)");
	anyopt.setOption("unknowns");
	anyopt.addUsage("   --maxuasize <n>       Ignore UA larger than <n> (81)");
	anyopt.setOption("maxuasize");
	anyopt.addUsage("   --minuasize <n>       Ignore UA smaller than <n> (0)");
	anyopt.setOption("minuasize");
	anyopt.addUsage("   --minvalency <n>      Ignore UA with valency smaller than <n> (0)");
	anyopt.setOption("minvalency");
	anyopt.addUsage("   --subcanon            Print canonicalized UA patterns");
	anyopt.setFlag("subcanon");
	anyopt.addUsage("   --mspuzzles <file>    Find UA solving pseudopuzzles in <file>");
	anyopt.setOption("mspuzzles");
	anyopt.addUsage("   --minus1              Find UA applying {-1} to valid puzzles");
	anyopt.setFlag("minus1");
	anyopt.addUsage("     --findvalency           Find also the maximal valency");
	anyopt.setFlag("findvalency");

	anyopt.addUsage(" --similar           Find similar puzzles");
	anyopt.setCommandFlag("similar");
	anyopt.addUsage("   --relabel <depth>     Relabel up to <depth> givens in stdin puzzles");
	anyopt.setOption("relabel");
	anyopt.addUsage("     --minimals              Minimal multi-sol. puzzles, otherwise unique");
	//anyopt.setFlag("minimals");
	anyopt.addUsage("     --unique                Single-solution puzzles only");
	anyopt.setFlag("unique");
	anyopt.addUsage("     --nosingles             Ignore puzzles solved by singles");
	anyopt.setFlag("nosingles");
	anyopt.addUsage("   --minus1plus <file>   Apply {-1+n} to puzzles in <file>.newms.txt");
	anyopt.setOption("minus1plus");
	anyopt.addUsage("   --puzzles <file>      Puzzle list");
	//anyopt.setOption("puzzles");
	anyopt.addUsage("   --knownpuzzles <file> Exclude the known puzzles");
	//anyopt.setOption("knownpuzzles");
	anyopt.addUsage("   --minimals            Check puzzles for minimality");
	//anyopt.setFlag("minimals");
	anyopt.addUsage("   --minus1              Apply {-1} to puzzles");
	//anyopt.setFlag("minus1");
	anyopt.addUsage("     --mspuzzles <file>      Append pseudopuzzles to <file>");
	//anyopt.setOption("mspuzzles");
	anyopt.addUsage("   --minusandup <xy>     Apply {-x} then {+1..y} to puzzleset from stdin and ouput unique minimals");
	anyopt.setOption("minusandup");
	anyopt.addUsage("   --twins               Check puzzleset from stdin for unknown twins");
	anyopt.setFlag("twins");
	anyopt.addUsage("     --subcanon              Canonicalize input puzzles");
	//anyopt.setFlag("subcanon");
	anyopt.addUsage("     --minimals              Skip non-minimal twins");
	anyopt.addUsage("   --cousins             Generate puzzles with same pattern from similar grids");
	anyopt.setFlag("cousins");
	anyopt.addUsage("   --subcanon            Canonicalize puzzles to mspuzzles");
	//anyopt.setFlag("subcanon");
	anyopt.addUsage("     --puzzles <file>    Read <file>");
	anyopt.setOption("puzzles");
	anyopt.addUsage("     --mspuzzles <file>  Output to ordered and duplicates free <file>");
	//anyopt.setOption("mspuzzles");
	anyopt.addUsage("   --removeredundant     Removes redundant clues");
	anyopt.setFlag("removeredundant");
	anyopt.addUsage("   --invert <grid|.|a|d> Inverts clues of puzzle using given/any/all/different completions");
	anyopt.setOption("invert");
	anyopt.addUsage("   --plus1               Apply {+1} to puzzles");
	anyopt.setFlag("plus1");
	anyopt.addUsage("     --subcanon              Canonicalize the puzzles");
	anyopt.addUsage("     --minimals              Minimal puzzles only, incl. multi-solution");
	anyopt.addUsage("     --unique                Single-solution puzzles only");
	//anyopt.setFlag("unique");
	anyopt.addUsage("   --plus2               Apply {+2} to puzzles and print uniques");
	anyopt.setFlag("plus2");
	anyopt.addUsage("   --bandminlex          Prints 6 band minlex transformations for valid puzzles");
	anyopt.setFlag("bandminlex");
	anyopt.addUsage("   --clusterize          Clusterize the puzzles");
	//anyopt.setFlag("clusterize");

	anyopt.addUsage(" --pattern           Process pattern");
	anyopt.setCommandFlag("pattern");
	anyopt.addUsage("   --redundancy          Prints redundancy level for patterns from stdin");
	anyopt.setFlag("redundancy");
	anyopt.addUsage("   --statistics          Prints statistics for patterns from stdin (not impl.)");
	anyopt.setFlag("statistics");
	anyopt.addUsage("   --enumerate <pattern> Prints puzzles for given pattern");
	anyopt.setOption("enumerate");
	anyopt.addUsage("     --subcanon              Print canonicalized ED puzzles");
	//anyopt.setFlag("subcanon");
	anyopt.addUsage("     --fixclues <puzzle>     Prints only puzzles containing this subpuzzle");
	anyopt.setOption("fixclues");
	anyopt.addUsage("   --scanfor <pattern>   Prints minimal puzzles with given pattern in grids from stdin");
	anyopt.setOption("scanfor");
	anyopt.addUsage("   --patcanon            Prints canonicalized unique patterns from stdin");
	anyopt.setFlag("patcanon");
	anyopt.addUsage("   --settle <pattern>    Converts non-minimal puzzles from stdin to minimals in pattern");
	anyopt.setOption("settle");
	anyopt.addUsage("   --pg depth,er,ER,ep,EP,ed,ED,maxPasses,noSingles  Re-labels stdin to stdout, related to Patterns Game");
	anyopt.addUsage("   --pg s<pattern> < grids.txt > puzzles.txt          Scans grids for puzzles having the given pattern");
	anyopt.setOption("pg");

	anyopt.addUsage(" --template              Template/rookery related commands");
	anyopt.setCommandFlag("template");
	anyopt.addUsage("   --get2templates           Prints identity for all 181 2-tepmplate classes (not impl.)");
	anyopt.setFlag("get2templates");
	anyopt.addUsage("   --get2rookeries           Prints identity for all 170 2-rookery classes (not impl.)");
	anyopt.setFlag("get2rookeries");
	anyopt.addUsage("   --get999911110            Prints identity for all 170 2-rookery classes");
	anyopt.setFlag("get999911110");
	anyopt.addUsage("   --r4tot4                  Prints all 4-templates for a given list of 4-rookeries");
	anyopt.setFlag("r4tot4");
	anyopt.addUsage("   --r4tot5                  Prints all 5-templates for a given list of 4-rookeries");
	anyopt.setFlag("r4tot5");
	anyopt.addUsage("");

	//by default all  options  will be checked on the command line and from option/resource file
	//anyopt.setFlag("help", 'h');   //a flag (takes no argument), supporting long and short form 
	//anyopt.setOption("size", 's'); //an option (takes an argument), supporting long and short form
	//anyopt.setOption("name");      //an option (takes an argument), supporting only long form
	//anyopt.setFlag('c');           //a flag (takes no argument), supporting only short form

	//for options that will be checked only on the command and line not in option/resource file
	//anyopt.setCommandFlag("zip" , 'z'); //a flag (takes no argument), supporting long and short form

	//for options that will be checked only from the option/resource file
	//anyopt.setFileOption("title");      //an option (takes an argument), supporting only long form
	verbose = false;
	time(&startTime);
}

extern const char *versionString;

bool options::read(int argc, char* argv[]) {
	//PROCESS THE COMMAND LINE AND RESOURCE FILE */
	//read options from a  option/resource file with ':' separated opttions or flags, one per line
	//anyopt.processFile( "/home/user/.options" );  
	//go through the command line and get the options/
	anyopt.processCommandArgs(argc, argv);

	if((!anyopt.hasOptions()) || anyopt.getFlag("help")) { //print usage if no options
		anyopt.printUsage();
		return true;
	}
	verbose = anyopt.getFlag("verbose");
	if(anyopt.getFlag("version")) { //print version and exit
		cout << endl << versionString << endl;
		return true;
	}
	return false;
}

void options::printUsage() {
	anyopt.printUsage();
}

const char* options::getValue(const char* key) {
	return anyopt.getValue(key);
}

bool options::getFlag(const char* key) {
	return anyopt.getFlag(key);
}

const char* options::getStartTime() const {
	return ctime(&startTime);
}

int options::execCommand() {
	//always read unav options
	uaOptions uao;
	uaOpt = &uao;
	uao.readOptions();
	if(anyopt.getFlag("scan")) {
		scanOptions so;
		scanOpt = &so;
		return so.go();
	}
	if(anyopt.getFlag("solve")) {
		solveOptions so;
		solveOpt = &so;
		return so.go();
	}
	if(anyopt.getFlag("nhb")) {
		nhbOptions so;
		nhbOpt = &so;
		return so.go();
	}
	if(anyopt.getFlag("unav")) {
		//uaOptions so;
		//uaOpt = &so;
		return uaOpt->go();
	}
	if(anyopt.getFlag("similar")) {
		similarOptions so;
		similarOpt = &so;
		return so.go();
	}
	if(anyopt.getFlag("pattern")) {
		patternOptions so;
		patternOpt = &so;
		return so.go();
	}
	if(anyopt.getFlag("template")) {
		templateOptions so;
		templateOpt = &so;
		return so.go();
	}
	cout << "Error: No command specified." << endl;
	return -1;
}

