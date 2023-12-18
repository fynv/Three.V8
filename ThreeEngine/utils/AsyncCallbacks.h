#pragma once

#include <memory>
#include <unordered_set>
#include "ConcurrentQueue.h"

class Callable
{
public:
	Callable() {};
	virtual ~Callable() { }
	virtual void call() = 0;
};

class AsyncQueue
{
public:
	AsyncQueue();
	~AsyncQueue();
	void Add(Callable* callable);	
	void CheckPendings();

private:
	ConcurrentQueue<Callable*> queue;
};

class AsyncCallbacks
{
public:
	static void Add(Callable* callable);
	static void AddSubQueue(AsyncQueue* queue);
	static void RemoveSubQueue(AsyncQueue* queue);	
	static void CheckPendings();

private:
	AsyncQueue m_default_queue;
	std::unordered_set<AsyncQueue*> m_sub_queues;

	AsyncCallbacks();
	~AsyncCallbacks();

	static AsyncCallbacks& _singleton();

};
