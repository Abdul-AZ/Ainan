#pragma once

#include "editor/Window.h"
#include "GLFWKey.h"

namespace Ainan {

	struct RegisteredKey
	{
		int GLFWKeyCode;
		std::string Description;
		std::function<void()> OnClickFunction;
		int EventTrigger;
	};

	class InputManager {
	public:
		static void Init();
		static void Terminate();
		//eventTrigger is either GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
		static void RegisterKey(int glfwKeyCode, std::string description, std::function<void()> func, int eventTrigger = GLFW_PRESS);
		static void RegisterMouseKey(int glfwMouseKeyCode, std::string description, std::function<void()> func, int eventTrigger = GLFW_PRESS);
		static void ClearKeys();
		static void HandleInput();
		static void DisplayGUI();
		static glm::vec2 GetMousePositionNDC();

		//returns either GLFW_PRESS or GLFW_REPEAT or GLFW_RELEASE
		static int GetKey(int glfwKey);

	public:
		static bool ControlsWindowOpen;
		static std::vector<std::function<void(double, double)>> m_ScrollFunctions;

	private:
		static std::vector<RegisteredKey> m_Keys;
		static std::vector<RegisteredKey> m_MouseKeys;
		
		//the key(int) is key code from glfw example : GLFW_KEY_A.
		//the value(int) has the following values, GLFW_PRESS, GLFW_REPEAT and GLFW_RELEASE.
		static std::unordered_map<int, int> m_KeyStates;

		friend static void scroll_callback(GLFWwindow* window, double x, double y);
	};
}