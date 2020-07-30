#include <pch.h>
#include <glad/glad.h>

#include "OpenGLRendererAPI.h"
#include "editor/Window.h"
#include <GLFW/glfw3.h>
#include "OpenGLShaderProgram.h"
#include "OpenGLVertexBuffer.h"
#include "OpenGLIndexBuffer.h"

namespace Ainan {
	namespace OpenGL {
		//TODO these stuff really belong to the window class and not here
		bool                 MouseJustPressed[5] = { false, false, false, false, false };
		GLFWcursor* MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };
		bool                 WantUpdateMonitors = true;
		GLFWmousebuttonfun   PrevUserCallbackMousebutton = NULL;
		GLFWscrollfun        PrevUserCallbackScroll = NULL;
		GLFWkeyfun           PrevUserCallbackKey = NULL;
		GLFWcharfun          PrevUserCallbackChar = NULL;

		struct ImGuiViewportDataGlfw
		{
			GLFWwindow* Window;
			bool        WindowOwned;

			ImGuiViewportDataGlfw() { Window = NULL; WindowOwned = false; }
			~ImGuiViewportDataGlfw() { IM_ASSERT(Window == NULL); }
		};

		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
		{
			if (PrevUserCallbackMousebutton != NULL && window == Window::Ptr)
				PrevUserCallbackMousebutton(window, button, action, mods);

			if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(MouseJustPressed))
				MouseJustPressed[button] = true;
		}

		void Glfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
		{
			if (PrevUserCallbackScroll != NULL && window == Window::Ptr)
				PrevUserCallbackScroll(window, xoffset, yoffset);

			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += (float)xoffset;
			io.MouseWheel += (float)yoffset;
		}

		void Glfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
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
			if (PrevUserCallbackChar != NULL && window == Window::Ptr)
				PrevUserCallbackChar(window, c);

			ImGuiIO& io = ImGui::GetIO();
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);
		}

		void Glfw_UpdateMonitors()
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
				platform_io.Monitors.push_back(monitor);
			}
			WantUpdateMonitors = false;
		}

		void Glfw_InitPlatformInterface()
		{
			//for cleaner code
#define GET_WINDOW(x) GLFWwindow* x = ((ImGuiViewportDataGlfw*)viewport->PlatformUserData)->Window;

			// Register platform interface (will be coupled with a renderer interface)
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			platform_io.Platform_CreateWindow = [](ImGuiViewport* viewport)
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
				glfwSetWindowCloseCallback(data->Window, [](GLFWwindow* window)
					{
						if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
							viewport->PlatformRequestClose = true;
					});
				glfwSetWindowPosCallback(data->Window, [](GLFWwindow* window, int, int)
					{
						if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
							viewport->PlatformRequestMove = true;
					});
				glfwSetWindowSizeCallback(data->Window, [](GLFWwindow* window, int, int)
					{
						if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(window))
							viewport->PlatformRequestResize = true;
					}
				);
				glfwMakeContextCurrent(data->Window);
			};
			platform_io.Platform_DestroyWindow = [](ImGuiViewport* viewport)
			{
				if (ImGuiViewportDataGlfw* data = (ImGuiViewportDataGlfw*)viewport->PlatformUserData)
				{
					if (data->WindowOwned)
					{
						glfwDestroyWindow(data->Window);
					}
					data->Window = NULL;
					IM_DELETE(data);
				}
				viewport->PlatformUserData = viewport->PlatformHandle = NULL;
			};
			platform_io.Platform_ShowWindow = [](ImGuiViewport* viewport)
			{
				GET_WINDOW(window);
				glfwShowWindow(window);
			};
			platform_io.Platform_SetWindowPos = [](ImGuiViewport* viewport, ImVec2 pos)
			{
				GET_WINDOW(window);
				glfwSetWindowPos(window, (int)pos.x, (int)pos.y);
			};
			platform_io.Platform_GetWindowPos = [](ImGuiViewport* viewport)
			{
				GET_WINDOW(window);
				int x = 0, y = 0;
				glfwGetWindowPos(window, &x, &y);
				return ImVec2((float)x, (float)y);
			};
			platform_io.Platform_SetWindowSize = [](ImGuiViewport* viewport, ImVec2 size)
			{
				GET_WINDOW(window);
				glfwSetWindowSize(window, (int)size.x, (int)size.y);
			};
			platform_io.Platform_GetWindowSize = [](ImGuiViewport* viewport)
			{
				GET_WINDOW(window);
				int w = 0, h = 0;
				glfwGetWindowSize(window, &w, &h);
				return ImVec2((float)w, (float)h);
			};
			platform_io.Platform_SetWindowFocus = [](ImGuiViewport* viewport)
			{
				GET_WINDOW(window);
				glfwFocusWindow(window);
			};
			platform_io.Platform_GetWindowFocus = [](ImGuiViewport* viewport)
			{
				GET_WINDOW(window);
				return glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
			};
			platform_io.Platform_GetWindowMinimized = [](ImGuiViewport* viewport)
			{
				GET_WINDOW(window);
				return glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0;
			};
			platform_io.Platform_SetWindowTitle = [](ImGuiViewport* viewport, const char* title)
			{
				GET_WINDOW(window);
				glfwSetWindowTitle(window, title);
			};
			platform_io.Platform_RenderWindow = [](ImGuiViewport* viewport, void*)
			{
				GET_WINDOW(window);
				glfwMakeContextCurrent(window);
			};
			platform_io.Platform_SwapBuffers = [](ImGuiViewport* viewport, void*)
			{
				GET_WINDOW(window);
				glfwMakeContextCurrent(window);
				glfwSwapBuffers(window);
			};
