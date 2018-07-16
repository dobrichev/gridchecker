#include "soappgServerService.h"
#include "soappgServerProxy.h"
#include "pgServer.nsmap"
#include "plugin/threads.h"
#include "api.h" //non-gSoap related header

//////
///  used global vars and functions
extern const char *versionString;
void getPlayablePuzzles(std::vector<std::string>& puzzleList);
bool dumpToFlatText(const std::string& filename, int& numStoredRecords);
void getRatingStatistics(std::vector<std::string>& puzzleList);

////////////////////////
/// API multithreading framework
bool pgApi::isActive() {
	std::unique_lock<std::mutex> mlock(mutex_); //wait until lock is achieved
	return ! exiting_;
}
int pgApi::enqueue(SOAP_SOCKET sock)
{
   int status = SOAP_OK;
   int next;
   std::unique_lock<std::mutex> mlock(mutex_);
   next = queueTail_ + 1;
   if (next >= queueSize)
      next = 0;
   if (next == queueHead_)
      status = SOAP_EOM;
   else
   {
      queue[queueTail_] = sock;
      queueTail_ = next;
      conditionQueueNotEmpty_.notify_all();
   }
   return status;
}
SOAP_SOCKET pgApi::dequeue()
{
   SOAP_SOCKET sock;
   std::unique_lock<std::mutex> mlock(mutex_);
   while (queueHead_ == queueTail_) {
	   conditionQueueNotEmpty_.wait(mlock);
   }
   sock = queue[queueHead_++];
   if (queueHead_ >= queueSize)
	   queueHead_ = 0;
   return sock;
}
void pgApi::process_request(pgApi& this_) {
	pgServerService* service = (static_cast<pgServerService*>(this_.service_))->copy(); //clone the service context for
	//fprintf(stderr, "pgApi::process_request: initialized\n");
	while (this_.isActive()) {//one of the ways to shutdown the thread
		service->soap->socket = this_.dequeue();
		//fprintf(stderr, "pgApi::process_request: request read %d\n", service->soap->socket);
		if (!soap_valid_socket(service->soap->socket)) { //one of the ways to shutdown the thread
			//fprintf(stderr, "pgApi::process_request: invalid request read, exiting %d\n", service->soap->socket);
			break;
		}
		service->serve();
		service->destroy(); // clean up the serialized data
		//fprintf(stderr, "pgApi::process_request: request served\n");
	}
	//fprintf(stderr, "pgApi::process_request: exiting\n");
	service->destroy(); // clean up
	delete service;
	//fprintf(stderr, "pgApi::process_request: exited\n");
}
void pgApi::listen(const char *host, int port) {
	host_ = host;
	port_ = port;
	listener_ = std::thread(std::bind(&pgApi::listenerThreadFunction, std::ref(*this)));
}
int pgApi::listenerThreadFunction(pgApi& this_) {
	//fprintf(stderr, "pgApi::listenerThreadFunction: entry\n");
	while(!this_.exiting_) {
		//fprintf(stderr, "pgApi::listenerThreadFunction: initializing\n");
		pgServerService service(SOAP_IO_KEEPALIVE); /* enable HTTP kee-alive */
		service.soap->send_timeout = service.soap->recv_timeout = 5; /* 5 sec socket idle timeout */
		service.soap->transfer_timeout = 30; /* 30 sec message transfer timeout */
		//service.soap->accept_timeout = 5; //restart after 5 seconds idle if not exiting
		//service.soap->linger_time = 5;
		SOAP_SOCKET masterSocket = service.bind(this_.host_, this_.port_, this_.backlog); /* master socket */
		if (!soap_valid_socket(masterSocket)) {
			//unrecoverable error
			return 1;
		}
		this_.service_ = &service;
		//prepare a clone of the service context for each of the processing threads and runn the threads
		//fprintf(stderr, "pgApi::listenerThreadFunction: running processing threads\n");
		for (int i = 0; i < numProcessingThreads; i++) {
			std::thread processingThread(std::bind(&process_request, std::ref(this_)));
			this_.processingThreads[i] = std::move(processingThread);
		}
		//fprintf(stderr, "pgApi::listenerThreadFunction: initialized\n");
		if (soap_valid_socket(masterSocket)) {
			SOAP_SOCKET slaveSocket;
			//fprintf(stderr, "pgApi::listenerThreadFunction: before accept loop\n");
			while (this_.isActive()) {
				//fprintf(stderr, "pgApi::listenerThreadFunction: before accept()\n");
				slaveSocket = service.accept(); //the blocking call
				//fprintf(stderr, "pgApi::listenerThreadFunction: after accept()\n");
				if (!soap_valid_socket(slaveSocket)) {
					if (service.soap->errnum) {
						service.soap_print_fault(stderr);
						continue; // retry
					}
					else {
						//fprintf(stderr, "Server timed out\n");
						break;
					}
				}
				//fprintf(stderr, "Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (soap.ip >> 24)&0xFF, (soap.ip >> 16)&0xFF, (soap.ip >> 8)&0xFF, soap.ip&0xFF);
				//fprintf(stderr, "pgApi::listenerThreadFunction: before enqueue()\n");
				while (this_.enqueue(slaveSocket) == SOAP_EOM) { //this is non-blocking
					sleep(1);
				}
				//fprintf(stderr, "pgApi::listenerThreadFunction: after enqueue()\n");
			}
			//shutdown all processing threads by sending this special request
			//fprintf(stderr, "pgApi::listenerThreadFunction: shutting down processind threads\n");
			for (int i = 0; i < numProcessingThreads; i++) {
				while (this_.enqueue(SOAP_INVALID_SOCKET) == SOAP_EOM) {
					sleep(1);
				}
			}
			//fprintf(stderr, "pgApi::listenerThreadFunction: shutdown messages sent\n");
			//wait for threads to finish
			for (int i = 0; i < numProcessingThreads; i++) {
				this_.processingThreads[i].join();
			}
			//fprintf(stderr, "pgApi::listenerThreadFunction: processingThreads were joined\n");
		}
		//service.soap_stream_fault(std::cerr);
		service.destroy(); /* clean up */
		this_.service_ = NULL;
		//fprintf(stderr, "pgApi::listenerThreadFunction: service destroyed\n");
	}
	//fprintf(stderr, "pgApi::listenerThreadFunction: exiting\n");
	return 0;
}
void pgApi::shutdown() {
	//set exiting flag and immediately do a (dummy) call to the API interface so that it realizes the flag is set
	std::string endpoint;
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		endpoint = host_;
		endpoint.append(":");
		endpoint.append(std::to_string(port_));
		exiting_ = true;
	}
	pgServerProxy client(endpoint.c_str());
	client.soap->recv_timeout = 3;
	client.soap->send_timeout = 3;
	client.soap->connect_timeout = 3; //if there is no listening socket on the server side
	std::string version;
	client.getVersion(version);
	listener_.join(); //block until the listener thread completes
}

////////////////////////////////
//API services implementation
int pgServerService::getVersion(std::string &version) {
	version = versionString;
	return SOAP_OK;
}

int pgServerService::getJob(pg__job& job) {
	job.jobName = "fastRate";
	job.objectList.push_back("1 ala-bala ala-bala");
	job.objectList.push_back("2 ala-bala ala-bala");
	return SOAP_OK;
}

int pgServerService::getPlayablePuzzles(std::vector<std::string>& puzzleList) {
	::getPlayablePuzzles(puzzleList);
	return SOAP_OK;
}

int pgServerService::dumpToFlatText(const std::string& filename, int& numStoredRecords) {
	if(::dumpToFlatText(filename, numStoredRecords)) return SOAP_ERR;
	return SOAP_OK;
}

int pgServerService::getRatingStatistics(std::vector<std::string>& ratingRecords) {
	::getRatingStatistics(ratingRecords);
	return SOAP_OK;
}
