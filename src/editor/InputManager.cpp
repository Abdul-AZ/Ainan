#include "InputManager.h"
#include <GLFW/glfw3.h>

#include "renderer/Renderer.h"

namespace Ainan {

	bool InputManager::ControlsWindowOpen = false;
	std::vector<RegisteredKey> InputManager::m_SpecificKeyHandler;
	std::vector<RegisteredKey> InputManager::m_SpecificMouseKeyHandler;
	std::unordered_map<int32_t, int32_t> InputManager::m_KeyStates;
	std::array<GLFWcursor*, ImGuiMouseCursor_COUNT> InputManager::m_MouseCursors = {};
	std::array<bool, 5> InputManager::MouseButtonPressed = { false, false, false, false, false };
	std::vector<std::function<void(double, double)>> InputManager::m_ScrollHandlers;
	std::vector<std::function<void(int32_t, int32_t, int32_t)>> InputManager::m_GlobalButtonHandlers;
	std::vector<std::function<void(int32_t, int32_t, int32_t, int32_t)>> InputManager::m_GlobalKeyHandlers;
	std::vector<std::function<void(uint32_t)>> InputManager::m_GlobalCharacterKeyHandler;
	int32_t InputManager::m_ModsDown = 0;
	glm::vec2 InputManager::m_OldMousePos(0.0f);
	glm::vec2 InputManager::MousePos(0.0f);
	glm::vec2 InputManager::MouseDelta(0.0f);

	static void scroll_callback(GLFWwindow* window, double x, double y) 
	{
		for (auto& fun : InputManager::m_ScrollHandlers)
		{
			fun(x, y);
		}
	}

	static void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
	{
		for (auto& fun : InputManager::m_GlobalButtonHandlers)
		{
			fun(button, action, mods);
		}
		InputManager::m_ModsDown = mods;
	}

	static void key_callback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
	{
		for (auto& fun : InputManager::m_GlobalKeyHandlers)
		{
			fun(key, scancode, action, mods);
		}

		int32_t& currentState = InputManager::m_KeyStates[key];

		if (action == GLFW_RELEASE)
			currentState = GLFW_RELEASE;
		else if (action == GLFW_PRESS)
		{
			if (currentState == GLFW_PRESS)
				currentState = GLFW_REPEAT;

			else if (currentState != GLFW_PRESS && currentState != GLFW_REPEAT)
				currentState = GLFW_PRESS;
		}
		else
			currentState = action;

		//GLFW_REPEAT actions are handled in the HandleInput function
		if (action == GLFW_PRESS)
		{
			for (auto& regKey : InputManager::m_SpecificKeyHandler)
			{
				if (regKey.GLFWKeyCode == key)
					regKey.OnClickFunction(mods);
			}
		}
		InputManager::m_ModsDown = mods;
	}

	static void char_callback(GLFWwindow*, uint32_t c)
	{
		for (auto& fun : InputManager::m_GlobalCharacterKeyHandler)
		{
			fun(c);
		}
	}

	void InputManager::Init()
	{
		glfwSetScrollCallback(Window::Ptr, scroll_callback);
		ImGuiIO& io = ImGui::GetIO();

		auto imguiScrollFunc = [](double xoffset, double yoffset)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += (float)xoffset;
			io.MouseWheel += (float)yoffset;
		};

		m_ScrollHandlers.push_back(imguiScrollFunc);

		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		io.SetClipboardTextFn = [](void* user_data, const char* text)
		{
			return glfwSetClipboardString((GLFWwindow*)user_data, text);
		};

		io.GetClipboardTextFn = [](void* user_data)
		{
			return glfwGetClipboardString((GLFWwindow*)user_data);
		};

		io.ClipboardUserData = (void*)Window::Ptr;

		m_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		m_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		glfwSetMouseButtonCallback(Window::Ptr, mouse_button_callback);

		auto imGuiMouseButtonFunc = [](int32_t button, int32_t action, int32_t mods)
		{
			if (action == GLFW_PRESS && button >= 0 && button < InputManager::MouseButtonPressed.size())
				InputManager::MouseButtonPressed[button] = true;
		};
		m_GlobalButtonHandlers.push_back(imGuiMouseButtonFunc);

		glfwSetKeyCallback(Window::Ptr, key_callback);

		auto imguiKeyCallback = [](int32_t key, int32_t scancode, int32_t action, int32_t mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (action == GLFW_PRESS)
				io.KeysDown[key] = true;
			if (action == GLFW_RELEASE)
				io.KeysDown[key] = false;

			// Modifiers are not reliable across systems
			io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
		};

		m_GlobalKeyHandlers.push_back(imguiKeyCallback);

		glfwSetCharCallback(Window::Ptr, char_callback);

