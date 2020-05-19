#include <pch.h>

#include "ImGuiWrapper.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//TEMPORARY
#ifdef PLATFORM_WINDOWS
#include "imgui_impl_dx11.cpp"
#include "renderer/d3d11/D3D11RendererAPI.h"
#include "imgui_impl_glfw.cpp"
#endif // PLATFORM_WINDOWS



#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window
#endif
#define GLFW_HAS_WINDOW_TOPMOST     (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED     (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI    (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3300) // 3.3+ glfwGetMonitorContentScale
#define GLFW_HAS_VULKAN             (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ glfwCreateWindowSurface
#define GLFW_HAS_FOCUS_WINDOW       (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 >= 3200) // 3.2+ glfwFocusWindow

namespace Ainan {

	static double               Time = 0.0;
	static bool                 MouseJustPressed[5] = { false, false, false, false, false };
	static GLFWcursor*          MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };
	static bool                 WantUpdateMonitors = true;
	static char					GlslVersionString[32] = "";
	static GLuint				FontTexture = 0;

	static std::shared_ptr<ShaderProgram> ImGuiShader = nullptr;
	static std::shared_ptr<IndexBuffer> ImGuiIndexBuffer = nullptr;
	static std::shared_ptr<VertexBuffer> ImGuiVertexBuffer = nullptr;

	static int					AttribLocationVtxPos = 0, AttribLocationVtxUV = 0, AttribLocationVtxColor = 0; // Vertex attributes location

	// Chain GLFW callbacks for main viewport: our callbacks will call the user's previously installed callbacks, if any.
	static GLFWmousebuttonfun   PrevUserCallbackMousebutton = NULL;
	static GLFWscrollfun        PrevUserCallbackScroll = NULL;
	static GLFWkeyfun           PrevUserCallbackKey = NULL;
	static GLFWcharfun          PrevUserCallbackChar = NULL;


	//forward declarations
	//these functions are all static and not defined in the header file because they should not be used outside of this cpp file
	static bool ImGuiInit(const char*);
	static void Glfw_CreateWindow(ImGuiViewport* viewport);
	static void Glfw_DestroyWindow(ImGuiViewport* viewport);
	static bool Glfw_GetWindowFocus(ImGuiViewport* viewport);
	static bool Glfw_GetWindowMinimized(ImGuiViewport* viewport);
	static ImVec2 Glfw_GetWindowPos(ImGuiViewport* viewport);
	static ImVec2 Glfw_GetWindowSize(ImGuiViewport* viewport);
	static void Glfw_ShowWindow(ImGuiViewport* viewport);
	static void Glfw_ShutdownPlatformInterface();
	static void Glfw_SwapBuffers(ImGuiViewport* viewport, void*);
	static void Glfw_UpdateMonitors();
	static void Glfw_MonitorCallback(GLFWmonitor*, int);
	static void Glfw_RenderWindow(ImGuiViewport* viewport, void*);
	static void Glfw_SetWindowFocus(ImGuiViewport* viewport);
	static void Glfw_InitPlatformInterface();
	static void Glfw_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos);
	static void Glfw_SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
	static void Glfw_SetWindowTitle(ImGuiViewport* viewport, const char* title);
	static void Win32_SetImeInputPos(ImGuiViewport* viewport, ImVec2 pos);
	static void Glfw_SetClipboardText(void* user_data, const char* text);
	static const char* Glfw_GetClipboardText(void* user_data);
	static void InitPlatformInterface();
	static void RenderWindow(ImGuiViewport* viewport, void*);
	static void CharCallback(GLFWwindow* window, unsigned int c);
	static void Glfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void Glfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static bool CreateDeviceObjects();
	static void DestroyDeviceObjects();
	static void Glfw_Shutdown();
	static void RenderDrawData(ImDrawData* draw_data);
	static void UpdateMousePosAndButtons();
	static void UpdateMouseCursor();

	struct ImGuiViewportDataGlfw
	{
		GLFWwindow* Window;
		bool        WindowOwned;

		ImGuiViewportDataGlfw() { Window = NULL; WindowOwned = false; }
		~ImGuiViewportDataGlfw() { IM_ASSERT(Window == NULL); }
	};

	void ImGuiWrapper::Init()
	{
		//TEMPORARY
		if (Renderer::m_CurrentActiveAPI->GetContext()->GetType() == RendererType::D3D11)
		{
			ImGui::CreateContext();


			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;           // Enable viewports

			auto context = (D3D11::D3D11RendererContext*)Renderer::m_CurrentActiveAPI->GetContext();
			auto x = ImGui_ImplDX11_Init(context->Device, context->DeviceContext);
			ImGui_ImplGlfw_Init(Window::Ptr, false, GlfwClientApi_Unknown);

			return;
		}
		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;           // Enable viewports

		ImGuiInit("#version 410 core");
	}

	void ImGuiWrapper::Terminate()
	{
		//TEMPORARY
		if (Renderer::m_CurrentActiveAPI->GetContext()->GetType() == RendererType::D3D11)
		{
			ImGui_ImplDX11_Shutdown();

			return;
		}

		ImGui::DestroyPlatformWindows();
		DestroyDeviceObjects();
		Glfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiWrapper::NewFrame()
	{
		//TEMPORARY
		if (Renderer::m_CurrentActiveAPI->GetContext()->GetType() == RendererType::D3D11)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			return;
		}

		if (!FontTexture)
			CreateDeviceObjects();

		ImGuiIO& io = ImGui::GetIO();
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end."
										"Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(Window::Ptr, &w, &h);
		glfwGetFramebufferSize(Window::Ptr, &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		if (w > 0 && h > 0)
			io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
		if (WantUpdateMonitors)
			Glfw_UpdateMonitors();

		// Setup time step
		double current_time = glfwGetTime();
		io.DeltaTime = Time > 0.0 ? (float)(current_time - Time) : (float)(1.0f / 60.0f);
		Time = current_time;

		UpdateMousePosAndButtons();
		UpdateMouseCursor();

		ImGui::NewFrame();
	}

	void ImGuiWrapper::Render()
	{
		//TEMPORARY
		if (Renderer::m_CurrentActiveAPI->GetContext()->GetType() == RendererType::D3D11)
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			return;
		}

		ImGui::Render();
		Ainan::RenderDrawData(ImGui::GetDrawData());
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	static bool ImGuiInit(const char* glsl_version)
	{
		Time = 0.0;

		// Setup back-end capabilities flags
		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;    // We can create multi-viewports on the Platform side (optional)
#if GLFW_HAS_GLFW_HOVERED && defined(_WIN32)
		io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
#endif
		io.BackendPlatformName = "glfw backend";

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

		io.SetClipboardTextFn = Glfw_SetClipboardText;
		io.GetClipboardTextFn = Glfw_GetClipboardText;
		io.ClipboardUserData = (void*)Window::Ptr;

		MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
		MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
		MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

		// Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
		PrevUserCallbackMousebutton = NULL;
		PrevUserCallbackScroll = NULL;
		PrevUserCallbackKey = NULL;
		PrevUserCallbackChar = NULL;
		PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(Window::Ptr, MouseButtonCallback);
		PrevUserCallbackScroll = glfwSetScrollCallback(Window::Ptr, Glfw_ScrollCallback);
		PrevUserCallbackKey = glfwSetKeyCallback(Window::Ptr, Glfw_KeyCallback);
		PrevUserCallbackChar = glfwSetCharCallback(Window::Ptr, CharCallback);

		// Our mouse update function expect PlatformHandle to be filled for the main viewport
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		main_viewport->PlatformHandle = (void*)Window::Ptr;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			Glfw_InitPlatformInterface();

		// Setup back-end capabilities flags
		io.BackendRendererName = "imgui_impl_opengl3";

		// Store GLSL version string so we can refer to it later in case we recreate shaders. Note: GLSL version is NOT the same as GL version. Leave this to NULL if unsure.
		IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(GlslVersionString));
		strcpy(GlslVersionString, glsl_version);
		strcat(GlslVersionString, "\n");

		// Make a dummy GL call (we don't actually need the result)
		// IF YOU GET A CRASH HERE: it probably means that you haven't initialized the OpenGL function loader used by this code.
		// Desktop OpenGL 3/4 need a function loader. See the IMGUI_IMPL_OPENGL_LOADER_xxx explanation above.
		GLint current_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			InitPlatformInterface();

		return true;
	}

	static void Glfw_SetClipboardText(void* user_data, const char* text)
	{
		glfwSetClipboardString((GLFWwindow*)user_data, text);
	}

	static const char* Glfw_GetClipboardText(void* user_data)
	{
		return glfwGetClipboardString((GLFWwindow*)user_data);
	}

	static void Glfw_InitPlatformInterface()
	{
		// Register platform interface (will be coupled with a renderer interface)
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		platform_io.Platform_CreateWindow = Glfw_CreateWindow;
		platform_io.Platform_DestroyWindow = Glfw_DestroyWindow;
		platform_io.Platform_ShowWindow = Glfw_ShowWindow;
		platform_io.Platform_SetWindowPos = Glfw_SetWindowPos;
		platform_io.Platform_GetWindowPos = Glfw_GetWindowPos;
		platform_io.Platform_SetWindowSize = Glfw_SetWindowSize;
		platform_io.Platform_GetWindowSize = Glfw_GetWindowSize;
		platform_io.Platform_SetWindowFocus = Glfw_SetWindowFocus;
		platform_io.Platform_GetWindowFocus = Glfw_GetWindowFocus;
		platform_io.Platform_GetWindowMinimized = Glfw_GetWindowMinimized;
		platform_io.Platform_SetWindowTitle = Glfw_SetWindowTitle;
		platform_io.Platform_RenderWindow = Glfw_RenderWindow;
		platform_io.Platform_SwapBuffers = Glfw_SwapBuffers;
#if GLFW_HAS_WINDOW_ALPHA
		platform_io.Platform_SetWindowAlpha = ImGui_ImplGlfw_SetWindowAlpha;
#endif
#if HAS_WIN32_IME
		platform_io.Platform_SetImeInputPos = Win32_SetImeInputPos;
#endif

		// Note: monitor callback are broken GLFW 3.2 and earlier (see github.com/glfw/glfw/issues/784)
		Glfw_UpdateMonitors();
		glfwSetMonitorCallback(Glfw_MonitorCallback);

		// Register main window handle (which is owned by the main application, not by us)
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGuiViewportDataGlfw* data = IM_NEW(ImGuiViewportDataGlfw)();
		data->Window = Window::Ptr;
		data->WindowOwned = false;
		main_viewport->PlatformUserData = data;
		main_viewport->PlatformHandle = (void*)Window::Ptr;
	}

	static void UpdateMousePosAndButtons()
	{
		ImGuiIO& io = ImGui::GetIO();
		// Update buttons
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
		{
			// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
			io.MouseDown[i] = MouseJustPressed[i] || glfwGetMouseButton(Window::Ptr, i) != 0;
			MouseJustPressed[i] = false;
		}

		// Update mouse position
		const ImVec2 mouse_pos_backup = io.MousePos;
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
		io.MouseHoveredViewport = 0;

		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
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
					if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
					{
						// Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
						int window_x, window_y;
						glfwGetWindowPos(window, &window_x, &window_y);
						io.MousePos = ImVec2((float)mouse_x + window_x, (float)mouse_y + window_y);
					}
					else
					{
						// Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
						io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
					}
				}
				for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
					io.MouseDown[i] |= glfwGetMouseButton(window, i) != 0;
			}

			// (Optional) When using multiple viewports: set io.MouseHoveredViewport to the viewport the OS mouse cursor is hovering.
			// Important: this information is not easy to provide and many high-level windowing library won't be able to provide it correctly, because
			// - This is _ignoring_ viewports with the ImGuiViewportFlags_NoInputs flag (pass-through windows).
			// - This is _regardless_ of whether another viewport is focused or being dragged from.
			// If ImGuiBackendFlags_HasMouseHoveredViewport is not set by the back-end, imgui will ignore this field and infer the information by relying on the
			// rectangles and last focused time of every viewports it knows about. It will be unaware of other windows that may be sitting between or over your windows.
			// [GLFW] FIXME: This is currently only correct on Win32. See what we do below with the WM_NCHITTEST, missing an equivalent for other systems.
			// See https://github.com/glfw/glfw/issues/1236 if you want to help in making this a GLFW feature.
