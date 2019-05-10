#include <pch.h>

#include "InputManager.h"

namespace ALZ {

	void InputManager::RegisterKey(int glfwKeyCode, std::string description, std::function<void()> func, int eventTrigger)
	{
		m_Keys.push_back({ glfwKeyCode, description, func, eventTrigger });
		m_KeyStates[glfwKeyCode] = 0;
	}

	void InputManager::RegisterMouseKey(int glfwMouseKeyCode, std::string description, std::function<void()> func, int eventTrigger)
	{
		m_MouseKeys.push_back({ glfwMouseKeyCode, description, func, eventTrigger });
	}

	void InputManager::ClearKeys()
	{
		m_Keys.clear();
	}

	void InputManager::HandleInput()
	{
		for (RegisteredKey& key : m_Keys)
		{
			int state = glfwGetKey(&Window::GetWindow(), key.GLFWKeyCode);

			int& currentState = m_KeyStates[key.GLFWKeyCode];

			if (state == GLFW_RELEASE)
				currentState = GLFW_RELEASE;
			else if (state == GLFW_PRESS)
			{
				if (currentState == GLFW_PRESS)
					currentState = GLFW_REPEAT;

				else if (currentState != GLFW_PRESS && currentState != GLFW_REPEAT)
					currentState = GLFW_PRESS;
			}
			else
				currentState = state;
		}

		for (RegisteredKey& key : m_Keys)
			if (m_KeyStates[key.GLFWKeyCode] == key.EventTrigger)
				key.OnClickFunction();

		for (RegisteredKey& key : m_MouseKeys)
			if (glfwGetMouseButton(&Window::GetWindow(), key.GLFWKeyCode) == key.EventTrigger)
				key.OnClickFunction();
	}

	void InputManager::DisplayGUI()
	{
		if (!ControlsWindowOpen)
			return;

		ImGui::Begin("Keyboard Controls", &ControlsWindowOpen);

		for (RegisteredKey& key : m_Keys) {
			if (key.Description == "")
				continue;

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), GLFWKeyToString(key.GLFWKeyCode));
			ImGui::SameLine();
			ImGui::SetCursorPosX(150.0f);
			ImGui::Text(key.Description.c_str());
			ImGui::Separator();
		}

		ImGui::Spacing();

		for (RegisteredKey& key : m_MouseKeys) {
			if (key.Description == "")
				continue;

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), GLFWMouseKeyToString(key.GLFWKeyCode));
			ImGui::SameLine();
			ImGui::SetCursorPosX(150.0f);
			ImGui::Text(key.Description.c_str());
			ImGui::Separator();
		}

		ImGui::End();
	}
}