#pragma once

#include "editor/Window.h"
#include "GLFWKey.h"

class GLFWcursor;

namespace Ainan {

	struct RegisteredKey
	{
		int GLFWKeyCode;
		std::string Description;
		std::function<void(int32_t mods)> OnClickFunction;
		int EventTrigger;
	};

	class InputManager 
	{
	public:
		static void Init();
		static void Terminate();
		//eventTrigger is either GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
		static void RegisterKey(int glfwKeyCode, std::string description, std::function<void(int32_t mods)> func, int eventTrigger = GLFW_PRESS);
		static void RegisterMouseKey(int glfwMouseKeyCode, std::string description, std::function<void(int32_t mods)> func, int eventTrigger = GLFW_PRESS);
		static void ClearKeys();
		static void HandleInput();
		static void DisplayGUI();

		//returns either GLFW_PRESS or GLFW_REPEAT or GLFW_RELEASE
		static int GetKey(int glfwKey);

	public:
		static bool ControlsWindowOpen;
		static std::vector<std::function<void(double, double)>> m_ScrollHandlers;
		static std::vector<std::function<void(int32_t, int32_t, int32_t)>> m_GlobalButtonHandlers;
		static std::vector<std::function<void(int32_t, int32_t, int32_t, int32_t)>> m_GlobalKeyHandlers;
		static std::vector<std::function<void(uint32_t)>> m_GlobalCharacterKeyHandler;
		static std::array<bool, 5> MouseButtonPressed;
		static int32_t m_ModsDown;

		//the key(int) is key code from glfw example : GLFW_KEY_A.
		//the value(int) has the following values, GLFW_PRESS, GLFW_REPEAT and GLFW_RELEASE.
		static std::unordered_map<int, int> m_KeyStates;

		static std::vector<RegisteredKey> m_SpecificKeyHandler;
		static std::vector<RegisteredKey> m_SpecificMouseKeyHandler;
		static std::array<GLFWcursor*, ImGuiMouseCursor_COUNT> m_MouseCursors;
	};
}