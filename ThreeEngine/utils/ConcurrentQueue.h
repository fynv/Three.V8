#pragma once

#include <queue>
#include "utils/Semaphore.h"

template<typename T>
class ConcurrentQueue
{
public:
	ConcurrentQueue()
	{
	}

	~ConcurrentQueue()
	{
	}

	size_t Size()
	{
		return m_queue.size();
	}

	T Front()
	{
		return m_queue.front();
	}

	void Push(T packet)
	{
		m_mutex.lock();
		m_queue.push(packet);
		m_mutex.unlock();
		m_semaphore_out.notify();
	}

	T Pop()
	{
		m_semaphore_out.wait();
		m_mutex.lock();
		T ret = m_queue.front();
		m_queue.pop();
		m_mutex.unlock();
		return ret;
	}

private:
	std::queue<T> m_queue;
	std::mutex m_mutex;
	Semaphore m_semaphore_out;
};

