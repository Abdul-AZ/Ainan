#pragma once

#include "Window.h"
#include "GLFWKeyToString.h"

namespace ALZ {

	struct RegisteredKey
	{
		int GLFWKeyCode;
		std::string Description;
		std::function<void()> OnClickFunction;
		int EventTrigger;
	};

	class InputManager {
	public:
		//eventTrigger is either GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
		void RegisterKey(int glfwKeyCode, std::string description, std::function<void()> func, int eventTrigger = GLFW_PRESS);
		void RegisterMouseKey(int glfwMouseKeyCode, std::string description, std::function<void()> func, int eventTrigger = GLFW_PRESS);
		void ClearKeys();
		void HandleInput();
		void DisplayGUI();
		glm::vec2 GetMousePositionNDC();

	public:
		bool ControlsWindowOpen = false;

	private:
		std::vector<RegisteredKey> m_Keys;
		std::vector<RegisteredKey> m_MouseKeys;
		
		//the key(int) is key code from glfw example : GLFW_KEY_A.
		//the value(int) has the following values, GLFW_PRESS, GLFW_REPEAT and GLFW_RELEASE.
		std::unordered_map<int, int> m_KeyStates;
	};
}