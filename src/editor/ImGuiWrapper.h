#pragma once

#include "editor/Window.h"

#include "renderer/Renderer.h"

namespace Ainan {

	class ImGuiWrapper
	{
	public:
		static void Init();
		static void Terminate();
		static void NewFrame();
		static void Render();
	};
}