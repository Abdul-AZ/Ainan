#include <pch.h>

#include "Renderer.h"

#include "opengl/OpenGLRendererAPI.h"
#include "opengl/OpenGLShaderProgram.h"
#include "opengl/OpenGLVertexBuffer.h"
#include "opengl/OpenGLIndexBuffer.h"
#include "opengl/OpenGLTexture.h"
#include "opengl/OpenGLFrameBuffer.h"
#include "opengl/OpenGLUniformBuffer.h"

#ifdef PLATFORM_WINDOWS

#include "d3d11/D3D11RendererAPI.h"
#include "d3d11/D3D11ShaderProgram.h"
#include "d3d11/D3D11VertexBuffer.h"
#include "d3d11/D3D11IndexBuffer.h"
#include "d3d11/D3D11UniformBuffer.h"
#include "d3d11/D3D11Texture.h"
#include "d3d11/D3D11FrameBuffer.h"

#endif // PLATFORM_WINDOWS

#include <GLFW/glfw3.h>

namespace Ainan {

	Renderer::RendererData* Renderer::Rdata = nullptr;

	struct ShaderLoadInfo
	{
		std::string Name;
		std::string VertexCodePath;
		std::string FragmentCodePath;
	};

	//shaders that are compiled on renderer initilization
	std::vector<ShaderLoadInfo> CompileOnInit =
	{
		//name                  //vertex shader                //fragment shader
		{ "BackgroundShader"    , "shaders/Background"    , "shaders/Background"     },
		{ "CircleOutlineShader" , "shaders/CircleOutline" , "shaders/CircleOutline"  },
		{ "LineShader"          , "shaders/Line"          , "shaders/FlatColor"      },
		{ "BlurShader"          , "shaders/Image"         , "shaders/Blur"           },
		{ "GizmoShader"         , "shaders/Gizmo"         , "shaders/Gizmo"          },
		{ "GridShader"          , "shaders/Grid"          , "shaders/Grid"           },
		{ "ImageShader"         , "shaders/Image"         , "shaders/Image"          },
		{ "QuadBatchShader"     , "shaders/QuadBatch"     , "shaders/QuadBatch"      }
	};

	void Renderer::Init(RendererType api)
	{
		Rdata = new RendererData();
		Rdata->Thread = std::thread(&Renderer::RendererThreadLoop, api);

		WaitUntilRendererIdle();
	}

	void Renderer::Terminate()
	{
		Rdata->DestroyThread = true;
		Rdata->cv.notify_all();
		Rdata->Thread.join();
		delete Rdata;
	}

	void Renderer::RendererThreadLoop(RendererType api)
	{
		{
			std::lock_guard lock(Rdata->DataMutex);

			//initilize the renderer api
			switch (api)
			{
			case RendererType::D3D11:
				Rdata->CurrentActiveAPI = new D3D11::D3D11RendererAPI();
				break;

			case RendererType::OpenGL:
				Rdata->CurrentActiveAPI = new OpenGL::OpenGLRendererAPI();
				break;
			}

			//load shaders
			for (auto& shaderInfo : CompileOnInit)
			{
				Rdata->ShaderLibrary[shaderInfo.Name] = Renderer::CreateShaderProgram(shaderInfo.VertexCodePath, shaderInfo.FragmentCodePath);
			}

			//setup batch renderer
			{
				VertexLayout layout(4);
				layout[0] = { "aPos", ShaderVariableType::Vec2 };
				layout[1] = { "aColor", ShaderVariableType::Vec4 };
				layout[2] = { "aTexture", ShaderVariableType::Float };
				layout[3] = { "aTexCoords", ShaderVariableType::Vec2 };

				switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
				{
				case RendererType::OpenGL:
					Rdata->QuadBatchVertexBuffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex), layout, true);
					break;

#ifdef PLATFORM_WINDOWS
				case RendererType::D3D11:
					Rdata->QuadBatchVertexBuffer = std::make_shared<D3D11::D3D11VertexBuffer>(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex), layout, Rdata->ShaderLibrary["QuadBatchShader"], true, Rdata->CurrentActiveAPI->GetContext());
					break;
#endif // PLATFORM_WINDOWS

				default:
					assert(false);
				}
				Rdata->ReservedVertexBuffers.push_back(Rdata->QuadBatchVertexBuffer);
			}

			uint32_t* indicies = new uint32_t[c_MaxQuadsPerBatch * 6];
			int u = 0;
			for (size_t i = 0; i < c_MaxQuadsPerBatch * 6; i += 6)
			{
				indicies[i + 0] = 0 + u;
				indicies[i + 1] = 1 + u;
				indicies[i + 2] = 2 + u;

				indicies[i + 3] = 0 + u;
				indicies[i + 4] = 2 + u;
				indicies[i + 5] = 3 + u;
				u += 4;
			}
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				Rdata->QuadBatchIndexBuffer = std::make_shared<OpenGL::OpenGLIndexBuffer>(indicies, c_MaxQuadsPerBatch * 6);
				break;

			case RendererType::D3D11:
				Rdata->QuadBatchIndexBuffer = std::make_shared<D3D11::D3D11IndexBuffer>(indicies, c_MaxQuadsPerBatch * 6, Rdata->CurrentActiveAPI->GetContext());
				break;

			default:
				assert(false);
			}
			Rdata->ReservedIndexBuffers.push_back(Rdata->QuadBatchIndexBuffer);
			delete[] indicies;

			Rdata->QuadBatchVertexBufferDataOrigin = new QuadVertex[c_MaxQuadVerticesPerBatch];
			Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;

			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				Rdata->QuadBatchTextures[0] = std::make_shared<OpenGL::OpenGLTexture>(glm::vec2(1, 1), TextureFormat::RGBA, nullptr);
				break;

