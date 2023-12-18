#include "AsyncCallbacks.h"

AsyncQueue::AsyncQueue()
{

}

AsyncQueue::~AsyncQueue()
{	
	CheckPendings();
}

void AsyncQueue::Add(Callable* callable)
{
	queue.Push(callable);
}

void AsyncQueue::CheckPendings()
{
	while (queue.Size() > 0)
	{
		Callable* callable = queue.Pop();
		callable->call();
		delete callable;
	}
}

AsyncCallbacks::AsyncCallbacks()
{
	
}

AsyncCallbacks::~AsyncCallbacks()
{

}

AsyncCallbacks& AsyncCallbacks::_singleton()
{
	static AsyncCallbacks singleton;
	return singleton;
}

void AsyncCallbacks::Add(Callable* callable)
{
	AsyncCallbacks& singleton = _singleton();
	singleton.m_default_queue.Add(callable);
}

void AsyncCallbacks::AddSubQueue(AsyncQueue* queue)
{
	AsyncCallbacks& singleton = _singleton();
	singleton.m_sub_queues.insert(queue);
}

void AsyncCallbacks::RemoveSubQueue(AsyncQueue* queue)
{
	AsyncCallbacks& singleton = _singleton();
	singleton.m_sub_queues.erase(queue);
}

void AsyncCallbacks::CheckPendings()
{
	AsyncCallbacks& singleton = _singleton();
	singleton.m_default_queue.CheckPendings();
	auto iter = singleton.m_sub_queues.begin();
	while (iter != singleton.m_sub_queues.end())
	{
		(*iter)->CheckPendings();
		iter++;
	}

}