#if GLFW_HAS_GLFW_HOVERED && defined(_WIN32)
			if (glfwGetWindowAttrib(window, GLFW_HOVERED) && !(viewport->Flags & ImGuiViewportFlags_NoInputs))
				io.MouseHoveredViewport = viewport->ID;
#endif
		}
	}

	static void UpdateMouseCursor()
	{
		ImGuiIO& io = ImGui::GetIO();
		if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(Window::Ptr, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
			return;

		ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		for (int n = 0; n < platform_io.Viewports.Size; n++)
		{
			GLFWwindow* window = (GLFWwindow*)platform_io.Viewports[n]->PlatformHandle;
			if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
			{
				// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}
			else
			{
				// Show OS mouse cursor
				// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
				glfwSetCursor(window, MouseCursors[imgui_cursor] ? MouseCursors[imgui_cursor] : MouseCursors[ImGuiMouseCursor_Arrow]);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------
	// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
	// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
	// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
	//--------------------------------------------------------------------------------------------------------

	static void ImGui_ImplGlfw_WindowCloseCallback(GLFWwindow* window)
	{
		if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
			viewport->PlatformRequestClose = true;
	}

	static void ImGui_ImplGlfw_WindowPosCallback(GLFWwindow* window, int, int)
	{
		if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
			viewport->PlatformRequestMove = true;
	}

	static void ImGui_ImplGlfw_WindowSizeCallback(GLFWwindow* window, int, int)
	{
		if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
			viewport->PlatformRequestResize = true;
	}

	static void Glfw_CreateWindow(ImGuiViewport* viewport)
	{
		ImGuiViewportDataGlfw* data = IM_NEW(ImGuiViewportDataGlfw)();
		viewport->PlatformUserData = data;

		// GLFW 3.2 unfortunately always set focus on glfwCreateWindow() if GLFW_VISIBLE is set, regardless of GLFW_FOCUSED
		glfwWindowHint(GLFW_VISIBLE, false);
		glfwWindowHint(GLFW_FOCUSED, false);
		glfwWindowHint(GLFW_DECORATED, (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? false : true);
		GLFWwindow* share_window = Window::Ptr;
		data->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", NULL, share_window);
		data->WindowOwned = true;
		viewport->PlatformHandle = (void*)data->Window;
		glfwSetWindowPos(data->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

		// Install callbacks for secondary viewports
		glfwSetMouseButtonCallback(data->Window, MouseButtonCallback);
		glfwSetScrollCallback(data->Window, Glfw_ScrollCallback);
		glfwSetKeyCallback(data->Window, Glfw_KeyCallback);
		glfwSetCharCallback(data->Window, CharCallback);
		glfwSetWindowCloseCallback(data->Window, ImGui_ImplGlfw_WindowCloseCallback);
		glfwSetWindowPosCallback(data->Window, ImGui_ImplGlfw_WindowPosCallback);
		glfwSetWindowSizeCallback(data->Window, ImGui_ImplGlfw_WindowSizeCallback);
		glfwMakeContextCurrent(data->Window);
	}

	static void Glfw_DestroyWindow(ImGuiViewport* viewport)
	{
		if (ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData)
		{
			if (data->WindowOwned)
			{
#if GLFW_HAS_GLFW_HOVERED && defined(_WIN32)
				HWND hwnd = glfwGetWin32Window(data->Window);
				::RemovePropA(hwnd, "IMGUI_VIEWPORT");
#endif
				glfwDestroyWindow(data->Window);
			}
			data->Window = NULL;
			IM_DELETE(data);
		}
		viewport->PlatformUserData = viewport->PlatformHandle = NULL;
	}

	static void Glfw_ShowWindow(ImGuiViewport* viewport)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;

#if defined(_WIN32)
		// GLFW hack: Hide icon from task bar
		HWND hwnd = glfwGetWin32Window(data->Window);
		if (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
		{
			LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
			ex_style &= ~WS_EX_APPWINDOW;
			ex_style |= WS_EX_TOOLWINDOW;
			::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
		}

		// GLFW hack: install hook for WM_NCHITTEST message handler
#if GLFW_HAS_GLFW_HOVERED && defined(_WIN32)
		::SetPropA(hwnd, "IMGUI_VIEWPORT", viewport);
		if (g_GlfwWndProc == NULL)
			g_GlfwWndProc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcNoInputs);
#endif

		// GLFW hack: GLFW 3.2 has a bug where glfwShowWindow() also activates/focus the window.
		// The fix was pushed to GLFW repository on 2018/01/09 and should be included in GLFW 3.3 via a GLFW_FOCUS_ON_SHOW window attribute.
		// See https://github.com/glfw/glfw/issues/1189
		// FIXME-VIEWPORT: Implement same work-around for Linux/OSX in the meanwhile.
		if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
		{
			::ShowWindow(hwnd, SW_SHOWNA);
			return;
		}
#endif

		glfwShowWindow(data->Window);
	}

	static ImVec2 Glfw_GetWindowPos(ImGuiViewport* viewport)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		int x = 0, y = 0;
		glfwGetWindowPos(data->Window, &x, &y);
		return ImVec2((float)x, (float)y);
	}

	static void Glfw_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwSetWindowPos(data->Window, (int)pos.x, (int)pos.y);
	}

	static ImVec2 Glfw_GetWindowSize(ImGuiViewport* viewport)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		int w = 0, h = 0;
		glfwGetWindowSize(data->Window, &w, &h);
		return ImVec2((float)w, (float)h);
	}

	static void Glfw_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwSetWindowSize(data->Window, (int)size.x, (int)size.y);
	}

	static void Glfw_SetWindowTitle(ImGuiViewport* viewport, const char* title)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwSetWindowTitle(data->Window, title);
	}

	static void Glfw_SetWindowFocus(ImGuiViewport* viewport)
	{
#if 1
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwFocusWindow(data->Window);
#else
		// FIXME: What are the effect of not having this function? At the moment imgui doesn't actually call SetWindowFocus - we set that up ahead, will answer that question later.
		(void)viewport;
#endif
	}

	static bool Glfw_GetWindowFocus(ImGuiViewport* viewport)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		return glfwGetWindowAttrib(data->Window, GLFW_FOCUSED) != 0;
	}

	static bool Glfw_GetWindowMinimized(ImGuiViewport* viewport)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		return glfwGetWindowAttrib(data->Window, GLFW_ICONIFIED) != 0;
	}

