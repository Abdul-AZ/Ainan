#pragma once

#include "renderer/Rectangle.h"
#include "editor/Window.h"
#include "renderer/FrameBuffer.h"

namespace Ainan {

	class ViewportWindow
	{
	public:
		void DisplayGUI(std::shared_ptr<FrameBuffer>& fb);

	public:
		Rectangle RenderViewport = {0, 0, 0, 0};
		bool IsHovered = false;
		bool IsFocused = false;

		//these are needed for rendering gizoms
		glm::vec2 WindowPosition = glm::vec2(0.0f, 0.0f);
		glm::vec2 WindowSize = glm::vec2(0.0f, 0.0f);
		glm::vec2 WindowContentRegionSize = glm::vec2(0.0f, 0.0f);
	};
}