#pragma once

#include "environment/Window.h"
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
		void RegisterKey(int glfwKeyCode, std::string description, std::function<void()> func, int eventTrigger = GLFW_PRESS);
		void RegisterMouseKey(int glfwMouseKeyCode, std::string description, std::function<void()> func, int eventTrigger = GLFW_PRESS);
		void ClearKeys();
		void HandleInput();
		void DisplayGUI();

	public:
		bool ControlsWindowOpen = false;

	private:
		std::vector<RegisteredKey> m_Keys;
		std::vector<RegisteredKey> m_MouseKeys;
	};
}