#ifdef PLATFORM_WINDOWS
			case RendererType::D3D11:
				Rdata->QuadBatchTextures[0] = std::make_shared<D3D11::D3D11Texture>(glm::vec2(1, 1), TextureFormat::RGBA, nullptr, Rdata->CurrentActiveAPI->GetContext());
				break;
#endif // PLATFORM_WINDOWS

			default:
				assert(false);
			}
			Rdata->ReservedTextures.push_back(Rdata->QuadBatchTextures[0]);
			auto img = std::make_shared<Image>();
			img->m_Width = 1;
			img->m_Height = 1;
			img->Format = TextureFormat::RGBA;
			img->m_Data = new uint8_t[4];
			memset(img->m_Data, (uint8_t)255, 4);
			Rdata->QuadBatchTextures[0]->SetImageUnsafe(img);

			//setup postprocessing
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				Rdata->BlurFrameBuffer = std::make_shared<OpenGL::OpenGLFrameBuffer>(Window::FramebufferSize);
				break;
#ifdef PLATFORM_WINDOWS

			case RendererType::D3D11:
				Rdata->BlurFrameBuffer = std::make_shared<D3D11::D3D11FrameBuffer>(Window::FramebufferSize, Rdata->CurrentActiveAPI->GetContext());
#endif
			}

			float quadVertices[24];
			{
				switch (api)
				{
				case Ainan::RendererType::OpenGL:
				{
					float openglVertices[] = {
						-1.0f, -1.0f, 0.0f, 0.0f,
						-1.0f, 1.0f, 0.0f, 1.0f,
						1.0f, -1.0f, 1.0f, 0.0f,

						-1.0f, 1.0f, 0.0f, 1.0f,
						1.0f, 1.0f, 1.0f, 1.0f,
						1.0f, -1.0f, 1.0f, 0.0f
					};
					memcpy(quadVertices, openglVertices, sizeof(quadVertices));
					break;
				}

				case Ainan::RendererType::D3D11:
				{
					float d3dVertices[] = {
						// positions   // texCoords
						-1.0f,  1.0f,  0.0f, 0.0f,
						 1.0f, -1.0f,  1.0f, 1.0f,
						-1.0f, -1.0f,  0.0f, 1.0f,

						-1.0f,  1.0f,  0.0f, 0.0f,
						 1.0f,  1.0f,  1.0f, 0.0f,
						 1.0f, -1.0f,  1.0f, 1.0f
					};
					memcpy(quadVertices, d3dVertices, sizeof(quadVertices));
					break;
				}
				}
			}


			{
				VertexLayout layout(2);
				layout[0] = { "aPos", ShaderVariableType::Vec2 };
				layout[1] = { "aTexCoords", ShaderVariableType::Vec2 };
				switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
				{
				case RendererType::OpenGL:
					Rdata->BlurVertexBuffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(quadVertices, sizeof(quadVertices), layout, false);
					break;

#ifdef PLATFORM_WINDOWS
				case RendererType::D3D11:
					Rdata->BlurVertexBuffer = std::make_shared<D3D11::D3D11VertexBuffer>(quadVertices, sizeof(quadVertices), layout, Rdata->ShaderLibrary["BlurShader"], false, Rdata->CurrentActiveAPI->GetContext());
					break;
#endif // PLATFORM_WINDOWS

				default:
					assert(false);
				}
				Rdata->ReservedVertexBuffers.push_back(Rdata->BlurVertexBuffer);
			}
			{
				VertexLayout bufferLayout =
				{
					{ "u_Resolution", ShaderVariableType::Vec2 },
					{ "u_BlurDirection", ShaderVariableType::Vec2 },
					{ "u_Radius", ShaderVariableType::Float }
				};
				switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
				{
				case RendererType::OpenGL:
					Rdata->BlurUniformBuffer = std::make_shared<OpenGL::OpenGLUniformBuffer>("BlurData", bufferLayout, nullptr);
					break;

#ifdef PLATFORM_WINDOWS
				case RendererType::D3D11:
					Rdata->BlurUniformBuffer = std::make_shared<D3D11::D3D11UniformBuffer>("BlurData", 1, bufferLayout, nullptr, Rdata->CurrentActiveAPI->GetContext());
					break;
#endif // PLATFORM_WINDOWS

				default:
					assert(false);
				}
				Rdata->ShaderLibrary["BlurShader"]->BindUniformBufferUnsafe(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);
			}

			Rdata->CurrentActiveAPI->SetBlendMode(Rdata->m_CurrentBlendMode);

			{
				VertexLayout bufferLayout =
				{
					{ "u_ViewProjection", ShaderVariableType::Mat4 }
				};
				switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
				{
				case RendererType::OpenGL:
					Rdata->SceneUniformbuffer = std::make_shared<OpenGL::OpenGLUniformBuffer>("FrameData", bufferLayout, nullptr);
					break;

#ifdef PLATFORM_WINDOWS
				case RendererType::D3D11:
					Rdata->SceneUniformbuffer = std::make_shared<D3D11::D3D11UniformBuffer>("FrameData", 0, bufferLayout, nullptr, Rdata->CurrentActiveAPI->GetContext());
					break;
#endif // PLATFORM_WINDOWS

				default:
					assert(false);
				}
				for (auto& shaderTuple : Rdata->ShaderLibrary)
				{
					shaderTuple.second->BindUniformBufferUnsafe(Rdata->SceneUniformbuffer, 0, RenderingStage::VertexShader);
				}
			}

			Rdata->CurrentActiveAPI->InitImGui();
		}

		while (true)
		{
			if (Rdata->DestroyThread)
				break;

			{
				{
					std::unique_lock<std::mutex> lock(Rdata->QueueMutex);
					Rdata->cv.wait(lock, []() { return Rdata->payload == true || Rdata->DestroyThread; });
				}
				std::function<void()> func = nullptr;
				while (Rdata->CommandBuffer.size() > 0)
				{
					{
						std::unique_lock lock(Rdata->QueueMutex);
						func = Rdata->CommandBuffer.front();
						Rdata->CommandBuffer.pop();
					}
					func();
				}
				Rdata->payload = false;
				Rdata->WorkDoneCV.notify_all();
			}
		}

		Rdata->ShaderLibrary.erase(Rdata->ShaderLibrary.begin(), Rdata->ShaderLibrary.end());

		auto type = Rdata->CurrentActiveAPI->GetContext()->GetType();

		delete Rdata->CurrentActiveAPI;

		//batch renderer data
		Rdata->QuadBatchVertexBuffer.reset();
		Rdata->QuadBatchIndexBuffer.reset();
		delete[] Rdata->QuadBatchVertexBufferDataOrigin;
		Rdata->QuadBatchTextures[0].reset();
	}

	void Renderer::PushCommand(std::function<void()> func)
	{
		if (Window::Minimized)
			return;

		std::unique_lock lock(Rdata->QueueMutex);
		Rdata->CommandBuffer.push(func);
		Rdata->payload = true;
		Rdata->cv.notify_one();
	}

	void Renderer::BeginScene(const SceneDescription& desc)
	{
		auto func = [desc]()
		{
			assert(desc.SceneDrawTarget);

			Rdata->CurrentSceneDescription = desc;
			Rdata->CurrentViewProjection = desc.SceneCamera.ProjectionMatrix * desc.SceneCamera.ViewMatrix;
			Rdata->CurrentNumberOfDrawCalls = 0;

			(*Rdata->CurrentSceneDescription.SceneDrawTarget)->BindUnsafe();
			ClearScreenUnsafe();

			//update the per-frame uniform buffer
			Rdata->SceneUniformbuffer->UpdateDataUnsafe(&Rdata->CurrentViewProjection);

			//update diagnostics stuff
			for (size_t i = 0; i < Rdata->ReservedTextures.size(); i++)
				if (Rdata->ReservedTextures[i].expired())
				{
					Rdata->ReservedTextures.erase(Rdata->ReservedTextures.begin() + i);
					i = 0;
				}
			for (size_t i = 0; i < Rdata->ReservedVertexBuffers.size(); i++)
				if (Rdata->ReservedVertexBuffers[i].expired())
				{
					Rdata->ReservedVertexBuffers.erase(Rdata->ReservedVertexBuffers.begin() + i);
					i = 0;
				}
			for (size_t i = 0; i < Rdata->ReservedIndexBuffers.size(); i++)
				if (Rdata->ReservedIndexBuffers[i].expired())
				{
					Rdata->ReservedIndexBuffers.erase(Rdata->ReservedIndexBuffers.begin() + i);
					i = 0;
				}
		};

		PushCommand(func);
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, Primitive mode, const unsigned int& vertexCount)
	{
		auto func = [&vertexBuffer, &shader, mode, vertexCount]()
		{
			vertexBuffer.Bind();

			Rdata->CurrentActiveAPI->Draw(shader, mode, vertexCount);

			vertexBuffer.Unbind();

			Rdata->CurrentNumberOfDrawCalls++;
		};

		PushCommand(func);
	}

	void Renderer::EndScene()
	{
		auto func = []()
		{
			if (Rdata->QuadBatchVertexBufferDataPtr != Rdata->QuadBatchVertexBufferDataOrigin)
				FlushQuadBatch();

			if (Rdata->CurrentSceneDescription.Blur && Rdata->m_CurrentBlendMode != RenderingBlendMode::Screen)
			{
				Blur(*Rdata->CurrentSceneDescription.SceneDrawTarget, Rdata->CurrentSceneDescription.BlurRadius);
			}

			memset(&Rdata->CurrentSceneDescription, 0, sizeof(SceneDescription));
			Rdata->NumberOfDrawCallsLastScene = Rdata->CurrentNumberOfDrawCalls;
		};

		PushCommand(func);
	}

	void Renderer::WaitUntilRendererIdle()
	{
		using namespace std::chrono;
		std::unique_lock lock(Rdata->WorkDoneMutex);
		while (Rdata->payload == true) Rdata->WorkDoneCV.wait(lock, []() { return Rdata->payload == false; });
	}

	void Ainan::Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, std::shared_ptr<Texture> texture)
	{
		auto func = [texture, position, color, scale]()
		{
			if ((Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
				Rdata->QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
				FlushQuadBatch();

			float textureSlot;
			if (texture == nullptr)
				textureSlot = 0.0f;
			else
			{
				bool foundTexture = false;
				//check if texture is already used
				for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
				{
					if (Rdata->QuadBatchTextures[i]->GetTextureID() == texture->GetTextureID())
					{
						foundTexture = true;
						textureSlot = i;
						break;
					}
				}

				if (!foundTexture)
				{
					Rdata->QuadBatchTextures[Rdata->QuadBatchTextureSlotsUsed] = texture;
					textureSlot = Rdata->QuadBatchTextureSlotsUsed;
					Rdata->QuadBatchTextureSlotsUsed++;
				}
			}

			Rdata->QuadBatchVertexBufferDataPtr->Position = position;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(0.0f, 1.0f) * scale;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(1.0f, 1.0f) * scale;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(1.0f, 0.0f) * scale;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;
		};

		PushCommand(func);
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, std::shared_ptr<Texture> texture)
	{
		auto func = [position, color, scale, rotationInRadians, texture]()
		{
			if ((Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
				Rdata->QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
				FlushQuadBatch();

			float textureSlot;
			if (texture == nullptr)
				textureSlot = 0.0f;
			else
			{
				bool foundTexture = false;
				//check if texture is already used
				for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
				{
					if (Rdata->QuadBatchTextures[i]->GetTextureID() == texture->GetTextureID())
					{
						foundTexture = true;
						textureSlot = i;
						break;
					}
				}

				if (!foundTexture)
				{
					Rdata->QuadBatchTextures[Rdata->QuadBatchTextureSlotsUsed] = texture;
					textureSlot = Rdata->QuadBatchTextureSlotsUsed;
					Rdata->QuadBatchTextureSlotsUsed++;
				}
			}

			float distance = 0.5f * scale;
			float sine = std::sin(rotationInRadians);
			float cosine = std::cos(rotationInRadians);

			glm::vec2 relPosV0 = glm::vec2((-distance) * cosine - (-distance) * sine, (-distance) * sine + (-distance) * cosine);
			glm::vec2 relPosV1 = glm::vec2((-distance) * cosine - (+distance) * sine, (-distance) * sine + (+distance) * cosine);
			glm::vec2 relPosV2 = glm::vec2((+distance) * cosine - (+distance) * sine, (+distance) * sine + (+distance) * cosine);
			glm::vec2 relPosV3 = glm::vec2((+distance) * cosine - (-distance) * sine, (+distance) * sine + (-distance) * cosine);

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + relPosV0;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + relPosV1;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + relPosV2;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;

			Rdata->QuadBatchVertexBufferDataPtr->Position = position + relPosV3;
			Rdata->QuadBatchVertexBufferDataPtr->Color = color;
			Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
			Rdata->QuadBatchVertexBufferDataPtr++;
		};

		PushCommand(func);
	}

	void Renderer::DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, std::shared_ptr<Texture> texture)
	{
		auto func = [position, color, scale, count, texture]()
		{
			assert(count < c_MaxQuadsPerBatch);

			if ((Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > count * 4 ||
				Rdata->QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
				FlushQuadBatch();

			float textureSlot;
			if (texture == nullptr)
				textureSlot = 0.0f;
			else
			{
				bool foundTexture = false;
				//check if texture is already used
				for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
				{
					if (Rdata->QuadBatchTextures[i]->GetTextureID() == texture->GetTextureID())
					{
						foundTexture = true;
						textureSlot = i;
						break;
					}
				}

				if (!foundTexture)
				{
					Rdata->QuadBatchTextures[Rdata->QuadBatchTextureSlotsUsed] = texture;
					textureSlot = Rdata->QuadBatchTextureSlotsUsed;
					Rdata->QuadBatchTextureSlotsUsed++;
				}
			}

			for (size_t i = 0; i < count; i++)
			{
				Rdata->QuadBatchVertexBufferDataPtr->Position = position[i];
				Rdata->QuadBatchVertexBufferDataPtr->Color = color[i];
				Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
				Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
				Rdata->QuadBatchVertexBufferDataPtr++;

				Rdata->QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(0.0f, 1.0f) * scale[i];
				Rdata->QuadBatchVertexBufferDataPtr->Color = color[i];
				Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
				Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
				Rdata->QuadBatchVertexBufferDataPtr++;

				Rdata->QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(1.0f, 1.0f) * scale[i];
				Rdata->QuadBatchVertexBufferDataPtr->Color = color[i];
				Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
				Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
				Rdata->QuadBatchVertexBufferDataPtr++;

				Rdata->QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(1.0f, 0.0f) * scale[i];
				Rdata->QuadBatchVertexBufferDataPtr->Color = color[i];
				Rdata->QuadBatchVertexBufferDataPtr->Texture = textureSlot;
				Rdata->QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
				Rdata->QuadBatchVertexBufferDataPtr++;
			}
		};

		PushCommand(func);
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, Primitive primitive, const IndexBuffer& indexBuffer)
	{
		auto func = [&vertexBuffer, &indexBuffer, &shader, primitive]()
		{
			vertexBuffer.Bind();
			indexBuffer.Bind();

			Rdata->CurrentActiveAPI->Draw(shader, primitive, indexBuffer);

			vertexBuffer.Unbind();
			indexBuffer.Unbind();

			Rdata->CurrentNumberOfDrawCalls++;
		};

		PushCommand(func);
		WaitUntilRendererIdle();
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, Primitive primitive, const IndexBuffer& indexBuffer, int vertexCount)
	{
		auto func = [&vertexBuffer, &shader, &indexBuffer, primitive, vertexCount]()
		{
			vertexBuffer.Bind();
			indexBuffer.Bind();

			Rdata->CurrentActiveAPI->Draw(shader, primitive, indexBuffer, vertexCount);

			vertexBuffer.Unbind();
			indexBuffer.Unbind();

			Rdata->CurrentNumberOfDrawCalls++;
		};

		PushCommand(func);
	}

	void Renderer::ImGuiNewFrame()
	{
		Rdata->CurrentActiveAPI->ImGuiNewFrame();
	}

	void Renderer::ImGuiEndFrame()
	{
		Rdata->CurrentActiveAPI->ImGuiEndFrame();
	}

	void Renderer::DrawImGui(ImDrawData* drawData)
	{
		auto func = [drawData]()
		{
			Rdata->CurrentActiveAPI->DrawImGui(drawData);
		};
		PushCommand(func);
		WaitUntilRendererIdle();
	}

	void Renderer::ClearScreen()
	{
		auto func = []()
		{
			Rdata->CurrentActiveAPI->ClearScreen();
		};
		PushCommand(func);
	}

	void Renderer::ClearScreenUnsafe()
	{
		Rdata->CurrentActiveAPI->ClearScreen();
	}

	void Renderer::Present()
	{
		auto func = []()
		{
			Rdata->CurrentActiveAPI->Present();
		};
		PushCommand(func);
	}

	void Renderer::RecreateSwapchain(const glm::vec2& newSwapchainSize)
	{
		auto func = [newSwapchainSize]()
		{
			Rdata->CurrentActiveAPI->RecreateSwapchain(newSwapchainSize);
		};
		PushCommand(func);
	}

	//Only called internally by the Renderer, and used only by the Renderer thread
	void Renderer::Blur(std::shared_ptr<FrameBuffer>& target, float radius)
	{
		Rectangle lastViewport = Renderer::GetCurrentViewport();
		RenderingBlendMode lastBlendMode = Rdata->m_CurrentBlendMode;
		Rdata->CurrentActiveAPI->SetBlendMode(RenderingBlendMode::Screen);

		Rectangle viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (int)target->GetSize().x;
		viewport.Height = (int)target->GetSize().y;

		Rdata->CurrentActiveAPI->SetViewport(viewport);
		auto& shader = Rdata->ShaderLibrary["BlurShader"];
		shader->BindUniformBufferUnsafe(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);

		auto resolution = target->GetSize();
		//make a buffer for all the uniform data
		uint8_t bufferData[20];
		glm::vec2 horizonatlDirection = glm::vec2(1.0f, 0.0f);
		glm::vec2 verticalDirection = glm::vec2(0.0f, 1.0f);
		memset(bufferData, 0, 20);

		memcpy(bufferData, &resolution, sizeof(glm::vec2));
		memcpy(bufferData + 8, &horizonatlDirection, sizeof(glm::vec2));
		memcpy(bufferData + 16, &radius, sizeof(float));
		Rdata->BlurUniformBuffer->UpdateDataUnsafe(bufferData);

		//Horizontal blur
		Rdata->BlurFrameBuffer->ResizeUnsafe(target->GetSize());

		Rdata->BlurFrameBuffer->BindUnsafe();
		Rdata->CurrentActiveAPI->ClearScreen();

		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		shader->BindTexture(target, 0, RenderingStage::FragmentShader);
		
		{
			Rdata->BlurVertexBuffer->Bind();
			Rdata->CurrentActiveAPI->Draw(*shader, Primitive::Triangles, 6);
		}

		//this specifies that we are doing vertical blur
		memcpy(bufferData + 8, &verticalDirection, sizeof(glm::vec2));
		Rdata->BlurUniformBuffer->UpdateDataUnsafe(bufferData);

		//clear the buffer we recieved
		target->BindUnsafe();
		Rdata->CurrentActiveAPI->ClearScreen();

		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		shader->BindTexture(Rdata->BlurFrameBuffer, 0, RenderingStage::FragmentShader);
		{
			Rdata->BlurVertexBuffer->Bind();
			Rdata->CurrentActiveAPI->Draw(*shader, Primitive::Triangles, 6);
		}

		Rdata->CurrentActiveAPI->SetViewport(lastViewport);
		Rdata->CurrentActiveAPI->SetBlendMode(lastBlendMode);

		std::lock_guard lock(Rdata->DataMutex);
		Rdata->CurrentNumberOfDrawCalls += 2;
	}

	void Renderer::SetBlendMode(RenderingBlendMode blendMode)
	{
		auto func = [blendMode]()
		{
			Rdata->CurrentActiveAPI->SetBlendMode(blendMode);
		};
		PushCommand(func);

		std::lock_guard lock(Rdata->DataMutex);
		Rdata->m_CurrentBlendMode = blendMode;
	}

	void Renderer::SetViewport(const Rectangle& viewport)
	{
		auto func = [viewport]()
		{
			Rdata->CurrentActiveAPI->SetViewport(viewport);
		};
		PushCommand(func);
		std::lock_guard lock(Rdata->DataMutex);
		Rdata->CurrentViewport = viewport;
	}

	Rectangle Renderer::GetCurrentViewport()
	{
		return Rdata->CurrentViewport;
	}

	void Renderer::SetRenderTargetApplicationWindow()
	{
		auto func = []()
		{
			Rdata->CurrentActiveAPI->SetRenderTargetApplicationWindow();
		};
		PushCommand(func);
	}

	std::shared_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, uint32_t size,
		const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
		bool dynamic)
	{
		std::shared_ptr<VertexBuffer> buffer;
		auto func = [&buffer, data, size, layout, shaderProgram, dynamic]()
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				buffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(data, size, layout, dynamic);
				break;

#ifdef PLATFORM_WINDOWS
			case RendererType::D3D11:
				buffer = std::make_shared<D3D11::D3D11VertexBuffer>(data, size, layout, shaderProgram, dynamic, Rdata->CurrentActiveAPI->GetContext());
				break;
#endif // PLATFORM_WINDOWS

			default:
				assert(false);
			}
			Rdata->ReservedVertexBuffers.push_back(buffer);
		};

		PushCommand(func);

		WaitUntilRendererIdle();
		return buffer;
	}

	std::shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(uint32_t* data, uint32_t count)
	{
		std::shared_ptr<IndexBuffer> buffer;

		auto func = [&buffer, data, count]()
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				buffer = std::make_shared<OpenGL::OpenGLIndexBuffer>(data, count);
				break;

			case RendererType::D3D11:
				buffer = std::make_shared<D3D11::D3D11IndexBuffer>(data, count, Rdata->CurrentActiveAPI->GetContext());
				break;

			default:
				assert(false);
			}
			Rdata->ReservedIndexBuffers.push_back(buffer);
		};

		PushCommand(func);

		WaitUntilRendererIdle();
		return buffer;
	}

	std::shared_ptr<ShaderProgram> Renderer::CreateShaderProgram(const std::string& vertPath, const std::string& fragPath)
	{
		switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLShaderProgram>(vertPath, fragPath);

		case RendererType::D3D11:
			return std::make_shared<D3D11::D3D11ShaderProgram>(vertPath, fragPath, Rdata->CurrentActiveAPI->GetContext());

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<ShaderProgram> Renderer::CreateShaderProgramRaw(const std::string& vertSrc, const std::string& fragSrc)
	{
		std::shared_ptr<ShaderProgram> shader;

		auto func = [&shader, vertSrc, fragSrc]()
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				shader = OpenGL::OpenGLShaderProgram::CreateRaw(vertSrc, fragSrc);

			default:
				assert(false);
				shader = nullptr;
			}
		};

		PushCommand(func);
		WaitUntilRendererIdle();
		return shader;
	}

	std::shared_ptr<FrameBuffer> Renderer::CreateFrameBuffer(const glm::vec2& size)
	{
		std::shared_ptr<FrameBuffer> buffer;

		auto func = [&buffer, size]()
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				buffer = std::make_shared<OpenGL::OpenGLFrameBuffer>(size);
				break;
#ifdef PLATFORM_WINDOWS

			case RendererType::D3D11:
				buffer = std::make_shared<D3D11::D3D11FrameBuffer>(size, Rdata->CurrentActiveAPI->GetContext());
				break;
#endif

			default:
				assert(false);
				buffer = nullptr;
			}
		};

		PushCommand(func);
		WaitUntilRendererIdle();
		return buffer;
	}

	std::shared_ptr<Texture> Renderer::CreateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data)
	{
		std::shared_ptr<Texture> texture;

		auto func = [&texture, size, format, data]
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				texture = std::make_shared<OpenGL::OpenGLTexture>(size, format, data);
				break;

#ifdef PLATFORM_WINDOWS
			case RendererType::D3D11:
				texture = std::make_shared<D3D11::D3D11Texture>(size, format, data, Rdata->CurrentActiveAPI->GetContext());
				break;
#endif // PLATFORM_WINDOWS

			default:
				assert(false);
			}

			Rdata->ReservedTextures.push_back(texture);
		};

		PushCommand(func);
		WaitUntilRendererIdle();
		return texture;
	}

	std::shared_ptr<Texture> Renderer::CreateTexture(Image& img)
	{
		std::shared_ptr<Texture> texture;
		auto func = [&texture, img]()
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				texture = std::make_shared<OpenGL::OpenGLTexture>(glm::vec2(img.m_Width, img.m_Height), img.Format, img.m_Data);
				break;

#ifdef PLATFORM_WINDOWS
			case RendererType::D3D11:
				texture = std::make_shared<D3D11::D3D11Texture>(glm::vec2(img.m_Width, img.m_Height), img.Format, img.m_Data, Rdata->CurrentActiveAPI->GetContext());
				break;
#endif // PLATFORM_WINDOWS

			default:
				assert(false);
			}
			Rdata->ReservedTextures.push_back(texture);
		};

		PushCommand(func);
		WaitUntilRendererIdle();
		return texture;
	}

	std::shared_ptr<UniformBuffer> Renderer::CreateUniformBuffer(const std::string& name, uint32_t reg, const VertexLayout& layout, void* data)
	{
		std::shared_ptr<UniformBuffer> buffer;

		auto func = [&buffer, name, reg, layout, data]()
		{
			switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
			{
			case RendererType::OpenGL:
				buffer = std::make_shared<OpenGL::OpenGLUniformBuffer>(name, layout, data);
				break;

#ifdef PLATFORM_WINDOWS
			case RendererType::D3D11:
				buffer = std::make_shared<D3D11::D3D11UniformBuffer>(name, reg, layout, data, Rdata->CurrentActiveAPI->GetContext());
				break;
#endif // PLATFORM_WINDOWS

			default:
				assert(false);
			}
		};

		PushCommand(func);
		WaitUntilRendererIdle();
		return buffer;
	}

	std::array<glm::vec2, 6> Renderer::GetQuadVertices()
	{
		switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
		{
			//these are in clockwise ordering
			std::array<glm::vec2, 6> openglVertices =
			{
				 glm::vec2(-1.0f, -1.0f), //bottom left
				 glm::vec2(1.0f, -1.0f),  //bottom right
				 glm::vec2(-1.0f, 1.0f),  //top left

				 glm::vec2(1.0f, -1.0f),  //bottom right
				 glm::vec2(1.0f, 1.0f),   //top right
				 glm::vec2(-1.0f, 1.0f)   //top left
			};
			return openglVertices;
		}

#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
		{
			//these are in clockwise ordering
			std::array<glm::vec2, 6> d3d11Vertices =
			{
				 glm::vec2(-1.0f,  1.0f),  //top left
				 glm::vec2(1.0f,   1.0f),  //top right
				 glm::vec2(-1.0f, -1.0f),  //bottom left

				 glm::vec2(1.0f,   1.0f),  //top left
				 glm::vec2(1.0f,  -1.0f),  //bottom right
				 glm::vec2(-1.0f, -1.0f)   //bottom left
			};
			return d3d11Vertices;
		}
#endif //PLATFORM_WINDOWS

		default:
		{
			assert(false);
			std::array<glm::vec2, 6> fallback;
			return fallback;
		}
		}
	}

	void Ainan::Renderer::FlushQuadBatch()
	{
		for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
			Rdata->ShaderLibrary["QuadBatchShader"]->BindTexture(Rdata->QuadBatchTextures[i], i, RenderingStage::FragmentShader);

		int numVertices = (Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin);

		Rdata->QuadBatchVertexBuffer->UpdateDataUnsafe(0,
			numVertices * sizeof(QuadVertex),
			Rdata->QuadBatchVertexBufferDataOrigin);

		Rdata->QuadBatchVertexBuffer->Bind();
		Rdata->QuadBatchIndexBuffer->Bind();

		Rdata->CurrentActiveAPI->Draw(*Rdata->ShaderLibrary["QuadBatchShader"], Primitive::Triangles, *Rdata->QuadBatchIndexBuffer, (numVertices * 3) / 2);

		Rdata->QuadBatchVertexBuffer->Unbind();
		Rdata->QuadBatchIndexBuffer->Unbind();

		Rdata->CurrentNumberOfDrawCalls++;

		//reset data so we can accept the next batch
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;
		Rdata->QuadBatchTextureSlotsUsed = 1;
	}

	std::string RendererTypeStr(RendererType type)
	{
		switch (type)
		{
#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
			return "DirectX 11";
#endif // PLATFORM_WINDOWS

		case RendererType::OpenGL:
		default:
			return "OpenGL";
		}
	}

	RendererType RendererTypeVal(const std::string& name)
	{
#ifdef PLATFORM_WINDOWS
		if (name == "DirectX 11")
			return RendererType::D3D11;
#endif // PLATFORM_WINDOWS

		if (name == "OpenGL")
			return RendererType::OpenGL;

		assert(false);
		return RendererType::OpenGL;
	}
}