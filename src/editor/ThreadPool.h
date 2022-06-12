#pragma once

#include <GLFW/glfw3.h>
#include "renderer/Rectangle.h"
#include "renderer/RendererAPI.h"

namespace Ainan {

	class ThreadPool
	{
	public:
		ThreadPool(uint32_t numThreads);
		~ThreadPool();
		void QueueCommand(const std::function<void()>& func);
		void WaitAll();

	private:
		void ThreadLoop();

		std::vector< std::thread > workers;
		std::deque< std::function<void()> > tasks;
		std::mutex queue_mutex;
		std::condition_variable cv_task;
		std::condition_variable cv_finished;
		std::atomic_uint busy, processed;
		bool stop;
	};
}