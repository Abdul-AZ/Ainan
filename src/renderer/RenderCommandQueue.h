#pragma once

#include "RenderCommand.h"

namespace Ainan {

	class RenderCommandQueue
	{
	public:
		void Push(const RenderCommand& cmd);
		void WaitUntilIdle();
		
		//returns false and sets cmd to nullptr if there is nothing to return
		void WaitPopAndExecuteAll(std::function<void(const RenderCommand&)> func);

	private:
		std::queue<RenderCommand> m_InternalQueue;
		std::condition_variable m_WorkAvailableCV;
		std::condition_variable m_WorkDoneCV;
		std::mutex m_Mutex;
		bool m_Busy = false;
	};
}