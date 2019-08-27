#include <pch.h>

#include "InputManager.h"
#include <GLFW/glfw3.h>

namespace ALZ {

	bool InputManager::ControlsWindowOpen = false;
	std::vector<RegisteredKey> InputManager::m_Keys;
	std::vector<RegisteredKey> InputManager::m_MouseKeys;
	std::unordered_map<int, int> InputManager::m_KeyStates;

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
			int state = InputManager::GetKey(key.GLFWKeyCode);

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
			if (glfwGetMouseButton(Window::Ptr, key.GLFWKeyCode) == key.EventTrigger)
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

	//NDC = Normalized Device Coordinates
	//aka ranges from -1 to 1
	glm::vec2 InputManager::GetMousePositionNDC()
	{
		double xpos, ypos;
		glfwGetCursorPos(Window::Ptr, &xpos, &ypos);

		int width, height;
		glfwGetWindowSize(Window::Ptr, &width, &height);

		//change from being relative to top left to being relative to bottom left
		ypos = -ypos + height;

		float NDC_xpos = (float)xpos * 2 / width - 1.0f;
		float NDC_ypos = (float)ypos * 2 / height - 1.0f;

		return glm::vec2(NDC_xpos, NDC_ypos);
	}

	int InputManager::GetKey(int glfwKey)
	{
		return glfwGetKey(Window::Ptr, glfwKey);
	}
}