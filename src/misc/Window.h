#pragma once

#include "renderer/Viewport.h"

#define WINDOW_SIZE_ON_LAUNCH_X 500
#define WINDOW_SIZE_ON_LAUNCH_Y 500 * 9 / 16
#define WINDOW_SIZE_ON_CREATE_ENVIRONMENT_X 700
#define WINDOW_SIZE_ON_CREATE_ENVIRONMENT_Y 700 * 9 / 16

namespace Ainan {

	const float GlobalScaleFactor = 1000.0f;

	class Window
	{
	public:
		static void Init();
		static void Present();
		static void HandleWindowEvents();
		static void Terminate();
		static void CenterWindow();
		static void Restore();
		static void Maximize();

		static void SetSize(const glm::ivec2& size);
		static void SetShouldClose();
		static void SetTitle(const std::string& title);

	public:

		//do NOT modify these ever, use Setxxx functions for that
		//they are only updated from functions inside Window.cpp
		static bool ShouldClose;
		static glm::vec2 FramebufferSize;
		static glm::vec2 Size;
		static glm::vec2 Position;
		static bool WindowSizeChangedSinceLastFrame;
		static Viewport WindowViewport;
		static GLFWwindow* Ptr;
	};
}