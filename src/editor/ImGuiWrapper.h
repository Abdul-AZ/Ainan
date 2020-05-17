#pragma once

#include "editor/Window.h"
#include "renderer/Renderer.h"

namespace Ainan {

	//abstracts the Dear ImGui library
	class ImGuiWrapper
	{
	public:
		static void Init();
		static void Terminate();
		static void NewFrame();
		static void Render();
	};
}