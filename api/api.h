/*
 * api.h
 *
 *  Created on: Jul 3, 2018
 *      Author: Mladen Dobrichev
 */

#ifndef API_API_H_
#define API_API_H_
#include <thread>
#include <mutex>
#include <condition_variable>
#include "stdsoap2.h" //SOAP_SOCKET

class pgApi {
	static const int backlog = 100;
	static const int numProcessingThreads = 10;
	static const int queueSize = 200;
	bool exiting_ = false;
	void* service_ = NULL;
	std::thread listener_;
	const char *host_;
	int port_;
	int queueHead_ = 0;
	int queueTail_ = 0;
	std::mutex mutex_;
	std::condition_variable conditionQueueNotEmpty_;
	std::thread processingThreads[numProcessingThreads];
	SOAP_SOCKET queue[queueSize]; // request queue of sockets

	int enqueue(SOAP_SOCKET sock);
	SOAP_SOCKET dequeue();
	//static void *process_request(void *arg);
	static void process_request(pgApi& this_);
	bool isActive();
public:
	void listen(const char *host, int port);
	static int listenerThreadFunction(pgApi& this_);
	void shutdown();
};

#endif /* API_API_H_ */
