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
		bool                 WantUpdateMonitors = true;

		struct ImGuiViewportDataGlfw
		{
			GLFWwindow* Window;
			bool        WindowOwned;

			ImGuiViewportDataGlfw() { Window = NULL; WindowOwned = false; }
			~ImGuiViewportDataGlfw() { IM_ASSERT(Window == NULL); }
		};

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
			ImGui::DestroyContext();
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, Primitive primitive, const IndexBuffer& indexBuffer)
		{
			OpenGLShaderProgram* openglShader = reinterpret_cast<OpenGLShaderProgram*>(&shader);

			glUseProgram(openglShader->m_RendererID);
			glDrawElements(GetOpenGLPrimitive(primitive), indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
			glUseProgram(0);
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, Primitive primitive, const IndexBuffer& indexBuffer, uint32_t vertexCount)
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

		void OpenGLRendererAPI::ImGuiNewFrame()
		{
			auto func = [this]()
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

					AttribLocationVtxPos = glGetAttribLocation(ImGuiShader->m_RendererID, "Position");
					AttribLocationVtxUV = glGetAttribLocation(ImGuiShader->m_RendererID, "UV");
					AttribLocationVtxColor = glGetAttribLocation(ImGuiShader->m_RendererID, "Color");

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
			};

			Renderer::PushCommand(func);
			Renderer::WaitUntilRendererIdle();
		}

		void OpenGLRendererAPI::ImGuiEndFrame()
		{
			ImGui::Render();
			auto func = []()
			{
				glfwMakeContextCurrent(nullptr);
			};
			Renderer::PushCommand(func);
			Renderer::WaitUntilRendererIdle();

			glfwMakeContextCurrent(Window::Ptr);
			ImGui::UpdatePlatformWindows();
			glfwMakeContextCurrent(nullptr);

			ImGui::RenderPlatformWindowsDefault();
			auto func2 = [this]()
			{
				glfwMakeContextCurrent(Window::Ptr);
				DrawImGui(ImGui::GetDrawData());
			};
			Renderer::PushCommand(func2);
			Renderer::WaitUntilRendererIdle();
		}

		void OpenGLRendererAPI::InitImGui()
		{
			auto& io = ImGui::GetIO();
			// Setup back-end capabilities flags
			const char* glsl_version = "#version 420 core";
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
			Rectangle lastViewport;
			glGetIntegerv(GL_VIEWPORT, &lastViewport.X);
			Rectangle lastScissor;
			glGetIntegerv(GL_SCISSOR_BOX, &lastScissor.X);


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

			glUseProgram(ImGuiShader->m_RendererID);
			glUniformMatrix4fv(glGetUniformLocation(ImGuiShader->m_RendererID, "ProjMtx"), 1, GL_FALSE, (GLfloat*)&orthoProjection);
			glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

			VertexLayout layout;
			std::shared_ptr<VertexBuffer> tempVA = std::make_shared<OpenGLVertexBuffer>(nullptr, 0, layout, false);
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
			glScissor(lastScissor.X, lastScissor.Y, lastScissor.Width, lastScissor.Height);
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

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, Primitive primitive, uint32_t vertexCount)
		{
			OpenGLShaderProgram* openglShader = reinterpret_cast<OpenGLShaderProgram*>(&shader);

			glUseProgram(openglShader->m_RendererID);
			glDrawArrays(GetOpenGLPrimitive(primitive), 0, vertexCount);
			glUseProgram(0);
		}
	}
}