#include <inttypes.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "skfr/ratingengine.h"

class fskfr {
	//buffers
	static const int bufSize = 128 * 6;
	skfr::puzzleToRate puzzlesToRate[bufSize];
	uint32_t *res[bufSize]; //where the compressed result goes (ED,EP,ER,0)
	int count;
	int head;
	int tail;
	bool active;
	std::mutex mutex_;
	std::condition_variable conditionNotEmpty;
	std::condition_variable conditionNotFull;
	std::vector<std::thread> pumps;
	static void ratingPump(fskfr& queue);

public:
	fskfr();
	~fskfr();
	//add to buffer
	void push(const char *p, uint32_t *res);
	//read from buffer
	void pop(char *p, uint32_t **rate);
	//rate partially populated buffer
	void commit();
//	void activate();
//	void deactivate();
	//rate single puzzle
	//int skfr::skfrER(const char *p) const;
	bool isActive();
};