#if GLFW_HAS_WINDOW_ALPHA
	static void ImGui_ImplGlfw_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwSetWindowOpacity(data->Window, alpha);
	}
#endif

	static void Glfw_RenderWindow(ImGuiViewport* viewport, void*)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwMakeContextCurrent(data->Window);
	}

	static void Glfw_SwapBuffers(ImGuiViewport* viewport, void*)
	{
		ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData;
		glfwMakeContextCurrent(data->Window);
		glfwSwapBuffers(data->Window);
	}

	// FIXME-PLATFORM: GLFW doesn't export monitor work area (see https://github.com/glfw/glfw/pull/989)
	static void Glfw_UpdateMonitors()
	{
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		int monitors_count = 0;
		GLFWmonitor** glfw_monitors = glfwGetMonitors(&monitors_count);
		platform_io.Monitors.resize(0);
		for (int n = 0; n < monitors_count; n++)
		{
			ImGuiPlatformMonitor monitor;
			int x, y;
			glfwGetMonitorPos(glfw_monitors[n], &x, &y);
			const GLFWvidmode* vid_mode = glfwGetVideoMode(glfw_monitors[n]);
			monitor.MainPos = monitor.WorkPos = ImVec2((float)x, (float)y);
			monitor.MainSize = monitor.WorkSize = ImVec2((float)vid_mode->width, (float)vid_mode->height);
#if GLFW_HAS_PER_MONITOR_DPI
			// Warning: the validity of monitor DPI information on Windows depends on the application DPI awareness settings, which generally needs to be set in the manifest or at runtime.
			float x_scale, y_scale;
			glfwGetMonitorContentScale(glfw_monitors[n], &x_scale, &y_scale);
			monitor.DpiScale = x_scale;
#endif
			platform_io.Monitors.push_back(monitor);
		}
		WantUpdateMonitors = false;
	}

	static void Glfw_MonitorCallback(GLFWmonitor*, int)
	{
		WantUpdateMonitors = true;
	}

	static void Glfw_ShutdownPlatformInterface()
	{
	}
	// We provide a Win32 implementation because this is such a common issue for IME users
