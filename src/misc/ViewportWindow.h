#pragma once

#include "renderer/Viewport.h"
#include "Window.h"

namespace Ainan {

	class ViewportWindow
	{
	public:
		void DisplayGUI();

	public:
		Viewport RenderViewport = {0};
	};
}