#undef GET_WINDOW(x)

			// Note: monitor callback are broken GLFW 3.2 and earlier (see github.com/glfw/glfw/issues/784)
			Glfw_UpdateMonitors();
			glfwSetMonitorCallback([](GLFWmonitor*, int)
				{
					WantUpdateMonitors = true;
				});

			// Register main window handle (which is owned by the main application, not by us)
			ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			ImGuiViewportDataGlfw* data = IM_NEW(ImGuiViewportDataGlfw)();
			data->Window = Window::Ptr;
			data->WindowOwned = false;
			main_viewport->PlatformUserData = data;
			main_viewport->PlatformHandle = (void*)Window::Ptr;
		}


		OpenGLRendererAPI* OpenGLRendererAPI::SingletonInstance = nullptr;

		static void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
		{
			if(type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || type == GL_DEBUG_TYPE_PERFORMANCE || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
				std::cout << message << std::endl;
		};

		OpenGLRendererAPI::OpenGLRendererAPI()
		{
			glfwMakeContextCurrent(Window::Ptr);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			glfwSwapInterval(1);
#ifdef DEBUG
			glDebugMessageCallback(&opengl_debug_message_callback, nullptr);
#endif // DEBUG
			glEnable(GL_BLEND);
			SingletonInstance = this;
		}

		OpenGLRendererAPI::~OpenGLRendererAPI()
		{
			SingletonInstance = nullptr;

			//terminate ImGui
			ImGui::DestroyPlatformWindows();
			if (FontTexture)
			{
				ImGuiIO& io = ImGui::GetIO();
				glDeleteTextures(1, &FontTexture);
				io.Fonts->TexID = 0;
				FontTexture = 0;
			}
			for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
			{
				glfwDestroyCursor(MouseCursors[cursor_n]);
				MouseCursors[cursor_n] = NULL;
			}
			ImGui::DestroyContext();
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
		{
			OpenGLShaderProgram* openglShader = reinterpret_cast<OpenGLShaderProgram*>(&shader);

			glUseProgram(openglShader->m_RendererID);
			glDrawElements(GetOpenGLPrimitive(primitive), indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
			glUseProgram(0);
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer, int vertexCount)
		{
			OpenGLShaderProgram* openglShader = reinterpret_cast<OpenGLShaderProgram*>(&shader);

			glUseProgram(openglShader->m_RendererID);
			glDrawElements(GetOpenGLPrimitive(primitive), vertexCount, GL_UNSIGNED_INT, nullptr);
			glUseProgram(0);
		}

		void OpenGLRendererAPI::ClearScreen()
		{
			glClear(GL_COLOR_BUFFER_BIT);
		}

		void OpenGLRendererAPI::SetViewport(const Rectangle& viewport)
		{
			glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
		}

		Rectangle OpenGLRendererAPI::GetCurrentViewport()
		{
			Rectangle viewport;

			glGetIntegerv(GL_VIEWPORT, &viewport.X);

			return viewport;
		}

		void OpenGLRendererAPI::SetScissor(const Rectangle& scissor)
		{
			glScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
		}

		Rectangle OpenGLRendererAPI::GetCurrentScissor()
		{
			Rectangle scissor;

			glGetIntegerv(GL_SCISSOR_BOX, &scissor.X);

			return scissor;
		}

		void OpenGLRendererAPI::SetBlendMode(RenderingBlendMode blendMode)
		{
			glEnable(GL_BLEND);

			switch (blendMode)
			{
			case RenderingBlendMode::Additive:
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				break;
			case RenderingBlendMode::Screen:
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				break;
			}
		}

		void OpenGLRendererAPI::ImGuiNewFrameUI()
		{
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
			}

			if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(Window::Ptr, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
			{

			}
			else
			{
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

			ImGui::NewFrame();
		}

		void OpenGLRendererAPI::ImGuiNewFrame()
		{
			if (!FontTexture)
			{
				// Backup GL state
				GLint last_texture, last_array_buffer;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
				glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);

				const GLchar* vertexShaderSrc =
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

				const GLchar* fragmentShaderSrc =
					R"(
			in vec2 Frag_UV;
			in vec4 Frag_Color;
			layout(binding = 0) uniform sampler2D Texture;
			layout (location = 0) out vec4 Out_Color;

			void main()
			{
			    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);
			})";

				std::string vertSrc = (std::string)"#version 420 core" + (std::string)vertexShaderSrc;
				std::string fragSrc = (std::string)"#version 420 core" + (std::string)fragmentShaderSrc;

				ImGuiShader = OpenGL::OpenGLShaderProgram::CreateRaw(vertSrc, fragSrc);

				AttribLocationVtxPos = glGetAttribLocation(ImGuiShader->GetRendererID(), "Position");
				AttribLocationVtxUV = glGetAttribLocation(ImGuiShader->GetRendererID(), "UV");
				AttribLocationVtxColor = glGetAttribLocation(ImGuiShader->GetRendererID(), "Color");

				// Create buffers
				VertexLayout layout;
				ImGuiVertexBuffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(nullptr, 0, layout, false);
				ImGuiIndexBuffer = std::make_shared<OpenGL::OpenGLIndexBuffer>(nullptr, 0);

				// Build texture atlas
				ImGuiIO& io = ImGui::GetIO();
				unsigned char* pixels;
				int width, height;
				io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

				// Upload texture to graphics system
				GLint last_font_texture;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_font_texture);
				glGenTextures(1, &FontTexture);
				glBindTexture(GL_TEXTURE_2D, FontTexture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

				// Store our identifier
				io.Fonts->TexID = (ImTextureID)(intptr_t)FontTexture;

				// Restore state
				glBindTexture(GL_TEXTURE_2D, last_font_texture);

				// Restore modified GL state
				glBindTexture(GL_TEXTURE_2D, last_texture);
				glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);

			}
		}

		void OpenGLRendererAPI::ImGuiEndFrame()
		{
			DrawImGui(ImGui::GetDrawData());
		}

		void OpenGLRendererAPI::ImGuiEndFrameUI()
		{
			ImGui::Render();
		}

		void OpenGLRendererAPI::ImGuiEndFrameUI1()
		{
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				//ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}

		void OpenGLRendererAPI::ImGuiEndFrameUI2()
		{
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				//ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}

		void OpenGLRendererAPI::InitImGui()
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable viewports

			const char* glsl_version = "#version 420 core";

			// Setup back-end capabilities flags
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;    // We can create multi-viewports on the Platform side (optional)
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

			io.SetClipboardTextFn = [](void* user_data, const char* text)
			{
				return glfwSetClipboardString((GLFWwindow*)user_data, text);
			};

			io.GetClipboardTextFn = [](void* user_data)
			{
				return glfwGetClipboardString((GLFWwindow*)user_data);
			};

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
			Glfw_InitPlatformInterface();

			// Setup back-end capabilities flags
			io.BackendRendererName = "OpenGL 4.2";

			// Store GLSL version string so we can refer to it later in case we recreate shaders. Note: GLSL version is NOT the same as GL version. Leave this to NULL if unsure.
			char GlslVersionString[32] = "";
			IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(GlslVersionString));
			strcpy(GlslVersionString, glsl_version);
			strcat(GlslVersionString, "\n");

			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			platform_io.Renderer_RenderWindow = [](ImGuiViewport* viewport, void*)
			{
				if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
				{
					ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
					glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
					glClear(GL_COLOR_BUFFER_BIT);
				}
				OpenGLRendererAPI::Snigleton().DrawImGui(viewport->DrawData);
			};
		}

		void OpenGLRendererAPI::DrawImGui(ImDrawData* drawData)
		{
			// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
			int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
			int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
			if (fb_width <= 0 || fb_height <= 0)
				return;

			// Backup GL state
			GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
			glActiveTexture(GL_TEXTURE0);
			GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
			GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
			GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
			GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
			Rectangle lastViewport = GetCurrentViewport();
			Rectangle lastScissor = GetCurrentScissor();
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

			// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_SCISSOR_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// Setup viewport, orthographic projection matrix
			// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is (0,0) for single viewport apps.
			SetViewport({ 0,0, (GLsizei)fb_width, (GLsizei)fb_height });
			float L = drawData->DisplayPos.x;
			float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
			float T = drawData->DisplayPos.y;
			float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
			glm::mat4 orthoProjection =
			{
				{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
				{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
				{ 0.0f,         0.0f,        -1.0f,   0.0f },
				{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
			};

			glUseProgram(ImGuiShader->GetRendererID());
			//ImGuiShader->SetUniformMat4("ProjMtx", orthoProjection);
			glUniformMatrix4fv(glGetUniformLocation(ImGuiShader->GetRendererID(), "ProjMtx"), 1, GL_FALSE, (GLfloat*)&orthoProjection);
			glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

			VertexLayout layout;
			std::shared_ptr<VertexBuffer> tempVA =// Renderer::CreateVertexBuffer(nullptr, 0, layout, ImGuiShader);
			std::make_shared<OpenGLVertexBuffer>(nullptr, 0, layout, false);
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
			ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
			ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

			// Render command lists
			for (int n = 0; n < drawData->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = drawData->CmdLists[n];
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
			glBindSampler(0, last_sampler);
			glActiveTexture(last_active_texture);
			glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
			glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
			glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
			if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
			if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
			if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
			if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);

			SetViewport(lastViewport);
			SetScissor(lastScissor);
		}

		void OpenGLRendererAPI::SetRenderTargetApplicationWindow()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		//we don't do anything because this is handled by OpenGL, it's different for other API's
		void OpenGLRendererAPI::RecreateSwapchain(const glm::vec2& newSwapchainSize)
		{}

		void OpenGLRendererAPI::Present()
		{
			glfwSwapBuffers(Window::Ptr);
			Window::WindowSizeChangedSinceLastFrame = false;
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount)
		{
			OpenGLShaderProgram* openglShader = reinterpret_cast<OpenGLShaderProgram*>(&shader);

			glUseProgram(openglShader->m_RendererID);
			glDrawArrays(GetOpenGLPrimitive(primitive), 0, vertexCount);
			glUseProgram(0);
		}
	}
}