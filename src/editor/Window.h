#pragma once

#include "renderer/Rectangle.h"
#include "renderer/RendererAPI.h"

namespace Ainan {

	const int32_t c_StartMenuWidth = 450;
	const int32_t c_StartMenuHeight = 350;
	const int32_t c_CreateEnvironmentWindowWidth = 700;
	const int32_t c_CreateEnvironmentWindowHeight = 700 * 9 / 16;

	class Window
	{
	public:
		static void Init(RendererType api);
		static void HandleWindowEvents();
		static void Terminate();
		static void CenterWindow();
		static void Restore();
		static void Maximize();

		static void SetSize(const glm::ivec2& size);
		static void SetSize(int32_t width, int32_t height);
		static void SetShouldClose();
		static void SetTitle(const std::string& title);

	public:

		//do NOT modify these ever, use Setxxx functions for that
		//they are only updated from functions inside Window.cpp
		static bool ShouldClose;
		static bool Minimized;
		static glm::vec2 FramebufferSize;
		static glm::vec2 Size;
		static glm::vec2 Position;
		static bool WindowSizeChangedSinceLastFrame;
		static Rectangle WindowViewport;
		static GLFWwindow* Ptr;
	};
}