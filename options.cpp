#define _CRT_SECURE_NO_DEPRECATE

#include <time.h>
#include "options.h"

options opt = options();

options::options() {
	//Yes, I know it is a bad practice to run such code in the constructor... TODO

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

	anyopt.addUsage(" --pattern           Process pattern");
	anyopt.setCommandFlag("pattern");
	anyopt.addUsage("   --redundancy          Prints redundancy level for patterns from stdin");
	anyopt.setFlag("redundancy");
	anyopt.addUsage("   --statistics          Prints statistics for patterns from stdin (not impl.)");
	anyopt.setFlag("statistics");
	anyopt.addUsage("   --enumerate <pattern> Prints puzzles for given pattern");
	anyopt.setOption("enumerate");
	anyopt.addUsage("     --subcanon              Print canonicalized ED puzzles");
	anyopt.setFlag("subcanon");
	anyopt.addUsage("     --fixclues <puzzle>     Prints only puzzles containing this subpuzzle");
	anyopt.setOption("fixclues");
	anyopt.addUsage("   --patcanon            Prints canonicalized unique patterns from stdin");
	anyopt.setFlag("patcanon");
	anyopt.addUsage("   --pg depth,er,ER,ep,EP,ed,ED,maxPasses,noSingles  Re-labels stdin to stdout, related to Patterns Game");
	anyopt.addUsage("   --pg s<pattern> < grids.txt > puzzles.txt          Scans grids for puzzles having the given pattern");
	anyopt.setOption("pg");
	anyopt.addUsage("   --pgserver user:password@address:port < oldDbFile > newDbFile    Handles Patterns Game");
	anyopt.setOption("pgserver");
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
	if(anyopt.getFlag("pattern")) {
		patternOptions so;
		patternOpt = &so;
		return so.go();
	}
	cout << "Error: No command specified." << endl;
	return -1;
}

