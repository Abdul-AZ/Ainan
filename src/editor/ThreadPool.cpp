#pragma once

#include "ThreadPool.h"

namespace Ainan {

	ThreadPool::ThreadPool(uint32_t numThreads) :
		busy(ATOMIC_VAR_INIT(0U)),
		processed(ATOMIC_VAR_INIT(0U)),
		stop(false)
	{
		workers.resize(numThreads);
		for (uint32_t i = 0; i < numThreads; i++)
		{
			workers.at(i) = std::thread(std::bind(&ThreadPool::ThreadLoop, this));
		}
	}

	ThreadPool::~ThreadPool()
	{
		// set stop-condition
		std::unique_lock<std::mutex> latch(queue_mutex);
		stop = true;
		cv_task.notify_all();
		latch.unlock();

		// all threads terminate, then we're done.
		for (auto& t : workers)
			t.join();
	}

	void ThreadPool::QueueCommand(const std::function<void()>& func)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		tasks.emplace_back(std::forward<const std::function<void()>&>(func));
		cv_task.notify_one();
	}

	void ThreadPool::WaitAll()
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		cv_finished.wait(lock, [this]() { return tasks.empty() && (busy == 0); });
	}

	void ThreadPool::ThreadLoop()
	{
		while (true)
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			cv_task.wait(lock, [this]() { return stop || !tasks.empty(); });
			if (!tasks.empty())
			{
				// got work. set busy.
				++busy;

				// pull from queue
				auto fn = tasks.front();
				tasks.pop_front();

				// release lock. run async
				lock.unlock();

				// run function outside context
				fn();

				lock.lock();
				++processed;
				--busy;
				cv_finished.notify_one();
			}
			else if (stop)
			{
				break;
			}
		}
	}
}