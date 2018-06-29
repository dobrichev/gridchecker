//uses skfrdll.lib, see details at http://code.google.com/p/skfr-sudoku-fast-rating/

//#define DLLEXPORT __declspec(dllexport)
//extern "C" DLLEXPORT int __stdcall ratePuzzle(char *ze, int * er, int * ep, int * ed, int * aig);
//extern "C" DLLEXPORT void __stdcall setMinMax(int mined,int maxed, int minep, int maxep, int miner, int maxer, unsigned int filt);
//extern "C" DLLEXPORT void __stdcall setParam(int o1, int delta, int os, int oq, int ot, int oexclude, int edcycles);
//extern "C" DLLEXPORT int __stdcall setTestMode(int ot, char * logFileName);
//extern "C" DLLEXPORT void __stdcall ratePuzzles(int nPuzzles, char *ze, int *er, int *ep, int *ed, int *aig, int *ir);

#if 1
#include "rate.h"

//skfr static lib exports
//struct puzzleToRate {
//	int er;
//	int ep;
//	int ed;
//	char p[81];
//};
//
//void rateManyPuzzles(int nPuzzles, puzzleToRate *p);

fskfr::fskfr() :
		count(0), head(0), tail(0), active(true) {
	int numRatingThreads = std::thread::hardware_concurrency();
	if(numRatingThreads == 0) numRatingThreads = 1; //at least one
	for (int i = 0; i < numRatingThreads; ++i) {
		std::thread pump(std::bind(&ratingPump, std::ref(*this)));
		pumps.push_back(std::move(pump));
	}
}
fskfr::~fskfr() {
	{
		std::unique_lock<std::mutex> mlock(mutex_); //wait until lock is achieved
		active = false;
	}
	conditionNotFull.notify_all();
	conditionNotEmpty.notify_all();
	for (auto& pump : pumps) {
		pump.join();
	}
}
bool fskfr::isActive() const {
	//std::unique_lock<std::mutex> mlock(mutex_); //wait until lock is achieved
	return active;
}
void fskfr::ratingPump(fskfr& queue) {
	for(; queue.isActive();) {
		skfr::puzzleToRate p;
		uint32_t *res;
		queue.pop(p.p, &res);
		skfr::rateOnePuzzle(p);
		if (queue.isActive()) *res = ((*res) & 0xFF) | (p.ed << 24) | (p.ep << 16) | (p.er << 8); //don't touch the less significant 8 bits!
	}
}
void fskfr::push(const char *p, uint32_t *rate) {
	{
		//do the job
		std::unique_lock<std::mutex> mlock(mutex_); //wait until lock is achieved
		while (count == bufSize && active) {
			conditionNotFull.wait(mlock);
		}
		for(int i = 0; i < 81; i++) {
			puzzlesToRate[head].p[i] = p[i] + '0';
		}
		res[head] = rate;
		head++;
		count++;
		if(head >= bufSize) {
			head = 0;
		}
		//commit();
		//mlock.unlock();
	}
	//cond_.notify_one(); //not sure whether a burst of pushes would notify more than one thread. Safely notify all.
	conditionNotEmpty.notify_all(); //notify all waiting threads that the buffer isn't empty
}
void fskfr::pop(char *p, uint32_t **rate) {
	{
		//do the job
		std::unique_lock<std::mutex> mlock(mutex_);
		while (count == 0 && active) {
			conditionNotEmpty.wait(mlock);
		}
		for (int i = 0; i < 81; i++) {
			p[i] = puzzlesToRate[tail].p[i];
		}
		*rate = res[tail];
		tail++;
		if (tail >= bufSize) {
			tail = 0;
		}
		count--;
		//mlock.unlock();
	}
	conditionNotFull.notify_all(); //notify all waiting threads that the buffer isn't full
}
void fskfr::commit() { //block the thread until somebody else emptied the buffer
	std::unique_lock<std::mutex> mlock(mutex_);
	while (active && count != 0) {
		conditionNotFull.wait(mlock);
	}
}
//void fskfr::commit() {
//	//skfr::rateManyPuzzles(count, puzzlesToRate);
//#ifdef _OPENMP
//#pragma omp parallel for schedule(dynamic, 1)
//#endif //_OPENMP
//	for(int i = 0; i < count; i++) {
//		skfr::rateOnePuzzle(puzzlesToRate[i]);
//		*res[i] = ((*res[i]) & 0xFF) | (puzzlesToRate[i].ed << 24) | (puzzlesToRate[i].ep << 16) | (puzzlesToRate[i].er << 8); //don't touch the less significant 8 bits!
//	}
//	count = 0;
//}
#endif
