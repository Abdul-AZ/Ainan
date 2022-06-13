#include "Profiler.h"

#include "renderer/Renderer.h"
#include <imgui.h>

namespace Ainan {

	void Profiler::DisplayGUI()
	{
		if (!WindowOpen)
			return;

		ImGui::Begin("Profiler", &WindowOpen, 0);

		if (ImGui::BeginTabBar("Profiler Mode", 0))
		{
			if (ImGui::BeginTabItem("General", 0, 0))
			{
				DisplayGeneralTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Rendering", 0, 0))
			{
				DisplayRenderingTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Profiler::RegisterTimeStep(const float simulationTS, const float realTS)
	{
		//calculate total time
		m_SimulationTime += simulationTS;
		m_RealTime += realTS;

		//save frame time
		m_FrameTimeCircularBuffer[m_FrameTimeBufferIndex] = realTS;
		m_FrameTimeBufferIndex++;
		if (m_FrameTimeBufferIndex >= m_FrameTimeCircularBuffer.size())
			m_FrameTimeBufferIndex = 0;
	}

	void Profiler::Reset()
	{
		m_SimulationTime = 0.0f;
		m_RealTime = 0.0f;
	}

	void Profiler::DisplayGeneralTab()
	{
		//this is to control how many decimal points we want to display
		std::stringstream stream;

		//we want 3 decimal places
		stream << std::fixed << std::setprecision(3) << m_SimulationTime;
		ImGui::Text("Simulation Time :");
		ImGui::SameLine();
		ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, stream.str().c_str());

		stream.str("");//clear the stream

		stream << std::fixed << std::setprecision(3) << m_RealTime;
		ImGui::Text("Actual Time :");
		ImGui::SameLine();
		ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, stream.str().c_str());
	}

	void Profiler::DisplayRenderingTab()
	{
		//update framerate every 30 frames
		static int32_t frameCounter = 1;
		if (frameCounter++ % 30 == 0)
		{
			m_AverageFPS = std::round(1.0f / (std::accumulate(m_FrameTimeCircularBuffer.begin(), m_FrameTimeCircularBuffer.end(), 0.0f) / m_FrameTimeCircularBuffer.size()));
			frameCounter = 1;
		}

		ImGui::PushItemWidth(-1);
		auto value_getter = [](void* value, int index) -> float
		{
			Profiler* prof = (Profiler*)value;
			int32_t idx = prof->m_FrameTimeBufferIndex - index;
			if (idx < 0)
				return prof->m_FrameTimeCircularBuffer[FRAME_TIME_ARRAY_SIZE + idx];
			else
				return prof->m_FrameTimeCircularBuffer[idx];
		};

		ImGui::PlotHistogram("Frame Time(s)",value_getter, this, FRAME_TIME_ARRAY_SIZE
		,0, ("Average FPS: " + std::to_string(m_AverageFPS)).c_str(), 0, 1 / 60.0f, ImVec2(0, 100));
		ImGui::PopItemWidth();

		ImGui::Text("Textures: ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(Renderer::Rdata->Textures.size()).c_str());

		ImGui::SameLine();
		ImGui::Text("   VBO(s): ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(Renderer::Rdata->VertexBuffers.size()).c_str());

		ImGui::SameLine();
		ImGui::Text("   EBO(s): ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(Renderer::Rdata->IndexBuffers.size()).c_str());

		ImGui::SameLine();
		ImGui::Text("   UBO(s): ");
		ImGui::SameLine();
		ImGui::Text(std::to_string(Renderer::Rdata->UniformBuffers.size()).c_str());

		bool displayTooltip = false;
		ImGui::SameLine();
		ImGui::Text("   Used GPU Memory: ");
		displayTooltip |= ImGui::IsItemHovered();
		ImGui::SameLine();
		uint32_t usedMemoryInBytes = Renderer::GetUsedGPUMemory();
		ImGui::Text(std::to_string(Renderer::GetUsedGPUMemory() / (1024 * 1024)).c_str());
		displayTooltip |= ImGui::IsItemHovered();
		ImGui::SameLine();
		ImGui::Text("Mb");
		displayTooltip |= ImGui::IsItemHovered();

		if (displayTooltip)
			ImGui::SetTooltip((std::to_string(usedMemoryInBytes / (1024)) + " KB").c_str());

		ImGui::Text("Global Particle Count :");
		ImGui::SameLine();
	}
}