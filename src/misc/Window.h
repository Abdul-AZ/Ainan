#pragma once

#include "renderer/Viewport.h"

#define WINDOW_SIZE_FACTOR_ON_LAUNCH 500

namespace ALZ {

	const float GlobalScaleFactor = 1000.0f;

	class Window
	{
	public:
		static void Init();
		static void Present();
		static void HandleWindowEvents();
		static void Clear();
		static void Terminate();
		static void CenterWindow();
		static void SetWindowLaunchSize();

		//do NOT modify these ever
		//they are only updated from functions inside Window.cpp
		static glm::vec2 FramebufferSize;
		static glm::vec2 Size;
		static glm::vec2 Position;
		static bool WindowSizeChangedSinceLastFrame;
		static Viewport WindowViewport;
		static GLFWwindow* Ptr;
	};
}