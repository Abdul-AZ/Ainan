#include "RenderCommandQueue.h"

namespace Ainan {

	void RenderCommandQueue::Push(const RenderCommand& cmd)
	{
		std::scoped_lock<std::mutex> lock(m_Mutex);
		m_InternalQueue.push(cmd);
		m_WorkAvailableCV.notify_one();
	}

	void RenderCommandQueue::WaitUntilIdle()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_WorkDoneCV.wait(lock, [this]() { return m_InternalQueue.empty() && (m_Busy == false); });
	}

	void RenderCommandQueue::WaitPopAndExecuteAll(std::function<void(const RenderCommand&)> func)
	{
        std::unique_lock<std::mutex> latch(m_Mutex);
        m_WorkAvailableCV.wait(latch, [this]() { return !m_InternalQueue.empty(); });
        while (!m_InternalQueue.empty())
        {
            // got work. set busy.
            m_Busy = true;

            // pull from queue
            auto cmd = m_InternalQueue.front();
            m_InternalQueue.pop();

            // release lock. run async
            latch.unlock();

            // run function outside context
            func(cmd);

            latch.lock();
            m_Busy = false;
        }

        m_WorkDoneCV.notify_one();
	}
}