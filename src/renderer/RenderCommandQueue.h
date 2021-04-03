#pragma once

#include "RenderCommand.h"

namespace Ainan {

	class RenderCommandQueue
	{
	public:
		void Push(const RenderCommand& cmd);
		void WaitUntilIdle();
		
		void WaitPopAndExecuteAll(std::function<void(const RenderCommand&)> func);

	private:
		std::queue<RenderCommand> m_InternalQueue;
		std::condition_variable m_WorkAvailableCV;
		std::condition_variable m_WorkDoneCV;
		std::condition_variable m_WorkConsumedCV;
		std::mutex m_Mutex;
		bool m_Busy = false;
	};
}