// metafile with interface definitions for gsoap preprocessor
// gsoap command: soapcpp2 -j -r -L api_meta.h

#import "gsoap/import/stlvector.h" // import std::vector XML data binding

//gsoap pg service name:            pgServer Patterns Game Server API
//gsoap pg service protocol:        SOAP
//g...soap pg service style:           rpc
//g...soap pg service encoding:        encoded
//gsoap pg service namespace:       http://dobrichev.spnet.net/pg/pgServer.wsdl
//gsoap pg service location:        http://dobrichev.spnet.net/pgserver

//gsoap pg schema namespace:        urn:pgServer

class pg__job {
public:
	std::string jobName;
	std::vector<std::string> objectList;
};

//gsoap pg service method: getVersion Returns the version of the running server
int pg__getVersion(std::string& version);

//gsoap pg service method: getJob A Worker asks for job
int pg__getJob(pg__job& job);

//g...soap ns2  service method-protocol:      getPlayablePuzzles SOAP
//gsoap pg service method: getPlayablePuzzles Query for exemplars of top rated puzzles per rating
int pg__getPlayablePuzzles(std::vector<std::string>& puzzleList);

//g...soap ns2  service method-protocol:      dumpToFlatText SOAP
//gsoap pg service method: dumpToFlatText		Save a snapshot of the database
int pg__dumpToFlatText(std::string filename, int& numStoredRecords);

