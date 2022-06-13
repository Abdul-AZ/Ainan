#pragma once

namespace Ainan {

#define FRAME_TIME_ARRAY_SIZE 128

	class Profiler
	{
	public:
		void DisplayGUI();

		void RegisterTimeStep(const float simulationTS, const float realTS);
		void Reset();

		bool WindowOpen = true;
	private:
		void DisplayGeneralTab();
		void DisplayRenderingTab();

	private:
		float									 m_SimulationTime = 0.0f;
		float									 m_RealTime = 0.0f;
		int32_t									 m_AverageFPS = 0;
	public:
		std::array<float, FRAME_TIME_ARRAY_SIZE> m_FrameTimeCircularBuffer;
		uint32_t								 m_FrameTimeBufferIndex = 0;
	};

}