#if defined(_WIN32) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS) && !defined(IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS) && !defined(__GNUC__)
#define HAS_WIN32_IME   1
#include <imm.h>
#ifdef _MSC_VER
#pragma comment(lib, "imm32")
#endif

	static void Win32_SetImeInputPos(ImGuiViewport* viewport, ImVec2 pos)
	{
		COMPOSITIONFORM cf = { CFS_FORCE_POSITION, { (LONG)(pos.x - viewport->Pos.x), (LONG)(pos.y - viewport->Pos.y) }, { 0, 0, 0, 0 } };
		if (ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData)
			if (HWND hwnd = glfwGetWin32Window(data->Window))
				if (HIMC himc = ::ImmGetContext(hwnd))
				{
					::ImmSetCompositionWindow(himc, &cf);
					::ImmReleaseContext(hwnd, himc);
				}
	}
#else
#define HAS_WIN32_IME   0
#endif

	static void InitPlatformInterface()
	{
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		platform_io.Renderer_RenderWindow = RenderWindow;
	}

	static void RenderWindow(ImGuiViewport* viewport, void*)
	{
		if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
		{
			ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
		}
		Ainan::RenderDrawData(viewport->DrawData);
	}

	static bool ImGui_ImplOpenGL3_CreateFontsTexture()
	{
		using namespace Ainan;
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		// Upload texture to graphics system
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &FontTexture);
		glBindTexture(GL_TEXTURE_2D, FontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->TexID = (ImTextureID)(intptr_t)FontTexture;

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);

		return true;
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		using namespace Ainan;
		if (PrevUserCallbackMousebutton != NULL && window == Window::Ptr)
			PrevUserCallbackMousebutton(window, button, action, mods);

		if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(MouseJustPressed))
			MouseJustPressed[button] = true;
	}
	
	static void Glfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		using namespace Ainan;
		if (PrevUserCallbackScroll != NULL && window == Window::Ptr)
			PrevUserCallbackScroll(window, xoffset, yoffset);
	
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += (float)xoffset;
		io.MouseWheel += (float)yoffset;
	}
	
	static void Glfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		using namespace Ainan;
		if (PrevUserCallbackKey != NULL && window == Window::Ptr)
			PrevUserCallbackKey(window, key, scancode, action, mods);
	
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
	}
	
	void CharCallback(GLFWwindow* window, unsigned int c)
	{
		using namespace Ainan;
		if (PrevUserCallbackChar != NULL && window == Window::Ptr)
			PrevUserCallbackChar(window, c);
	
		ImGuiIO& io = ImGui::GetIO();
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter((unsigned short)c);
	}



	// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
	static bool CheckShader(GLuint handle, const char* desc)
	{
		GLint status = 0, log_length = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: CreateDeviceObjects: failed to compile %s!\n", desc);
		if (log_length > 0)
		{
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}

	// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
	static bool CheckProgram(GLuint handle, const char* desc)
	{
		using namespace Ainan;

		GLint status = 0, log_length = 0;
		glGetProgramiv(handle, GL_LINK_STATUS, &status);
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: CreateDeviceObjects: failed to link %s! (with GLSL '%s')\n", desc, GlslVersionString);
		if (log_length > 0)
		{
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
		return (GLboolean)status == GL_TRUE;
	}

	static void ImGui_ImplOpenGL3_DestroyFontsTexture()
	{
		using namespace Ainan;
		if (FontTexture)
		{
			ImGuiIO& io = ImGui::GetIO();
			glDeleteTextures(1, &FontTexture);
			io.Fonts->TexID = 0;
			FontTexture = 0;
		}
	}

	static bool CreateDeviceObjects()
	{
		using namespace Ainan;

		// Backup GL state
		GLint last_texture, last_array_buffer;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

		// Parse GLSL version string
		int glsl_version = 130;
		assert(sscanf(GlslVersionString, "#version %d", &glsl_version));

		const GLchar* vertex_shader_glsl_120 = 
			R"(
			uniform mat4 ProjMtx;
			attribute vec2 Position;
			attribute vec2 UV;
			attribute vec4 Color;
			varying vec2 Frag_UV;
			varying vec4 Frag_Color;

			void main()
			{
			    Frag_UV = UV;
			    Frag_Color = Color;
			    gl_Position = ProjMtx * vec4(Position.xy,0,1);
			}
			)";

		const GLchar* vertex_shader_glsl_130 =
			R"(
			uniform mat4 ProjMtx;
			in vec2 Position;
			in vec2 UV;
			in vec4 Color;
			out vec2 Frag_UV;
			out vec4 Frag_Color;

			void main()
			{
			    Frag_UV = UV;
			    Frag_Color = Color;
			    gl_Position = ProjMtx * vec4(Position.xy,0,1);
			}
			)";

		const GLchar* vertex_shader_glsl_300_es =
			R"(
			precision mediump float;
			layout (location = 0) in vec2 Position;
			layout (location = 1) in vec2 UV;
			layout (location = 2) in vec4 Color;
			uniform mat4 ProjMtx;
			out vec2 Frag_UV;
			out vec4 Frag_Color;

			void main()
			{
			    Frag_UV = UV;
			    Frag_Color = Color;
			    gl_Position = ProjMtx * vec4(Position.xy,0,1);
			})";

		const GLchar* vertex_shader_glsl_410_core =
			R"(
			layout (location = 0) in vec2 Position;
			layout (location = 1) in vec2 UV;
			layout (location = 2) in vec4 Color;
			uniform mat4 ProjMtx;
			out vec2 Frag_UV;
			out vec4 Frag_Color;

			void main()
			{
			    Frag_UV = UV;
			    Frag_Color = Color;
			    gl_Position = ProjMtx * vec4(Position.xy,0,1);
			})";

		const GLchar* fragment_shader_glsl_120 =
			R"(
			#ifdef GL_ES
			    precision mediump float;
			#endif
			uniform sampler2D Texture;
			varying vec2 Frag_UV;
			varying vec4 Frag_Color;

			void main()
			{
			    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);
			})";

		const GLchar* fragment_shader_glsl_130 =
			R"(
			uniform sampler2D Texture;
			in vec2 Frag_UV;
			in vec4 Frag_Color;
			out vec4 Out_Color;

			void main()
			{
			    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
			})";

		const GLchar* fragment_shader_glsl_300_es =
			R"(
			precision mediump float;
			uniform sampler2D Texture;
			in vec2 Frag_UV;
			in vec4 Frag_Color;
			layout (location = 0) out vec4 Out_Color;

			void main()
			{
			    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
			})";

		const GLchar* fragment_shader_glsl_410_core =
			R"(
			in vec2 Frag_UV;
			in vec4 Frag_Color;
			uniform sampler2D Texture;
			layout (location = 0) out vec4 Out_Color;

			void main()
			{
			    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
			})";

		// Select shaders matching our GLSL versions
		const GLchar* vertex_shader = NULL;
		const GLchar* fragment_shader = NULL;
		if (glsl_version < 130)
		{
			vertex_shader = vertex_shader_glsl_120;
			fragment_shader = fragment_shader_glsl_120;
		}
		else if (glsl_version >= 410)
		{
			vertex_shader = vertex_shader_glsl_410_core;
			fragment_shader = fragment_shader_glsl_410_core;
		}
		else if (glsl_version == 300)
		{
			vertex_shader = vertex_shader_glsl_300_es;
			fragment_shader = fragment_shader_glsl_300_es;
		}
		else
		{
			vertex_shader = vertex_shader_glsl_130;
			fragment_shader = fragment_shader_glsl_130;
		}

		std::string vertSrc = (std::string)GlslVersionString + (std::string)vertex_shader;
		std::string fragSrc = (std::string)GlslVersionString + (std::string)fragment_shader;

		ImGuiShader = Renderer::CreateShaderProgramRaw(vertSrc, fragSrc);

		AttribLocationVtxPos = glGetAttribLocation(ImGuiShader->GetRendererID(), "Position");
		AttribLocationVtxUV = glGetAttribLocation(ImGuiShader->GetRendererID(), "UV");
		AttribLocationVtxColor = glGetAttribLocation(ImGuiShader->GetRendererID(), "Color");

		// Create buffers
		VertexLayout layout;
		ImGuiVertexBuffer = Renderer::CreateVertexBuffer(nullptr, 0, layout, ImGuiShader);
		ImGuiIndexBuffer = Renderer::CreateIndexBuffer(nullptr, 0);

		Ainan::ImGui_ImplOpenGL3_CreateFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);

		return true;
	}

	static void DestroyDeviceObjects()
	{
		using namespace Ainan;
		ImGuiVertexBuffer.reset();
		ImGuiIndexBuffer.reset();
		
		ImGuiShader.reset();

		Ainan::ImGui_ImplOpenGL3_DestroyFontsTexture();
	}

	static void Glfw_Shutdown()
	{
		using namespace Ainan;
		Glfw_ShutdownPlatformInterface();

		for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
		{
			glfwDestroyCursor(MouseCursors[cursor_n]);
			MouseCursors[cursor_n] = NULL;
		}
	}


	// OpenGL3 Render function.
	// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
	// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
	static void RenderDrawData(ImDrawData* draw_data)
	{
		using namespace Ainan;

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;

		// Backup GL state
		GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
		GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
		GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifdef GL_POLYGON_MODE
		GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
		Rectangle lastViewport = Renderer::GetCurrentViewport();
		Rectangle lastScissor = Renderer::GetCurrentScissor();
		GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
		GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
		GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
		GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
		GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
		bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
		GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		if (last_clip_origin == GL_UPPER_LEFT)
			clip_origin_lower_left = false;
#endif

		// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is (0,0) for single viewport apps.
		Renderer::SetViewport({ 0,0, (GLsizei)fb_width, (GLsizei)fb_height });
		float L = draw_data->DisplayPos.x;
		float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float T = draw_data->DisplayPos.y;
		float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		glm::mat4 orthoProjection =
		{
			{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
		};

		ImGuiShader->Bind();
		ImGuiShader->SetUniform1i("Texture", 0);
		ImGuiShader->SetUniformMat4("ProjMtx", orthoProjection);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

		std::shared_ptr<VertexArray> tempVA = Renderer::CreateVertexArray();
		tempVA->Bind();

		// Bind vertex/index buffers and setup attributes for ImDrawVert
		ImGuiVertexBuffer->Bind();
		ImGuiIndexBuffer->Bind();
		glEnableVertexAttribArray(AttribLocationVtxPos);
		glEnableVertexAttribArray(AttribLocationVtxUV);
		glEnableVertexAttribArray(AttribLocationVtxColor);
		glVertexAttribPointer(AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			size_t idx_buffer_offset = 0;

			// Upload vertex/index buffers
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					// User callback (registered via ImDrawList::AddCallback)
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clip_rect;
					clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
					clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
					clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
					clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

					if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
					{
						// Apply scissor/clipping rectangle
						if (clip_origin_lower_left)
							glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
						else
							glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)

						// Bind texture, Draw
						glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
						glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)idx_buffer_offset);
					}
				}
				idx_buffer_offset += pcmd->ElemCount * sizeof(ImDrawIdx);
			}
		}

		// Restore modified GL state
		glUseProgram(last_program);
		glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, last_sampler);
#endif
		glActiveTexture(last_active_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
		glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
		if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
		Renderer::SetViewport(lastViewport);
		Renderer::SetScissor(lastScissor);
	}

}
