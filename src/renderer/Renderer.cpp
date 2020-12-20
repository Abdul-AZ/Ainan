#include "Renderer.h"

#include "opengl/OpenGLRendererAPI.h"
#include "opengl/OpenGLShaderProgram.h"
#include "opengl/OpenGLVertexBuffer.h"
#include "opengl/OpenGLIndexBuffer.h"
#include "opengl/OpenGLTexture.h"
#include "opengl/OpenGLFrameBuffer.h"
#include "opengl/OpenGLUniformBuffer.h"

#include <GLFW/glfw3.h>

#ifdef PLATFORM_WINDOWS

#include "d3d11/D3D11RendererAPI.h"
#include "d3d11/D3D11ShaderProgram.h"
#include "d3d11/D3D11VertexBuffer.h"
#include "d3d11/D3D11IndexBuffer.h"
#include "d3d11/D3D11UniformBuffer.h"
#include "d3d11/D3D11Texture.h"
#include "d3d11/D3D11FrameBuffer.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#undef max

#endif // PLATFORM_WINDOWS

bool WantUpdateMonitors = true;

namespace Ainan {

	double LastFrameFinishTime = 0.0;
	double LastFrameDeltaTime = 0.0;

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
		{ "CircleOutlineShader" , "shaders/CircleOutline" , "shaders/CircleOutline"  },
		{ "LineShader"          , "shaders/FlatColor"     , "shaders/FlatColor"      },
		{ "BlurShader"          , "shaders/Image"         , "shaders/Blur"           },
		{ "GizmoShader"         , "shaders/Gizmo"         , "shaders/Gizmo"          },
		{ "GridShader"          , "shaders/Grid"          , "shaders/Grid"           },
		{ "ImageShader"         , "shaders/Image"         , "shaders/Image"          },
		{ "QuadBatchShader"     , "shaders/QuadBatch"     , "shaders/QuadBatch"      },
		{ "LitSpriteShader"     , "shaders/LitSprite"     , "shaders/LitSprite"      }
	};

	void Renderer::Init(RendererType api)
	{
		//allocate renderer memory
		Rdata = new RendererData();
		Rdata->API = api;

		auto initFunc = [api]()
		{
			Renderer::RendererThreadLoop(api);
			Renderer::InternalTerminate();
		};

		Rdata->Thread = std::thread(initFunc);

		//wait until the renderer thread finished initializing
		Renderer::InternalInit(api);
		WaitUntilRendererIdle();
	}

	void Renderer::Terminate()
	{
		//signal and wait for the renderer thread to stop
		Rdata->DestroyThread = true;
		Rdata->cv.notify_all();
		Rdata->Thread.join();

		//free renderer memory
		delete Rdata;
	}

	void Renderer::InternalInit(RendererType api)
	{
		//load shaders
		for (auto& shaderInfo : CompileOnInit)
		{
			Rdata->ShaderLibraryNew[shaderInfo.Name] = CreateShaderProgramNew(shaderInfo.VertexCodePath, shaderInfo.FragmentCodePath);
		}

		//setup batch renderer
		{
			VertexLayout layout(4);
			layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
			layout[1] = VertexLayoutElement("NORMAL", 0, ShaderVariableType::Vec4);
			layout[2] = VertexLayoutElement("TEXCOORD", 0, ShaderVariableType::Float);
			layout[3] = VertexLayoutElement("TEXCOORD", 1, ShaderVariableType::Vec2);

			Rdata->QuadBatchVertexBuffer = CreateVertexBufferNew(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex), layout, Rdata->ShaderLibraryNew["QuadBatchShader"], true);
		}

		const int32_t indexCount = c_MaxQuadsPerBatch * 6;
		uint32_t* indicies = new uint32_t[indexCount];
		size_t u = 0;
		for (size_t i = 0; i < indexCount; i += 6)
		{
			indicies[i + 0] = 0 + u;
			indicies[i + 1] = 1 + u;
			indicies[i + 2] = 2 + u;

			indicies[i + 3] = 0 + u;
			indicies[i + 4] = 2 + u;
			indicies[i + 5] = 3 + u;
			u += 4;
		}

		Rdata->QuadBatchIndexBuffer = CreateIndexBufferNew(indicies, indexCount);
		delete[] indicies;

		Rdata->QuadBatchVertexBufferDataOrigin = new QuadVertex[c_MaxQuadVerticesPerBatch];
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;

		Rdata->QuadBatchTextures[0] = CreateTextureNew(glm::vec2(1, 1), TextureFormat::RGBA, nullptr);

		auto img = std::make_shared<Image>();
		img->m_Width = 1;
		img->m_Height = 1;
		img->Format = TextureFormat::RGBA;
		img->m_Data = new uint8_t[4];
		memset(img->m_Data, (uint8_t)255, 4);
		Rdata->QuadBatchTextures[0].SetImage(img);

		//setup postprocessing
		Rdata->BlurFrameBuffer = CreateFrameBufferNew(Window::FramebufferSize);

		{
			std::array<std::pair<glm::vec2, glm::vec2>, 6> quadVertices;

			switch (api)
			{
			case RendererType::OpenGL:
			{
				quadVertices =
				{
					std::pair(glm::vec2(-1.0f, -1.0f),  glm::vec2(0.0f, 0.0f)),
					std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 1.0f)),
					std::pair(glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 0.0f)),

					std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 1.0f)),
					std::pair(glm::vec2(1.0f,  1.0f),  glm::vec2(1.0f, 1.0f)),
					std::pair(glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 0.0f))
				};
				break;
			}

			case RendererType::D3D11:
			{
				quadVertices =
				{
					std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 0.0f)),
					std::pair(glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 1.0f)),
					std::pair(glm::vec2(-1.0f, -1.0f),  glm::vec2(0.0f, 1.0f)),

					std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 0.0f)),
					std::pair(glm::vec2(1.0f,  1.0f),  glm::vec2(1.0f, 0.0f)),
					std::pair(glm::vec2(1.0f, -1.0f),  glm::vec2(1.0f, 1.0f))
				};
				break;
			}
			}
			VertexLayout layout(2);
			layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
			layout[1] = VertexLayoutElement("NORMAL", 0, ShaderVariableType::Vec2);
			Rdata->BlurVertexBuffer = CreateVertexBufferNew(quadVertices.data(), sizeof(quadVertices), layout, Rdata->ShaderLibraryNew["BlurShader"]);
		}

		{
			VertexLayout layout =
			{
				VertexLayoutElement("u_Resolution",0, ShaderVariableType::Vec2),
				VertexLayoutElement("u_BlurDirection",0, ShaderVariableType::Vec2),
				VertexLayoutElement("u_Radius",0, ShaderVariableType::Float)
			};
			Rdata->BlurUniformBuffer = CreateUniformBufferNew("BlurData", 1, layout);
			Rdata->ShaderLibraryNew["BlurShader"].BindUniformBuffer(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);
		}

		{
			VertexLayout layout =
			{
				VertexLayoutElement("u_ViewProjection",    0, ShaderVariableType::Mat4),

				VertexLayoutElement("RadialLightPosition", 0, ShaderVariableType::Vec2Array,  c_MaxRadialLightCount),
				VertexLayoutElement("RadialLightColor",    0, ShaderVariableType::Vec4Array,  c_MaxRadialLightCount),
				VertexLayoutElement("RadialLightIntensity",0, ShaderVariableType::FloatArray, c_MaxRadialLightCount),

				VertexLayoutElement("SpotLightPosition",   0, ShaderVariableType::Vec2Array,  c_MaxSpotLightCount),
				VertexLayoutElement("SpotLightColor",      0, ShaderVariableType::Vec4Array,  c_MaxSpotLightCount),
				VertexLayoutElement("SpotLightAngle",      0, ShaderVariableType::FloatArray, c_MaxSpotLightCount),
				VertexLayoutElement("SpotLightInnerCutoff",0, ShaderVariableType::FloatArray, c_MaxSpotLightCount),
				VertexLayoutElement("SpotLightOuterCutoff",0, ShaderVariableType::FloatArray, c_MaxSpotLightCount),
				VertexLayoutElement("SpotLightIntensity",  0, ShaderVariableType::FloatArray, c_MaxSpotLightCount)
			};

			Rdata->SceneUniformbufferNew = CreateUniformBufferNew("FrameData", 0, layout);

			for (auto& shaderTuple : Rdata->ShaderLibraryNew)
			{
				shaderTuple.second.BindUniformBuffer(Rdata->SceneUniformbufferNew, 0, RenderingStage::VertexShader);
				shaderTuple.second.BindUniformBuffer(Rdata->SceneUniformbufferNew, 0, RenderingStage::FragmentShader);
			}
		}

		auto func = []()
		{
			std::lock_guard lock(Rdata->DataMutex);

			Rdata->CurrentActiveAPI->SetBlendMode(Rdata->m_CurrentBlendMode);

			InitImGuiRendering();
		};

		Renderer::PushCommand(func);
	}

	void Renderer::RendererThreadLoop(RendererType api)
	{
		//initilize the renderer api
		switch (api)
		{
#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
			Rdata->CurrentActiveAPI = new D3D11::D3D11RendererAPI();
			break;
#endif

		case RendererType::OpenGL:
			Rdata->CurrentActiveAPI = new OpenGL::OpenGLRendererAPI();
			break;
		}

		AINAN_LOG_INFO("Renderer Initilized\nBackend: " + RendererTypeStr(Rdata->CurrentActiveAPI->GetContext()->GetType()) +
			"             Version: " + Rdata->CurrentActiveAPI->GetContext()->GetVersionString() +
			"             Physical Device: " + Rdata->CurrentActiveAPI->GetContext()->GetPhysicalDeviceName());

		while (true)
		{
			if (Rdata->DestroyThread)
				break;
			{
				{
					std::unique_lock<std::mutex> lock(Rdata->QueueMutex);
					Rdata->cv.wait(lock, []() { return Rdata->payload == true || Rdata->DestroyThread; });
				}
				RenderCommand cmd;
				while (Rdata->CommandBuffer.size() > 0)
				{
					{
						std::unique_lock lock(Rdata->QueueMutex);
						cmd = Rdata->CommandBuffer.front();
						Rdata->CommandBuffer.pop();
					}

					if (cmd.Type == RenderCommandType::CustomCommand)
						cmd.CustomCommand();
					else
						Rdata->CurrentActiveAPI->ExecuteCommand(cmd);
				}
				Rdata->payload = false;
				Rdata->WorkDoneCV.notify_all();
			}
		}
	}

	void Renderer::InternalTerminate()
	{
		delete Rdata->CurrentActiveAPI;

		//batch renderer data
		//Rdata->QuadBatchVertexBuffer.reset();
		//Rdata->QuadBatchIndexBuffer.reset();
		delete[] Rdata->QuadBatchVertexBufferDataOrigin;
		//Rdata->QuadBatchTextures[0].reset();
	}

	void Renderer::PushCommand(RenderCommand cmd)
	{
		if (Window::Minimized)
			return;

		std::unique_lock lock(Rdata->QueueMutex);
		Rdata->CommandBuffer.push(cmd);
		Rdata->payload = true;
		Rdata->cv.notify_one();
	}

	void Renderer::BeginScene(const SceneDescription& desc)
	{
		Rdata->CurrentSceneDescription = desc;
		Rdata->SceneBuffer.CurrentViewProjection = desc.SceneCamera.ProjectionMatrix * desc.SceneCamera.ViewMatrix;
		Rdata->CurrentNumberOfDrawCalls = 0;
		Rdata->RadialLightSubmissionCount = 0;
		Rdata->SpotLightSubmissionCount = 0;
		Rdata->SceneUniformbufferNew.UpdateData(&Rdata->SceneBuffer, sizeof(RendererData::SceneUniformBuffer));

		Rdata->CurrentSceneDescription.SceneDrawTarget.Bind();
		auto func = [desc]()
		{
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
			for (size_t i = 0; i < Rdata->ReservedUniformBuffers.size(); i++)
				if (Rdata->ReservedUniformBuffers[i].expired())
				{
					Rdata->ReservedUniformBuffers.erase(Rdata->ReservedUniformBuffers.begin() + i);
					i = 0;
				}
		};

		PushCommand(func);
	}

	void Renderer::AddRadialLight(const glm::vec2& pos, const glm::vec4& color, float intensity)
	{
		auto& i = Rdata->RadialLightSubmissionCount;
		auto& buffer = Rdata->SceneBuffer;

		buffer.RadialLightPositions[i] = c_GlobalScaleFactor * pos;
		buffer.RadialLightColors[i] = color;
		buffer.RadialLightIntensities[i] = intensity;
		i++;
	}

	void Renderer::AddSpotLight(const glm::vec2& pos, const glm::vec4 color, float angle, float innerCutoff, float outerCutoff, float intensity)
	{
		auto& i = Rdata->SpotLightSubmissionCount;
		auto& buffer = Rdata->SceneBuffer;

		buffer.SpotLightPositions[i] = c_GlobalScaleFactor * pos;
		buffer.SpotLightColors[i] = color;
		buffer.SpotLightIntensities[i] = intensity;
		buffer.SpotLightAngles[i] =	      glm::radians(angle);
		buffer.SpotLightInnerCutoffs[i] = glm::radians(innerCutoff);
		buffer.SpotLightOuterCutoffs[i] = glm::radians(outerCutoff);
		i++;
	}

	void Renderer::Draw(VertexBufferNew vertexBuffer, ShaderProgramNew shader, Primitive primitive, int32_t vertexCount)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::Draw_NewShader;
		cmd.NewVBuffer = Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.ExtraData = &Rdata->ShaderPrograms[shader.Identifier];
		cmd.Misc1 = (uint32_t)primitive;
		cmd.Misc2 = vertexCount;

		PushCommand(cmd);
	}

	void Renderer::EndScene()
	{
		if (Rdata->QuadBatchVertexBufferDataPtr != Rdata->QuadBatchVertexBufferDataOrigin)
			FlushQuadBatch();
		
		auto func = []()
		{
			if (Rdata->CurrentSceneDescription.Blur && Rdata->m_CurrentBlendMode != RenderingBlendMode::Screen)
			{
				Blur(Rdata->CurrentSceneDescription.SceneDrawTarget, Rdata->CurrentSceneDescription.BlurRadius);
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
		while (Rdata->payload == true) Rdata->WorkDoneCV.wait_for(lock, 1ms, []() { return Rdata->payload == false; });
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, TextureNew texture)
	{
		if ((Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
			Rdata->QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();
		
		float textureSlot;
		if (texture.IsValid() == false)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
			{
				if (Rdata->QuadBatchTextures[i].GetTextureID() == texture.GetTextureID())
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
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, TextureNew texture)
	{
		if ((Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
			Rdata->QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture.Identifier == std::numeric_limits<uint32_t>::max())
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
			{
				if (Rdata->QuadBatchTextures[i].GetTextureID() == texture.GetTextureID())
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
	}

	void Renderer::DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, TextureNew texture)
	{
		if ((Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > count * 4 ||
			Rdata->QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();
		
		assert(count < c_MaxQuadsPerBatch);

		float textureSlot;
		if (texture.IsValid() == false)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
			{
				if (Rdata->QuadBatchTextures[i].GetTextureID() == texture.GetTextureID())
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
	}

	void Renderer::ImGuiNewFrame()
	{
		Rdata->CurrentActiveAPI->ImGuiNewFrame();

		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		glfwGetWindowSize(Window::Ptr, &w, &h);
		glfwGetFramebufferSize(Window::Ptr, &display_w, &display_h);
		io.DisplaySize = ImVec2((float)w, (float)h);
		if (w > 0 && h > 0)
			io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
		if (WantUpdateMonitors)
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

		// Setup time step
		double current_time = glfwGetTime();
		io.DeltaTime = Rdata->Time > 0.0 ? (float)(current_time - Rdata->Time) : (float)(1.0f / 60.0f);
		Rdata->Time = current_time;

		ImGui::NewFrame();
	}

	void Renderer::Draw(VertexBufferNew vertexBuffer, ShaderProgramNew shader, Primitive primitive, IndexBufferNew indexBuffer)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DrawIndexedNew;

		cmd.NewVBuffer = Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.NewIBuffer = Rdata->IndexBuffers[indexBuffer.Identifier];
		cmd.NewShader = Rdata->ShaderPrograms[shader.Identifier];
		cmd.DrawingPrimitive = primitive;

		Renderer::PushCommand(cmd);
	}


	void Renderer::ImGuiEndFrame()
	{
		Rdata->CurrentActiveAPI->ImGuiEndFrame();
	}

	uint32_t Renderer::GetUsedGPUMemory()
	{
		uint32_t memory = 0;

		memory += std::accumulate(Rdata->ReservedVertexBuffers.begin(), Rdata->ReservedVertexBuffers.end(), 0,
			[](uint32_t a, const std::weak_ptr<VertexBuffer>& b) -> uint32_t
			{
				return a + b.lock()->GetUsedMemory();
			});

		memory += std::accumulate(Rdata->ReservedIndexBuffers.begin(), Rdata->ReservedIndexBuffers.end(), 0,
			[](uint32_t a, const std::weak_ptr<IndexBuffer>& b) -> uint32_t
			{
				return a + b.lock()->GetUsedMemory();
			});

		memory += std::accumulate(Rdata->ReservedUniformBuffers.begin(), Rdata->ReservedUniformBuffers.end(), 0,
			[](uint32_t a, const std::weak_ptr<UniformBuffer>& b) -> uint32_t
			{
				return a + b.lock()->GetAlignedSize();
			});

		memory += std::accumulate(Rdata->ReservedTextures.begin(), Rdata->ReservedTextures.end(), 0,
			[](uint32_t a, const std::weak_ptr<Texture>& b) -> uint32_t
			{
				return a + b.lock()->GetMemorySize();
			});
		 
		return memory;
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
		RenderCommand cmd;
		cmd.Type = RenderCommandType::ClearScreen;
		PushCommand(cmd);
	}

	void Renderer::ClearScreenUnsafe()
	{
		Rdata->CurrentActiveAPI->ClearScreen();
	}

	void Renderer::Present()
	{
		std::chrono::high_resolution_clock::now();
		 
		RenderCommand cmd;
		cmd.Type = RenderCommandType::Present;
		PushCommand(cmd);
		WaitUntilRendererIdle();

		bool running = true;

		while (running) 
		{
			double time = glfwGetTime();
			LastFrameDeltaTime = time - LastFrameFinishTime;

			if (LastFrameDeltaTime >= c_ApplicationMaxFramePeriod)
			{
				LastFrameFinishTime = time;
				break;
			}
		}
	}

	void Renderer::RecreateSwapchain(const glm::vec2& newSwapchainSize)
	{
		auto func = [newSwapchainSize]()
		{
			Rdata->CurrentActiveAPI->RecreateSwapchain(newSwapchainSize);
		};
		PushCommand(func);
	}

	void Renderer::Draw(ShaderProgramNew shader, VertexBufferNew vertexBuffer, Primitive primitive, uint32_t vertexCount)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DrawNew;
		cmd.NewShader = Rdata->ShaderPrograms[shader.Identifier];
		cmd.NewVBuffer = Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.DrawingPrimitive = primitive;
		cmd.Misc1 = vertexCount;

		Renderer::PushCommand(cmd);
	} 

	//Only called internally by the Renderer, and used only by the Renderer thread
	void Renderer::Blur(FrameBufferNew target, float radius)
	{
		Rectangle lastViewport = Renderer::GetCurrentViewport();
		RenderingBlendMode lastBlendMode = Rdata->m_CurrentBlendMode;
		auto func = []()
		{
			Rdata->CurrentActiveAPI->SetBlendMode(RenderingBlendMode::Screen);
		};
		PushCommand(func);
		
		Rectangle viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (int)Renderer::Rdata->FrameBuffers[target.Identifier].Size.x;
		viewport.Height = (int)Renderer::Rdata->FrameBuffers[target.Identifier].Size.y;
		
		auto func2 = [viewport]() 
		{
			Rdata->CurrentActiveAPI->SetViewport(viewport);
		};
		PushCommand(func2);
		auto& shader = Rdata->ShaderLibraryNew["BlurShader"];
		shader.BindUniformBuffer(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);
		
		auto resolution = Renderer::Rdata->FrameBuffers[target.Identifier].Size;
		//make a buffer for all the uniform data
		uint8_t bufferData[20];
		glm::vec2 horizonatlDirection = glm::vec2(1.0f, 0.0f);
		glm::vec2 verticalDirection = glm::vec2(0.0f, 1.0f);
		memset(bufferData, 0, 20);
		
		memcpy(bufferData, &resolution, sizeof(glm::vec2));
		memcpy(bufferData + 8, &horizonatlDirection, sizeof(glm::vec2));
		memcpy(bufferData + 16, &radius, sizeof(float));
		Rdata->BlurUniformBuffer.UpdateData(bufferData, sizeof(bufferData));
		
		//Horizontal blur
		Rdata->BlurFrameBuffer.Resize(Renderer::Rdata->FrameBuffers[target.Identifier].Size);
		
		Rdata->BlurFrameBuffer.Bind();
		ClearScreen();
		
		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		shader.BindTexture(target, 0, RenderingStage::FragmentShader);
		Draw(shader, Rdata->BlurVertexBuffer, Primitive::Triangles, 6);
		
		//this specifies that we are doing vertical blur
		memcpy(bufferData + 8, &verticalDirection, sizeof(glm::vec2));
		Rdata->BlurUniformBuffer.UpdateData(bufferData, sizeof(bufferData));
		
		//clear the buffer we recieved
		target.Bind();
		ClearScreen();
		
		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		shader.BindTexture(Rdata->BlurFrameBuffer, 0, RenderingStage::FragmentShader);
		Draw(shader, Rdata->BlurVertexBuffer, Primitive::Triangles, 6);
		
		auto func3 = [lastViewport, lastBlendMode]()
		{
			Rdata->CurrentActiveAPI->SetViewport(lastViewport);
			Rdata->CurrentActiveAPI->SetBlendMode(lastBlendMode);
		};
		PushCommand(func3);
		
		std::lock_guard lock(Rdata->DataMutex);
		Rdata->CurrentNumberOfDrawCalls += 2;
	}

	void Renderer::InitImGuiRendering()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable viewports

		// Setup back-end capabilities flags
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
		io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;    // We can create multi-viewports on the Platform side (optional)
		io.BackendPlatformName = "glfw backend";

		// Our mouse update function expect PlatformHandle to be filled for the main viewport
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		main_viewport->PlatformHandle = (void*)Window::Ptr;

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
			GLFWwindow* share_window = nullptr;
			if (Rdata->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
			{
				share_window = Window::Ptr;
			}
			else
			{
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			}
			data->Window = glfwCreateWindow((int)viewport->Size.x, (int)viewport->Size.y, "No Title Yet", NULL, share_window);
			data->WindowOwned = true;
			viewport->PlatformHandle = (void*)data->Window;
			viewport->PlatformHandleRaw = glfwGetWin32Window(data->Window);
			glfwSetWindowPos(data->Window, (int)viewport->Pos.x, (int)viewport->Pos.y);

			// Install callbacks
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
			if (Rdata->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
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
			if (Rdata->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
				glfwMakeContextCurrent(window);
		};
		platform_io.Platform_SwapBuffers = [](ImGuiViewport* viewport, void*)
		{
			GET_WINDOW(window);
			if (Rdata->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
			{
				glfwMakeContextCurrent(window);
				glfwSwapBuffers(window);
			}
		};
#undef GET_WINDOW(x)

		// Note: monitor callback are broken GLFW 3.2 and earlier (see github.com/glfw/glfw/issues/784)
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
		glfwSetMonitorCallback([](GLFWmonitor*, int)
			{
				WantUpdateMonitors = true;
			});

		// Register main window handle (which is owned by the main application, not by us)
		ImGuiViewportDataGlfw* data = IM_NEW(ImGuiViewportDataGlfw)();
		data->Window = Window::Ptr;
		data->WindowOwned = false;
		main_viewport->PlatformUserData = data;
		main_viewport->PlatformHandle = (void*)Window::Ptr;

		io.Fonts->AddFontFromFileTTF("res/Roboto-Medium.ttf", 15);
		Rdata->CurrentActiveAPI->InitImGui();
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

	IndexBufferNew Renderer::CreateIndexBufferNew(uint32_t* data, uint32_t count)
	{
		static uint32_t s_IdentifierCounter = 1;
		IndexBufferNew bufferHandle;
		bufferHandle.Identifier = s_IdentifierCounter;
		IndexBufferDataView view;
		view.Count = count;
		view.Size = count * sizeof(uint32_t);
		Rdata->IndexBuffers[s_IdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateIndexBuffer;
		IndexBufferCreationInfo* info = new IndexBufferCreationInfo;
		info->InitialData = new uint8_t[view.Size];
		memcpy(info->InitialData, data, view.Size);
		info->Count = count;
		cmd.ExtraData = info;
		cmd.Output = &Rdata->IndexBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	VertexBufferNew Renderer::CreateVertexBufferNew(void* data, uint32_t size, const VertexLayout& layout, ShaderProgramNew shaderProgram, bool dynamic)
	{
		static uint32_t s_IdentifierCounter = 1;
		VertexBufferNew bufferHandle;
		bufferHandle.Identifier = s_IdentifierCounter;
		VertexBufferDataView view;
		view.Size = size;
		Rdata->VertexBuffers[s_IdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateVertexBuffer;
		VertexBufferCreationInfo* info = new VertexBufferCreationInfo;
		if (data != nullptr)
		{
			info->InitialData = new uint8_t[size];
			memcpy(info->InitialData, data, size);
		}
		else
			info->InitialData = nullptr;
		info->Shader = Rdata->ShaderPrograms[shaderProgram.Identifier].Identifier;
		info->Layout = layout;
		info->Size = size;
		info->Dynamic = dynamic;
		cmd.ExtraData = info;
		cmd.Output = &Rdata->VertexBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	UniformBufferNew Renderer::CreateUniformBufferNew(const std::string& name, uint32_t reg,
		const VertexLayout& layout)
	{
		static uint32_t s_IdentifierCounter = 1;
		UniformBufferNew bufferHandle;
		bufferHandle.Identifier = s_IdentifierCounter;
		UniformBufferDataView view;
		view.Name = name;
		Rdata->UniformBuffers[s_IdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateUniformBuffer;
		UniformBufferCreationInfo* info = new UniformBufferCreationInfo;
		info->Name = name;
		info->reg = reg;
		info->layout = layout;
		cmd.ExtraData = info;
		cmd.Output = &Rdata->UniformBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	FrameBufferNew Renderer::CreateFrameBufferNew(const glm::vec2& size)
	{
		static uint32_t s_IdentifierCounter = 1;
		FrameBufferNew bufferHandle;
		bufferHandle.Identifier = s_IdentifierCounter;
		FrameBufferDataView view;
		view.Size = size;
		Rdata->FrameBuffers[s_IdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateFrameBuffer;
		FrameBufferCreationInfo* info = new FrameBufferCreationInfo;
		info->Size = size;
		cmd.ExtraData = info;
		cmd.Output = &Rdata->FrameBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	TextureNew Renderer::CreateTextureNew(const glm::vec2& size, TextureFormat format, uint8_t* data)
	{
		Image img;
		img.m_Width = size.x;
		img.m_Height = size.y;
		img.Format = format;
		img.m_Data = data;
		return CreateTextureNew(img);
	}

	TextureNew Renderer::CreateTextureNew(Image& img)
	{
		static uint32_t s_IdentifierCounter = 1;
		TextureNew textureHandle;
		textureHandle.Identifier = s_IdentifierCounter;
		TextureDataView view;
		Rdata->Textures[s_IdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateTexture;
		TextureCreationInfo* info = new TextureCreationInfo;
		info->Size = glm::vec2(img.m_Width, img.m_Height);
		info->Format = img.Format;
		int32_t comp = 0;
		switch (info->Format)
		{
		case TextureFormat::RGBA:
			comp = 4;
			break;

		case TextureFormat::RGB:
			comp = 3;
			break;

		case TextureFormat::RG:
			comp = 2;
			break;

		case TextureFormat::R:
			comp = 1;
			break;
		default:
			break;
		}
		if (img.m_Data)
		{
			info->InitialData = new uint8_t[img.m_Width * img.m_Height * comp];
			memcpy(info->InitialData, img.m_Data, sizeof(uint8_t) * img.m_Width * img.m_Height * comp);
		}
		else
			info->InitialData = nullptr;
		cmd.ExtraData = info;
		cmd.Output = &Rdata->Textures[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return textureHandle;
	}

	ShaderProgramNew Renderer::CreateShaderProgramNew(const std::string& vertPath, const std::string& fragPath)
	{
		static uint32_t s_IdentifierCounter = 1;
		ShaderProgramNew programHandle;
		programHandle.Identifier = s_IdentifierCounter;
		ShaderProgramDataView view;
		Rdata->ShaderPrograms[s_IdentifierCounter] = view;
		
		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateShaderProgram;
		ShaderProgramCreationInfo* info = new ShaderProgramCreationInfo;
		info->vertPath = vertPath;
		info->fragPath = fragPath;
		cmd.ExtraData = info;
		cmd.Output = &Rdata->ShaderPrograms[s_IdentifierCounter];
		
		PushCommand(cmd);
		s_IdentifierCounter++;
		return programHandle;
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

	std::array<std::pair<glm::vec2, glm::vec2>, 6> Renderer::GetTexturedQuadVertices()
	{
		std::array<std::pair<glm::vec2, glm::vec2>, 6> quadVertices;

		switch (Rdata->API)
		{
		case RendererType::OpenGL:
		{
			quadVertices =
			{
				std::pair(glm::vec2(-1.0f, -1.0f),  glm::vec2(0.0f, 0.0f)),
				std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 1.0f)),
				std::pair(glm::vec2( 1.0f, -1.0f),  glm::vec2(1.0f, 0.0f)),

				std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 1.0f)),
				std::pair(glm::vec2( 1.0f,  1.0f),  glm::vec2(1.0f, 1.0f)),
				std::pair(glm::vec2( 1.0f, -1.0f),  glm::vec2(1.0f, 0.0f))
			};
			break;
		}

		case RendererType::D3D11:
		{
			quadVertices =
			{
				std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 0.0f)),
				std::pair(glm::vec2( 1.0f, -1.0f),  glm::vec2(1.0f, 1.0f)),
				std::pair(glm::vec2(-1.0f, -1.0f),  glm::vec2(0.0f, 1.0f)),

				std::pair(glm::vec2(-1.0f,  1.0f),  glm::vec2(0.0f, 0.0f)),
				std::pair(glm::vec2( 1.0f,  1.0f),  glm::vec2(1.0f, 0.0f)),
				std::pair(glm::vec2( 1.0f, -1.0f),  glm::vec2(1.0f, 1.0f))
			};
			break;
		}
		}

		return quadVertices;
	}

	void Renderer::Draw(VertexBufferNew vertexBuffer, ShaderProgramNew shader, Primitive primitive, IndexBufferNew indexBuffer, uint32_t indexCount)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DrawIndexedNewWithCustomNumberOfVertices;
		cmd.NewVBuffer = Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.NewIBuffer = Rdata->IndexBuffers[indexBuffer.Identifier];
		cmd.NewShader = Rdata->ShaderPrograms[shader.Identifier];
		cmd.DrawingPrimitive = primitive;
		cmd.Misc1 = indexCount;

		Renderer::PushCommand(cmd);
	}

	void Renderer::FlushQuadBatch()
	{
		for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
			Rdata->ShaderLibraryNew["QuadBatchShader"].BindTexture(Rdata->QuadBatchTextures[i], i, RenderingStage::FragmentShader);

		int32_t numVertices = (Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin);

		Rdata->QuadBatchVertexBuffer.UpdateData(0,
			numVertices * sizeof(QuadVertex),
			Rdata->QuadBatchVertexBufferDataOrigin);

		Draw(Rdata->QuadBatchVertexBuffer, Rdata->ShaderLibraryNew["QuadBatchShader"], Primitive::Triangles, Rdata->QuadBatchIndexBuffer, (numVertices * 3) / 2);

		Rdata->CurrentNumberOfDrawCalls++;

		//reset data so we can accept the next batch
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;
		Rdata->QuadBatchTextureSlotsUsed = 1;

		//TODO remove this safely
		WaitUntilRendererIdle();
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