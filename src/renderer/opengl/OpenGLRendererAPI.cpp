#include "OpenGLRendererAPI.h"

#include "renderer/Renderer.h"
#include "editor/Window.h"
#include <GLFW/glfw3.h>
#include "file/AssetManager.h" //for reading shader files

namespace Ainan {
	namespace OpenGL {
		bool                 WantUpdateMonitors = true;

		struct ImGuiViewportDataGlfw
		{
			GLFWwindow* Window;
			bool        WindowOwned;

			ImGuiViewportDataGlfw() { Window = NULL; WindowOwned = false; }
			~ImGuiViewportDataGlfw() { IM_ASSERT(Window == NULL); }
		};

		static uint32_t GetBasestd140Alignemnt(ShaderVariableType type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return 4;
				break;
			case ShaderVariableType::UnsignedInt:
				return 4;
				break;
			case ShaderVariableType::Float:
				return 4;
				break;
			case ShaderVariableType::Vec2:
				return 8;
				break;
			case ShaderVariableType::Vec3:
				return 16;
				break;
			case ShaderVariableType::Vec4:
				return 16;
				break;
			case ShaderVariableType::Mat3:
				return 16;
				break;
			case ShaderVariableType::Mat4:
				return 16;
				break;

			default:
				return 4 * 4;
				break;
			}
		}

