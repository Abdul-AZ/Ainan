#pragma once

#include "renderer/Viewport.h"
#include "Window.h"

namespace ALZ {

	class ViewportWindow
	{
	public:
		void DisplayGUI();

	public:
		Viewport RenderViewport = {0};
	};
}