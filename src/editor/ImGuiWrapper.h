#pragma once

#include "editor/Window.h"
#include "renderer/Renderer.h"

namespace Ainan {

	//helper functions for ImGui
	class ImGuiWrapper
	{
	public:
		static void BeginGlobalDocking(bool hasMenuBar);
		static void EndGlobalDocking();
	};
}