		constexpr GLenum GetOpenglTypeFromShaderType(const ShaderVariableType& type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return GL_INT;

			case ShaderVariableType::UnsignedInt:
				return GL_UNSIGNED_INT;

			case ShaderVariableType::Float:
			case ShaderVariableType::Vec2:
			case ShaderVariableType::Vec3:
			case ShaderVariableType::Vec4:
			case ShaderVariableType::Mat3:
			case ShaderVariableType::Mat4:
				return GL_FLOAT;

			default:
				return 0;
			}
		}

		static std::string LoadAndParseShader(std::string path)
		{
			std::string shader = AssetManager::ReadEntireTextFile(path);

			//parse include statements
			size_t includeLocation = 0;
			includeLocation = shader.find("#include ");
			while (includeLocation != std::string::npos)
			{
				std::filesystem::path shaderFolder = path;
				shaderFolder = shaderFolder.parent_path();

				std::string includeStatement = shader.substr(includeLocation, shader.find('\n', includeLocation) - includeLocation);
				size_t subPathBeginLocation = includeStatement.find('<');
				size_t subPathEndLocation = includeStatement.find('>');
				std::string includePath = includeStatement.substr(subPathBeginLocation + 1, subPathEndLocation - subPathBeginLocation - 1);

				std::filesystem::path fullPath = shaderFolder.string() + "/" + includePath;
				if (std::filesystem::exists(fullPath))
				{
					std::string includeFileContents = AssetManager::ReadEntireTextFile(fullPath.string());

					shader.erase(includeLocation, includeStatement.size());
					shader.insert(includeLocation, includeFileContents.c_str());
				}
				else
					assert(false);
				includeLocation = shader.find("#include ");
			}

			return shader;
		}

		OpenGLRendererAPI* OpenGLRendererAPI::SingletonInstance = nullptr;

		static void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
		{
			if (severity == GL_DEBUG_SEVERITY_HIGH)
				AINAN_LOG_ERROR(message);
			if (severity == GL_DEBUG_SEVERITY_MEDIUM)
				AINAN_LOG_WARNING(message);
		};

		OpenGLRendererAPI::OpenGLRendererAPI()
		{
			glfwMakeContextCurrent(Window::Ptr);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#ifndef NDEBUG
			glDebugMessageCallback(&opengl_debug_message_callback, nullptr);
#endif // DEBUG
			glEnable(GL_BLEND);
			glEnable(GL_MULTISAMPLE);

			//enable backface culling
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			//change the default depth clip values in window coordinates from {-1, 1} to {0, 1} 
			glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

			SingletonInstance = this;
			Context.OpenGLVersion = std::string((const char*)glGetString(GL_VERSION)).substr(0, 5);
			Context.PhysicalDeviceName = (const char*)glGetString(GL_RENDERER);
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
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
			case RenderingBlendMode::Screen:
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				break;
			case RenderingBlendMode::Overlay:
					glBlendFunc(GL_ONE, GL_ZERO);
				break;
			}
		}

		void OpenGLRendererAPI::ExecuteCommand(RenderCommand cmd)
		{
			switch (cmd.Type)
			{
			case RenderCommandType::Clear:
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				break;

			case RenderCommandType::Present:
				Present();
				break;

			case RenderCommandType::SetViewport:
				glViewport(cmd.SetViewportCmdDesc.X, cmd.SetViewportCmdDesc.Y, cmd.SetViewportCmdDesc.Width, cmd.SetViewportCmdDesc.Height);
				glDepthRange(cmd.SetViewportCmdDesc.MinDepth, cmd.SetViewportCmdDesc.MaxDepth);
				break;

			case RenderCommandType::SetBlendMode:
				SetBlendMode(cmd.SetBlendModeCmdDesc.Mode);
				break;

			case RenderCommandType::CreateUniformBuffer:
				CreateUniformBuffer(cmd);
				break;

			case RenderCommandType::UpdateUniformBuffer:
				UpdateUniformBufferNew(cmd);
				break;

			case RenderCommandType::BindUniformBuffer:
				BindUniformBufferNew(cmd);
				break;

			case RenderCommandType::DestroyUniformBuffer:
				DestroyUniformBuffer(cmd);
				break;

			case RenderCommandType::CreateShaderProgram:
				CreateShaderProgram(cmd);
				break;
			
			case RenderCommandType::DestroyShaderProgram:
				glDeleteProgram(cmd.DestroyShaderProgramCmdDesc.Program->Identifier);
				cmd.DestroyShaderProgramCmdDesc.Program->Deleted = true;
				break;

			case RenderCommandType::CreateVertexBuffer:
				CreateVertexBuffer(cmd);
				break;

			case RenderCommandType::DestroyVertexBuffer:
				DestroyVertexBuffer(cmd);
				break;

			case RenderCommandType::CreateIndexBuffer:
				CreateIndexBuffer(cmd);
				break;

			case RenderCommandType::DestroyIndexBuffer:
				DestroyIndexBuffer(cmd);
				break;

			case RenderCommandType::CreateFramebuffer:
				CreateFramebuffer(cmd);
				break;

			case RenderCommandType::BindFramebufferAsTexture:
				glActiveTexture(GL_TEXTURE0 + cmd.BindFramebufferAsTextureCmdDesc.Slot);
				glBindTexture(GL_TEXTURE_2D, cmd.BindFramebufferAsTextureCmdDesc.Buffer->TextureIdentifier);
				break;

			case RenderCommandType::ReadFramebuffer:
				ReadFramebuffer(cmd);
				break;

			case RenderCommandType::DestroyFramebuffer:
				DestroyFramebufferNew(cmd);
				break;

			case RenderCommandType::CreateTexture:
				CreateTexture(cmd);
				break;

			case RenderCommandType::DrawIndexedNew:
				DrawIndexedWithNewAPI(cmd);
				break;

			case RenderCommandType::DrawIndexedNewWithCustomNumberOfVertices:
				DrawIndexedWithCustomNumberOfVertices(cmd);
				break;

			case RenderCommandType::DrawNew:
				DrawNew(cmd);
				break;

			case RenderCommandType::UpdateVertexBuffer:
				UpdateVertexBufferNew(cmd);
				break;

			case RenderCommandType::UpdateTexture:
				UpdateTextureNew(cmd);
				break;

			case RenderCommandType::DestroyTexture:
				DestroyTexture(cmd);
				break;

			case RenderCommandType::BindTexture:
				glActiveTexture(GL_TEXTURE0 + cmd.BindTextureProgramCmdDesc.Slot);
				switch (cmd.BindTextureProgramCmdDesc.Texture->Type)
				{
				case TextureType::Texture2D:
					glBindTexture(GL_TEXTURE_2D, (uint32_t)cmd.BindTextureProgramCmdDesc.Texture->Identifier);
					break;

				case TextureType::Cubemap:
					glBindTexture(GL_TEXTURE_CUBE_MAP, (uint32_t)cmd.BindTextureProgramCmdDesc.Texture->Identifier);
					break;

				default:
					AINAN_LOG_FATAL("Unkown texture type")
				}
				break;

			case RenderCommandType::ResizeFramebuffer:
				glBindTexture(GL_TEXTURE_2D, cmd.ResizeFramebufferCmdDesc.Buffer->TextureIdentifier);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cmd.ResizeFramebufferCmdDesc.Width, cmd.ResizeFramebufferCmdDesc.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				glBindTexture(GL_TEXTURE_2D, 0);
				break;

			case RenderCommandType::BindFramebufferAsRenderTarget:
				glBindFramebuffer(GL_FRAMEBUFFER, cmd.BindFramebufferAsRenderTargetCmdDesc.Buffer->Identifier);
				break;

			case RenderCommandType::BindBackBufferAsRenderTarget:
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				break;

			default:
				break;
			}
		}

		void OpenGLRendererAPI::DrawIndexedWithNewAPI(const RenderCommand& cmd)
		{
			glBindVertexArray(cmd.DrawIndexedCmdDesc.VertexBuffer->Array);
			glBindBuffer(GL_ARRAY_BUFFER, cmd.DrawIndexedCmdDesc.VertexBuffer->Identifier);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.DrawIndexedCmdDesc.IndexBuffer->Identifier);
			glUseProgram(cmd.DrawIndexedCmdDesc.Shader->Identifier);
			glDrawElements(GetOpenGLPrimitive(cmd.DrawIndexedCmdDesc.DrawingPrimitive), cmd.DrawIndexedCmdDesc.IndexBuffer->Count, GL_UNSIGNED_INT, nullptr);
			glUseProgram(0);
		}

		void OpenGLRendererAPI::CreateFramebuffer(const RenderCommand& cmd)
		{
			FramebufferCreationInfo* info = cmd.CreateFramebufferCmdDesc.Info;
			FramebufferDataView* output = cmd.CreateFramebufferCmdDesc.Output;
			
			uint32_t bufferHandle, textureHandle;
			glGenFramebuffers(1, &bufferHandle);
			glGenTextures(1, &textureHandle);

			glBindFramebuffer(GL_FRAMEBUFFER, bufferHandle);
			glBindTexture(GL_TEXTURE_2D, textureHandle);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, info->Size.x, info->Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			output->Identifier = bufferHandle;
			output->TextureIdentifier = textureHandle;
			delete info;
		}

		void OpenGLRendererAPI::CreateShaderProgram(const RenderCommand& cmd)
		{
			ShaderProgramCreationInfo* info = cmd.CreateShaderProgramCmdDesc.Info;
			ShaderProgramDataView* output = cmd.CreateShaderProgramCmdDesc.Output;

			uint32_t vertex, fragment, program;

			vertex = glCreateShader(GL_VERTEX_SHADER);
			std::string vShaderCode = LoadAndParseShader(info->vertPath + ".vert");
			const char* c_vShaderCode = vShaderCode.c_str();
			glShaderSource(vertex, 1, &c_vShaderCode, NULL);
			glCompileShader(vertex);

			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			std::string fShaderCode = LoadAndParseShader(info->fragPath + ".frag");
			const char* c_fShaderCode = fShaderCode.c_str();
			glShaderSource(fragment, 1, &c_fShaderCode, NULL);
			glCompileShader(fragment);

			// shader Program
			program = glCreateProgram();
			glAttachShader(program, vertex);
			glAttachShader(program, fragment);

			glLinkProgram(program);
			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			output->Identifier = program;

			delete info;
		}

		void OpenGLRendererAPI::CreateUniformBuffer(const RenderCommand& cmd)
		{
			UniformBufferCreationInfo* info = cmd.CreateUniformBufferCmdDesc.Info;
			UniformBufferDataView* output = cmd.CreateUniformBufferCmdDesc.Output;

			output->PackedSize = std::accumulate(info->layout.begin(), info->layout.end(), 0,
				[](const uint32_t& a, const VertexLayoutElement& b)
				{
					return a + b.GetSize();
				});

			//calculate std140 layout size
			for (auto& layoutPart : info->layout)
			{
				uint32_t size = layoutPart.GetSize();

				//if its an array
				if (layoutPart.Count > 1)
				{
					for (size_t i = 0; i < layoutPart.Count; i++)
					{
						output->AlignedSize += output->AlignedSize % 16 == 0 ? 0 : 16 - (output->AlignedSize % 16);
						output->AlignedSize += size / layoutPart.Count;
					}
				}
				else
				{
					uint32_t baseAlignment = GetBasestd140Alignemnt(layoutPart.Type);
					output->AlignedSize += output->AlignedSize % baseAlignment == 0 ? 0 : baseAlignment - (output->AlignedSize % baseAlignment);
					output->AlignedSize += size;
				}
			}
			output->AlignedSize += output->AlignedSize % 16 == 0 ? 0 : 16 - (output->AlignedSize % 16);
			output->BufferMemory = new uint8_t[output->AlignedSize]();

			uint32_t identifier = 0;
			glGenBuffers(1, &identifier);
			glBindBuffer(GL_UNIFORM_BUFFER, identifier);
			glBufferData(GL_UNIFORM_BUFFER, output->AlignedSize, nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
			output->Identifier = identifier;

			output->Layout = info->layout;
			delete info;
		}

		void OpenGLRendererAPI::DestroyUniformBuffer(const RenderCommand& cmd)
		{
			uint32_t buffer = cmd.DestroyUniformBufferCmdDesc.Buffer->Identifier;
			glDeleteBuffers(1, &buffer);
			cmd.DestroyUniformBufferCmdDesc.Buffer->Deleted = true;
		}

		void OpenGLRendererAPI::CreateIndexBuffer(const RenderCommand& cmd)
		{
			IndexBufferCreationInfo* info = cmd.CreateIndexBufferCmdDesc.Info;
			IndexBufferDataView* output = cmd.CreateIndexBufferCmdDesc.Output;

			uint32_t bufferHandle = 0;
			glGenBuffers(1, &bufferHandle);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, info->Count * sizeof(uint32_t), info->InitialData, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			output->Identifier = bufferHandle;
			delete info->InitialData;
			delete info;
		}

		void OpenGLRendererAPI::UpdateUniformBufferNew(const RenderCommand& cmd)
		{
			UniformBufferDataView* buffer = cmd.UpdateUniformBufferCmdDesc.Buffer;

			//align data with std140 uniform layouts and put the result in m_BufferMemory
			uint32_t unalignedDataIndex = 0;
			uint32_t alignedDataIndex = 0;
			uint8_t* unalignedData = (uint8_t*)cmd.UpdateUniformBufferCmdDesc.Data;
			for (auto& layoutPart : buffer->Layout)
			{
				uint32_t size = layoutPart.GetSize();
				if (layoutPart.Count > 1)
				{
					for (size_t i = 0; i < layoutPart.Count; i++)
					{
						alignedDataIndex += alignedDataIndex % 16 == 0 ? 0 : 16 - (alignedDataIndex % 16);
						memcpy(&buffer->BufferMemory[alignedDataIndex], &unalignedData[unalignedDataIndex], size / layoutPart.Count);
						unalignedDataIndex += size / layoutPart.Count;
						alignedDataIndex += size / layoutPart.Count;
					}
				}
				else
				{
					uint32_t baseAlignment = GetBasestd140Alignemnt(layoutPart.Type);

					alignedDataIndex += alignedDataIndex % baseAlignment == 0 ? 0 : baseAlignment - (alignedDataIndex % baseAlignment);
					memcpy(&buffer->BufferMemory[alignedDataIndex], &unalignedData[unalignedDataIndex], size);
					alignedDataIndex += size;
					unalignedDataIndex += size;
				}
			}

			glBindBuffer(GL_UNIFORM_BUFFER, buffer->Identifier);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, buffer->AlignedSize, buffer->BufferMemory);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			delete[] unalignedData;
		}

		void OpenGLRendererAPI::BindUniformBufferNew(const RenderCommand& cmd)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, cmd.BindUniformBufferCmdDesc.Slot,
				cmd.BindUniformBufferCmdDesc.Buffer->Identifier, 0, cmd.BindUniformBufferCmdDesc.Buffer->AlignedSize);
		}

		void OpenGLRendererAPI::CreateVertexBuffer(const RenderCommand& cmd)
		{
			VertexBufferCreationInfo* info = cmd.CreateVertexBufferCmdDesc.Info;
			VertexBufferDataView* output = cmd.CreateVertexBufferCmdDesc.Output;

			uint32_t arrayHandle = 0, bufferHandle = 0;
			glGenVertexArrays(1, &arrayHandle);
			glBindVertexArray(arrayHandle);

			//create buffer
			glBindVertexArray(arrayHandle);
			glGenBuffers(1, &bufferHandle);
			glBindBuffer(GL_ARRAY_BUFFER, bufferHandle);
			if (info->Dynamic)
				glBufferData(GL_ARRAY_BUFFER, info->Size, info->InitialData, GL_DYNAMIC_DRAW);
			else
				glBufferData(GL_ARRAY_BUFFER, info->Size, info->InitialData, GL_STATIC_DRAW);

			//set layout
			int32_t index = 0;
			int32_t offset = 0;
			int32_t stride = 0;

			for (auto& layoutPart : info->Layout)
			{
				stride += layoutPart.GetSize();
			}

			for (auto& layoutPart : info->Layout)
			{
				int32_t size = layoutPart.GetSize();
				int32_t componentCount = GetShaderVariableComponentCount(layoutPart.Type);
				GLenum openglType = GetOpenglTypeFromShaderType(layoutPart.Type);

				glVertexAttribPointer(index, componentCount, openglType, false, stride, (void*)(uintptr_t)offset);
				offset += size;

				glEnableVertexAttribArray(index);
				index++;
			}

			output->Array = arrayHandle;
			output->Identifier = bufferHandle;
			delete info->InitialData;
			delete info;
		}

		void OpenGLRendererAPI::DestroyVertexBuffer(const RenderCommand& cmd)
		{
			uint32_t varray = cmd.DestroyVertexBufferCmdDesc.Buffer->Array;
			uint32_t buffer = cmd.DestroyVertexBufferCmdDesc.Buffer->Identifier;
			glDeleteVertexArrays(1, &varray);
			glDeleteBuffers(1, &buffer);
			cmd.DestroyVertexBufferCmdDesc.Buffer->Deleted = true;
		}

		void OpenGLRendererAPI::DestroyIndexBuffer(const RenderCommand& cmd)
		{
			uint32_t buffer = cmd.DestroyIndexBufferCmdDesc.Buffer->Identifier;
			glDeleteBuffers(1, &buffer);
			cmd.DestroyIndexBufferCmdDesc.Buffer->Deleted = true;
		}

		void OpenGLRendererAPI::UpdateVertexBufferNew(const RenderCommand& cmd)
		{
			glBindBuffer(GL_ARRAY_BUFFER, cmd.UpdateVertexBufferCmdDesc.VertexBuffer->Identifier);
			glBindVertexArray(cmd.UpdateVertexBufferCmdDesc.VertexBuffer->Array);
			glBufferSubData(GL_ARRAY_BUFFER, cmd.UpdateVertexBufferCmdDesc.Offset, cmd.UpdateVertexBufferCmdDesc.Size, cmd.UpdateVertexBufferCmdDesc.Data);

			delete[] cmd.UpdateVertexBufferCmdDesc.Data;
		}

		void OpenGLRendererAPI::DrawIndexedWithCustomNumberOfVertices(const RenderCommand& cmd)
		{
			glBindVertexArray(cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.VertexBuffer->Array);
			glBindBuffer(GL_ARRAY_BUFFER, cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.VertexBuffer->Identifier);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.IndexBuffer->Identifier);
			glUseProgram(cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.Shader->Identifier);
			glDrawElements(GetOpenGLPrimitive(cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.DrawingPrimitive),
				cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.IndexCount, GL_UNSIGNED_INT, nullptr);
		}

		void OpenGLRendererAPI::ReadFramebuffer(const RenderCommand& cmd)
		{
			Image* img = cmd.ReadFramebufferCmdDesc.Output;
			FramebufferDataView* buffer = cmd.ReadFramebufferCmdDesc.Buffer;
			glm::vec2 bottomLeftPixel = { cmd.ReadFramebufferCmdDesc.BottomLeftX, cmd.ReadFramebufferCmdDesc.BottomLeftY };
			glm::vec2 topRightPixel = { cmd.ReadFramebufferCmdDesc.TopRightX, cmd.ReadFramebufferCmdDesc.TopRightY };

			img->m_Width = (uint32_t)buffer->Size.x;
			img->m_Height = (uint32_t)buffer->Size.y;
			img->m_Data = new uint8_t[(uint32_t)(buffer->Size.x * buffer->Size.y * 4)];
			img->Format = TextureFormat::RGBA;

			glBindFramebuffer(GL_FRAMEBUFFER, buffer->Identifier);
			glReadPixels(
				bottomLeftPixel.x,
				bottomLeftPixel.y,
				topRightPixel.x == 0 ? buffer->Size.x : topRightPixel.x,
				topRightPixel.y == 0 ? buffer->Size.y : topRightPixel.y,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				img->m_Data);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void OpenGLRendererAPI::DestroyFramebufferNew(const RenderCommand& cmd)
		{
			uint32_t buffer = cmd.DestroyFramebufferCmdDesc.Buffer->Identifier;
			uint32_t texture = cmd.DestroyFramebufferCmdDesc.Buffer->TextureIdentifier;
			glDeleteFramebuffers(1, &buffer);
			glDeleteTextures(1, &texture);
			cmd.DestroyFramebufferCmdDesc.Buffer->Deleted = true;
		}

		void OpenGLRendererAPI::CreateTexture(const RenderCommand& cmd)
		{
			TextureCreationInfo* info = cmd.CreateTextureProgramCmdDesc.Info;
			TextureDataView* output = cmd.CreateTextureProgramCmdDesc.Output;

			uint32_t textureHandle = 0;
			glGenTextures(1, &textureHandle);

			int32_t glInternalFormat = 0;
			int32_t glFormat = 0;
			switch (info->Format)
			{
			case TextureFormat::RGBA:
				glInternalFormat = GL_RGB8;
				glFormat = GL_RGBA;
				break;

			case TextureFormat::RGB:
				glInternalFormat = GL_RGB8;
				glFormat = GL_RGB;
				break;

			case TextureFormat::RG:
				glInternalFormat = GL_RG8;
				glFormat = GL_RG;
				break;

			case TextureFormat::R:
				glInternalFormat = GL_R8;
				glFormat = GL_RED;
				break;

			case TextureFormat::Unspecified:
			default:
				AINAN_LOG_FATAL("Unknown texture format specified");
			}

			switch (info->Type)
			{
			case TextureType::Texture2D:
				glBindTexture(GL_TEXTURE_2D, textureHandle);
				glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, info->Size.x, info->Size.y, 0, glFormat, GL_UNSIGNED_BYTE, info->InitialData);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glGenerateMipmap(GL_TEXTURE_2D);

				break;

			case TextureType::Cubemap:
				glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
				for (uint32_t i = 0; i < 6; i++)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, info->Size.x, info->Size.y, 0, glFormat, GL_UNSIGNED_BYTE,
						info->InitialData + (int32_t)i * (int32_t)info->Size.x * (int32_t)info->Size.y * GetBytesPerPixel(info->Format));
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				break;

			case TextureType::Unspecified:
			default:
				AINAN_LOG_FATAL("Unkown texture type specified");
			}

			output->Identifier = textureHandle;
			output->Size = info->Size;

			delete[] info->InitialData;
			delete info;
		}

		void OpenGLRendererAPI::UpdateTextureNew(const RenderCommand& cmd)
		{
			uint32_t width = cmd.UpdateTextureCmdDesc.Width;
			uint32_t height = cmd.UpdateTextureCmdDesc.Height;
			void* data = cmd.UpdateTextureCmdDesc.Data;

			int32_t glInternalFormat = 0;
			int32_t glFormat = 0;
			switch (cmd.UpdateTextureCmdDesc.Format)
			{
			case TextureFormat::RGBA:
				glInternalFormat = GL_RGB8;
				glFormat = GL_RGBA;
				break;

			case TextureFormat::RGB:
				glInternalFormat = GL_RGB8;
				glFormat = GL_RGB;
				break;

			case TextureFormat::RG:
				glInternalFormat = GL_RG8;
				glFormat = GL_RG;
				break;

			case TextureFormat::R:
				glInternalFormat = GL_R8;
				glFormat = GL_RED;
				break;

			case TextureFormat::Unspecified:
			default:
				AINAN_LOG_FATAL("Unknown texture format specified");
			}

			switch (cmd.UpdateTextureCmdDesc.Texture->Type)
			{
			case TextureType::Texture2D:
				glBindTexture(GL_TEXTURE_2D, cmd.UpdateTextureCmdDesc.Texture->Identifier);
				glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;

			case TextureType::Cubemap:
				glBindTexture(GL_TEXTURE_CUBE_MAP, cmd.UpdateTextureCmdDesc.Texture->Identifier);
				for (uint32_t i = 0; i < 6; i++)
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE,
						(uint8_t*)data + (int32_t)i * width * height * GetBytesPerPixel(cmd.UpdateTextureCmdDesc.Format));
				break;

			case TextureType::Unspecified:
			default:
				AINAN_LOG_FATAL("Unkown texture type specified");
			}
			delete[] data;
		}

		void OpenGLRendererAPI::DestroyTexture(const RenderCommand& cmd)
		{
			uint32_t tex = cmd.DestroyTextureCmdDesc.Texture->Identifier;
			glDeleteTextures(1, &tex);
			cmd.DestroyTextureCmdDesc.Texture->Deleted = true;
		}

		void OpenGLRendererAPI::DrawNew(const RenderCommand& cmd)
		{
			glUseProgram(cmd.DrawNewCmdDesc.Shader->Identifier);
			glBindVertexArray(cmd.DrawNewCmdDesc.VertexBuffer->Array);
			glBindBuffer(GL_ARRAY_BUFFER, cmd.DrawNewCmdDesc.VertexBuffer->Identifier);

			glDrawArrays(GetOpenGLPrimitive(cmd.DrawNewCmdDesc.DrawingPrimitive), 0, cmd.DrawNewCmdDesc.VertexCount);

			glUseProgram(0);
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
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

					uint32_t vertex, fragment;

					vertex = glCreateShader(GL_VERTEX_SHADER);
					const char* c_vShaderCode = vertSrc.c_str();
					glShaderSource(vertex, 1, &c_vShaderCode, NULL);
					glCompileShader(vertex);


					fragment = glCreateShader(GL_FRAGMENT_SHADER);
					const char* c_fShaderCode = fragSrc.c_str();
					glShaderSource(fragment, 1, &c_fShaderCode, NULL);
					glCompileShader(fragment);


					// shader Program
					ImGuiShader.Identifier = glCreateProgram();
					glAttachShader(ImGuiShader.Identifier, vertex);
					glAttachShader(ImGuiShader.Identifier, fragment);

					glLinkProgram(ImGuiShader.Identifier);
					// delete the shaders as they're linked into our program now and no longer necessery
					glDeleteShader(vertex);
					glDeleteShader(fragment);

					AttribLocationVtxPos = glGetAttribLocation(ImGuiShader.Identifier, "Position");
					AttribLocationVtxUV = glGetAttribLocation(ImGuiShader.Identifier, "UV");
					AttribLocationVtxColor = glGetAttribLocation(ImGuiShader.Identifier, "Color");

					// Create buffers
					VertexLayout layout;
					{
						RenderCommand cmd;
						cmd.Type = RenderCommandType::CreateVertexBuffer;
						auto vBufferInfo = new VertexBufferCreationInfo;
						vBufferInfo->InitialData = nullptr;
						vBufferInfo->Size = 0;
						vBufferInfo->Layout = layout;
						vBufferInfo->Shader = &ImGuiShader;
						vBufferInfo->Dynamic = false;
						cmd.CreateVertexBufferCmdDesc.Info = vBufferInfo;
						cmd.CreateVertexBufferCmdDesc.Output = &ImGuiVertexBuffer;
						CreateVertexBuffer(cmd);
					}

					{
						RenderCommand cmd;
						cmd.Type = RenderCommandType::CreateIndexBuffer;
						auto vBufferInfo = new IndexBufferCreationInfo;
						vBufferInfo->InitialData = nullptr;
						vBufferInfo->Count = 0;
						cmd.CreateIndexBufferCmdDesc.Info = vBufferInfo;
						cmd.CreateIndexBufferCmdDesc.Output = &ImGuiIndexBuffer;
						CreateIndexBuffer(cmd);
					}

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

		void OpenGLRendererAPI::ImGuiEndFrame(bool redraw)
		{
			auto func = []()
			{
				glfwMakeContextCurrent(nullptr);
			};
			Renderer::PushCommand(func);
			Renderer::WaitUntilRendererIdle();

			glfwMakeContextCurrent(Window::Ptr);
			ImGui::UpdatePlatformWindows();
			glfwMakeContextCurrent(nullptr);
			if (!redraw)
			{
				auto func2 = [this]()
				{
					glfwMakeContextCurrent(Window::Ptr);
				};
				Renderer::PushCommand(func2);
				Renderer::WaitUntilRendererIdle();
				return;
			}

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

		void OpenGLRendererAPI::TerminateImGui()
		{
			{
				RenderCommand cmd;
				cmd.Type = RenderCommandType::DestroyShaderProgram;
				cmd.DestroyShaderProgramCmdDesc.Program = &ImGuiShader;
				Renderer::PushCommand(cmd);
			}

			{
				RenderCommand cmd;
				cmd.Type = RenderCommandType::DestroyVertexBuffer;
				cmd.DestroyVertexBufferCmdDesc.Buffer = &ImGuiVertexBuffer;
				Renderer::PushCommand(cmd);
			}

			{
				RenderCommand cmd;
				cmd.Type = RenderCommandType::DestroyIndexBuffer;
				cmd.DestroyIndexBufferCmdDesc.Buffer = &ImGuiIndexBuffer;
				Renderer::PushCommand(cmd);
			}
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

			glUseProgram(ImGuiShader.Identifier);
			glUniformMatrix4fv(glGetUniformLocation(ImGuiShader.Identifier, "ProjMtx"), 1, GL_FALSE, (GLfloat*)&orthoProjection);
			glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.

			uint32_t tempVA = 0;
			glGenVertexArrays(1, &tempVA);
			glBindVertexArray(tempVA);

			// Bind vertex/index buffers and setup attributes for ImDrawVert
			glBindVertexArray(ImGuiVertexBuffer.Array);
			glBindBuffer(GL_ARRAY_BUFFER, ImGuiVertexBuffer.Identifier);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ImGuiIndexBuffer.Identifier);
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
			glDeleteVertexArrays(1, &tempVA);
		}

		void OpenGLRendererAPI::Present()
		{
			glfwSwapBuffers(Window::Ptr);
			Window::WindowSizeChangedSinceLastFrame = false;
		}
	}
}
