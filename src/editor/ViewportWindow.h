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
		bool IsFocused = false;
	};
}