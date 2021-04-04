#include "RenderCommandQueue.h"

namespace Ainan {

    const int32_t c_MaxQueueSize = 10;

	void RenderCommandQueue::Push(const RenderCommand& cmd)
	{
        if (m_InternalQueue.size() >= c_MaxQueueSize)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_WorkConsumedCV.wait(lock, [this]() { return m_InternalQueue.size() < c_MaxQueueSize; });
        }

		std::scoped_lock<std::mutex> lock(m_Mutex);
		m_InternalQueue.push(cmd);
		m_WorkAvailableCV.notify_one();
	}

	void RenderCommandQueue::WaitUntilIdle()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
        using namespace std::chrono_literals;
		m_WorkDoneCV.wait_for(lock, 500ms, [this]() { return m_InternalQueue.empty() && (m_Busy == false); });
	}

	void RenderCommandQueue::WaitPopAndExecuteAll(std::function<void(const RenderCommand&)> func)
	{
        std::unique_lock<std::mutex> latch(m_Mutex);
        using namespace std::chrono_literals;
        m_WorkAvailableCV.wait_for(latch, 20ms, [this]() { return !m_InternalQueue.empty(); });
        while (!m_InternalQueue.empty())
        {
            // got work. set busy.
            m_Busy = true;

            // pull from queue
            auto cmd = m_InternalQueue.front();
            m_InternalQueue.pop();

            // release lock. run async
            latch.unlock();
            
            m_WorkConsumedCV.notify_one();

            // run function outside context
            func(cmd);

            latch.lock();
            m_Busy = false;
        }

        m_WorkDoneCV.notify_one();
	}
}