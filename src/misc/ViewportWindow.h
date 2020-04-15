#pragma once

#include "renderer/Rectangle.h"
#include "Window.h"

namespace Ainan {

	class ViewportWindow
	{
	public:
		void DisplayGUI();

	public:
		Rectangle RenderViewport = {0, 0, 0, 0};
		bool IsFocused = false;
	};
}