		auto imguiCharCallback = [](uint32_t c)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((uint16_t)c);
		};

		m_GlobalCharacterKeyHandler.push_back(imguiCharCallback);
	}

	void InputManager::Terminate()
	{
		glfwSetScrollCallback(Window::Ptr, nullptr);
		ClearKeys();

		//destroy cursors
		for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
		{
			glfwDestroyCursor(m_MouseCursors[cursor_n]);
			m_MouseCursors[cursor_n] = nullptr;
		}
	}

	void InputManager::RegisterKey(int glfwKeyCode, std::string description, std::function<void(int32_t mods)> func, int32_t eventTrigger)
	{
		m_SpecificKeyHandler.push_back({ glfwKeyCode, description, func, eventTrigger });
		m_KeyStates[glfwKeyCode] = 0;
	}

	void InputManager::RegisterMouseKey(int glfwMouseKeyCode, std::string description, std::function<void(int32_t mods)> func, int32_t eventTrigger)
	{
		m_SpecificMouseKeyHandler.push_back({ glfwMouseKeyCode, description, func, eventTrigger });
	}

	void InputManager::ClearKeys()
	{
		m_SpecificKeyHandler.clear();
		m_SpecificMouseKeyHandler.clear();
		m_ScrollHandlers.clear();
	}

	void InputManager::HandleInput()
	{
		for (RegisteredKey& key : m_SpecificKeyHandler)
			if(key.EventTrigger == GLFW_REPEAT)
				if (m_KeyStates[key.GLFWKeyCode] == GLFW_PRESS || m_KeyStates[key.GLFWKeyCode] == GLFW_REPEAT)
					key.OnClickFunction(m_ModsDown);

		for (RegisteredKey& key : m_SpecificMouseKeyHandler)
			if (glfwGetMouseButton(Window::Ptr, key.GLFWKeyCode) == key.EventTrigger)
				key.OnClickFunction(m_ModsDown);

		//update mouse cursor shape
		auto& io = ImGui::GetIO();
		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		for (int n = 0; n < platform_io.Viewports.Size; n++)
		{
			GLFWwindow* window = (GLFWwindow*)platform_io.Viewports[n]->PlatformHandle;
			glfwSetCursor(window, m_MouseCursors[imgui_cursor] ? m_MouseCursors[imgui_cursor] : m_MouseCursors[ImGuiMouseCursor_Arrow]);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		// Update buttons
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
		{
			// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
			io.MouseDown[i] = MouseButtonPressed[i] || glfwGetMouseButton(Window::Ptr, i) != 0;
			MouseButtonPressed[i] = false;
		}

		// Update mouse position
		const ImVec2 mouse_pos_backup = io.MousePos;
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		io.MouseHoveredViewport = 0;

		for (int n = 0; n < platform_io.Viewports.Size; n++)
		{
			ImGuiViewport* viewport = platform_io.Viewports[n];
			GLFWwindow* window = (GLFWwindow*)viewport->PlatformHandle;
			IM_ASSERT(window != NULL);
			const bool focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
			if (focused)
			{
				if (io.WantSetMousePos)
				{
					glfwSetCursorPos(window, ((double)mouse_pos_backup.x - (double)viewport->Pos.x), ((double)mouse_pos_backup.y - (double)viewport->Pos.y));
				}
				else
				{
					double mouse_x, mouse_y;
					glfwGetCursorPos(window, &mouse_x, &mouse_y);
					MousePos = glm::vec2(mouse_x, mouse_y);
					MouseDelta = MousePos - m_OldMousePos;
					m_OldMousePos = MousePos;

					// Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
					int window_x, window_y;
					glfwGetWindowPos(window, &window_x, &window_y);
					io.MousePos = ImVec2((float)mouse_x + window_x, (float)mouse_y + window_y);
				}
				for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
					io.MouseDown[i] |= glfwGetMouseButton(window, i) != 0;
			}
		}
	}

	void InputManager::DisplayGUI()
	{
		if (!ControlsWindowOpen)
			return;

		ImGui::Begin("Keyboard Controls", &ControlsWindowOpen);

		for (RegisteredKey& key : m_SpecificKeyHandler) {
			if (key.Description == "")
				continue;

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), GLFWKeyToString(key.GLFWKeyCode));
			ImGui::SameLine();
			ImGui::SetCursorPosX(150.0f);
			ImGui::Text(key.Description.c_str());
			ImGui::Separator();
		}

		ImGui::Spacing();

		for (RegisteredKey& key : m_SpecificMouseKeyHandler) {
			if (key.Description == "")
				continue;

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), GLFWMouseKeyToString(key.GLFWKeyCode));
			ImGui::SameLine();
			ImGui::SetCursorPosX(150.0f);
			ImGui::Text(key.Description.c_str());
			ImGui::Separator();
		}

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	int32_t InputManager::GetKey(int glfwKey)
	{
		return glfwGetKey(Window::Ptr, glfwKey);
	}
}