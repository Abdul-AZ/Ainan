#include "Renderer.h"

#include "opengl/OpenGLRendererAPI.h"
#include "ImGuizmo.h"
#include <GLFW/glfw3.h>

#ifdef PLATFORM_WINDOWS

#include "d3d11/D3D11RendererAPI.h"

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
		{ "LitSpriteShader"     , "shaders/LitSprite"     , "shaders/LitSprite"      },
		{ "3DAmbientShader"     , "shaders/3DAmbient"     , "shaders/3DAmbient"      },
		{ "SkyboxShader"        , "shaders/Skybox"        , "shaders/Skybox"         }
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
		Rdata->CurrentActiveAPI->TerminateImGui();
		DestroyVertexBuffer(Rdata->QuadBatchVertexBuffer);
		DestroyVertexBuffer(Rdata->BlurVertexBuffer);
		DestroyIndexBuffer(Rdata->QuadBatchIndexBuffer);
		DestroyUniformBuffer(Rdata->SceneUniformBuffer);
		DestroyUniformBuffer(Rdata->BlurUniformBuffer);
		DestroyTexture(Rdata->WhiteTexture);
		DestroyFramebuffer(Rdata->BlurFramebuffer);

		//free the shader library
		for (auto shader : Rdata->ShaderLibrary)
		{
			RenderCommand cmd;
			cmd.Type = RenderCommandType::DestroyShaderProgram;
			cmd.DestroyShaderProgramCmdDesc.Program = &Rdata->ShaderPrograms[shader.second.Identifier];

			Renderer::PushCommand(cmd);
		}

		//signal and wait for the renderer thread to stop
		Rdata->DestroyThread = true;
		Rdata->Thread.join();

		CleanupDeletedObjects();

		//check for memory leaks
		if (Rdata->ShaderPrograms.size() > 0)
			AINAN_LOG_FATAL("Memory Leak: " + std::to_string(Rdata->ShaderPrograms.size()) + " shaders were not freed");
		if (Rdata->VertexBuffers.size() > 0)
			AINAN_LOG_FATAL("Memory Leak: " + std::to_string(Rdata->VertexBuffers.size()) + " vertex buffers were not freed");
		if (Rdata->IndexBuffers.size() > 0)
			AINAN_LOG_FATAL("Memory Leak: " + std::to_string(Rdata->IndexBuffers.size()) + " index buffers were not freed");
		if (Rdata->UniformBuffers.size() > 0)
			AINAN_LOG_FATAL("Memory Leak: " + std::to_string(Rdata->UniformBuffers.size()) + " uniform buffers were not freed");
		if (Rdata->Framebuffers.size() > 0)
			AINAN_LOG_FATAL("Memory Leak: " + std::to_string(Rdata->Framebuffers.size()) + " frame buffers were not freed");
		if (Rdata->Textures.size() > 0)
			AINAN_LOG_FATAL("Memory Leak: " + std::to_string(Rdata->Textures.size()) + " textures were not freed");

		//free renderer memory
		delete Rdata;
	}

	void Renderer::CleanupDeletedObjects()
	{
		{
			auto& shaders = Rdata->ShaderPrograms;
			auto it = shaders.begin();
			while (it != shaders.end())
			{
				if (it->second.Deleted)
					it = shaders.erase(it);
				else
					it++;
			}
		}

		{
			auto& vbuffers = Rdata->VertexBuffers;
			auto it = vbuffers.begin();
			while (it != vbuffers.end())
			{
				if (it->second.Deleted)
					it = vbuffers.erase(it);
				else
					it++;
			}
		}

		{
			auto& ibuffers = Rdata->IndexBuffers;
			auto it = ibuffers.begin();
			while (it != ibuffers.end())
			{
				if (it->second.Deleted)
					it = ibuffers.erase(it);
				else
					it++;
			}
		}

		{
			auto& ubuffers = Rdata->UniformBuffers;
			auto it = ubuffers.begin();
			while (it != ubuffers.end())
			{
				if (it->second.Deleted)
					it = ubuffers.erase(it);
				else
					it++;
			}
		}

		{
			auto& fbuffers = Rdata->Framebuffers;
			auto it = fbuffers.begin();
			while (it != fbuffers.end())
			{
				if (it->second.Deleted)
					it = fbuffers.erase(it);
				else
					it++;
			}
		}

		{
			auto& utextures = Rdata->Textures;
			auto it = utextures.begin();
			while (it != utextures.end())
			{
				if (it->second.Deleted)
					it = utextures.erase(it);
				else
					it++;
			}
		}
	}

	void Renderer::InternalInit(RendererType api)
	{
		//load shaders
		for (auto& shaderInfo : CompileOnInit)
		{
			Rdata->ShaderLibrary[shaderInfo.Name] = CreateShaderProgram(shaderInfo.VertexCodePath, shaderInfo.FragmentCodePath);
		}

		//setup batch renderer
		{
			VertexLayout layout(4);
			layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
			layout[1] = VertexLayoutElement("NORMAL", 0, ShaderVariableType::Vec4);
			layout[2] = VertexLayoutElement("TEXCOORD", 0, ShaderVariableType::Float);
			layout[3] = VertexLayoutElement("TEXCOORD", 1, ShaderVariableType::Vec2);

			Rdata->QuadBatchVertexBuffer = CreateVertexBuffer(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex), layout, Rdata->ShaderLibrary["QuadBatchShader"], true);
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

		Rdata->QuadBatchIndexBuffer = CreateIndexBuffer(indicies, indexCount);
		delete[] indicies;

		Rdata->QuadBatchVertexBufferDataOrigin = new QuadVertex[c_MaxQuadVerticesPerBatch];
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;

		Rdata->WhiteTexture = CreateTexture(glm::vec2(1, 1), TextureFormat::RGBA, TextureType::Texture2D, nullptr);
		Rdata->QuadBatchTextures[0] = Rdata->WhiteTexture;

		auto img = std::make_shared<Image>();
		img->m_Width = 1;
		img->m_Height = 1;
		img->Format = TextureFormat::RGBA;
		img->m_Data = new uint8_t[4];
		memset(img->m_Data, (uint8_t)255, 4);
		Rdata->QuadBatchTextures[0].UpdateData(img);
		Rdata->QuadBatchTextureSlotsUsed = 1;

		for (size_t i = 0; i < c_MaxQuadTexturesPerBatch; i++)
			Rdata->ShaderLibrary["QuadBatchShader"].BindTexture(Rdata->WhiteTexture, i, RenderingStage::FragmentShader);

		//setup postprocessing
		Rdata->BlurFramebuffer = CreateFramebuffer(Window::FramebufferSize);

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
			Rdata->BlurVertexBuffer = CreateVertexBuffer(quadVertices.data(), sizeof(quadVertices), layout, Rdata->ShaderLibrary["BlurShader"]);
		}

		{
			VertexLayout layout =
			{
				VertexLayoutElement("u_Resolution",0, ShaderVariableType::Vec2),
				VertexLayoutElement("u_BlurDirection",0, ShaderVariableType::Vec2),
				VertexLayoutElement("u_Radius",0, ShaderVariableType::Float)
			};
			Rdata->BlurUniformBuffer = CreateUniformBuffer("BlurData", 1, layout);
			Rdata->ShaderLibrary["BlurShader"].BindUniformBuffer(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);
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

			Rdata->SceneUniformBuffer = CreateUniformBuffer("FrameData", 0, layout);

			for (auto& shaderTuple : Rdata->ShaderLibrary)
			{
				shaderTuple.second.BindUniformBuffer(Rdata->SceneUniformBuffer, 0, RenderingStage::VertexShader);
				shaderTuple.second.BindUniformBuffer(Rdata->SceneUniformBuffer, 0, RenderingStage::FragmentShader);
			}
		}

		SetBlendMode(Rdata->m_CurrentBlendMode);
		InitImGuiRendering();
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

		auto execCmd = [](const RenderCommand& cmd)
		{
			if (cmd.Type == RenderCommandType::CustomCommand)
				cmd.CustomCommand();
			else
				Rdata->CurrentActiveAPI->ExecuteCommand(cmd);
		};
		while (true)
		{
			if (Rdata->DestroyThread)
				break;
			{
				
				Rdata->CommandQueue.WaitPopAndExecuteAll(execCmd);
			}
		}
	}

	void Renderer::InternalTerminate()
	{
		delete[] Rdata->QuadBatchVertexBufferDataOrigin;
		delete Rdata->CurrentActiveAPI;
	}

	void Renderer::PushCommand(RenderCommand cmd)
	{
		Rdata->CommandQueue.Push(cmd);
	}

	void Renderer::BeginScene(const SceneDescription& desc)
	{
		//clear previous frame's lights
		Rdata->CurrentSceneDescription = desc;
		Rdata->SceneBufferData.CurrentViewProjection = desc.SceneCamera.GetViewProjectionMatrix();
		Rdata->CurrentNumberOfDrawCalls = 0;
		Rdata->RadialLightSubmissionCount = 0;
		Rdata->SpotLightSubmissionCount = 0;
		Rdata->SceneUniformBuffer.UpdateData(&Rdata->SceneBufferData, sizeof(RendererData::SceneUniformBufferData));

		Rdata->CurrentSceneDescription.SceneDrawTarget.Bind();
	}

	void Renderer::AddRadialLight(const glm::vec2& pos, const glm::vec4& color, float intensity)
	{
		auto& i = Rdata->RadialLightSubmissionCount;
		auto& buffer = Rdata->SceneBufferData;

		buffer.RadialLightPositions[i] = pos;
		buffer.RadialLightColors[i] = color;
		buffer.RadialLightIntensities[i] = intensity;
		i++;
	}

	void Renderer::AddSpotLight(const glm::vec2& pos, const glm::vec4 color, float angle, float innerCutoff, float outerCutoff, float intensity)
	{
		auto& i = Rdata->SpotLightSubmissionCount;
		auto& buffer = Rdata->SceneBufferData;

		buffer.SpotLightPositions[i] = pos;
		buffer.SpotLightColors[i] = color;
		buffer.SpotLightIntensities[i] = intensity;
		buffer.SpotLightAngles[i] =	angle;
		buffer.SpotLightInnerCutoffs[i] = glm::radians(innerCutoff);
		buffer.SpotLightOuterCutoffs[i] = glm::radians(outerCutoff);
		i++;
	}

	void Renderer::Draw(VertexBuffer vertexBuffer, ShaderProgram shader, Primitive primitive, int32_t vertexCount)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DrawNew;
		cmd.DrawNewCmdDesc.VertexBuffer = &Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.DrawNewCmdDesc.Shader = &Rdata->ShaderPrograms[shader.Identifier];
		cmd.DrawNewCmdDesc.DrawingPrimitive = primitive;
		cmd.DrawNewCmdDesc.VertexCount = vertexCount;

		PushCommand(cmd);
	}

	void Renderer::EndScene()
	{
		if (Rdata->QuadBatchVertexBufferDataPtr != Rdata->QuadBatchVertexBufferDataOrigin)
			FlushQuadBatch();
		
		if (Rdata->CurrentSceneDescription.Blur && Rdata->m_CurrentBlendMode != RenderingBlendMode::Screen)
		{
			Blur(Rdata->CurrentSceneDescription.SceneDrawTarget, Rdata->CurrentSceneDescription.BlurRadius);
		}

		memset(&Rdata->CurrentSceneDescription, 0, sizeof(SceneDescription));
		memset(&Rdata->SceneBufferData, 0, sizeof(Renderer::RendererData::SceneUniformBufferData));
		Rdata->NumberOfDrawCallsLastScene = Rdata->CurrentNumberOfDrawCalls;
	}

	void Renderer::WaitUntilRendererIdle()
	{
		Rdata->CommandQueue.WaitUntilIdle();
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, Texture texture)
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
			for (size_t i = 1; i < Rdata->QuadBatchTextureSlotsUsed; i++)
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

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, Texture texture)
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
			for (size_t i = 1; i < Rdata->QuadBatchTextureSlotsUsed; i++)
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

	void Renderer::DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, Texture texture)
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
			for (size_t i = 1; i < Rdata->QuadBatchTextureSlotsUsed; i++)
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
		ImGuizmo::BeginFrame();
		Rdata->WindowsAboveViewport.clear();
	}

	void Renderer::RegisterWindowThatCanCoverViewport()
	{
		if (ImGui::GetCurrentWindow()->Viewport == ImGui::GetMainViewport() && ImGui::IsWindowDocked() == false)
		{
			Rdata->WindowsAboveViewport.push_back(ImGui::GetWindowDrawList());
		}
	}

	void Renderer::Draw(VertexBuffer vertexBuffer, ShaderProgram shader, Primitive primitive, IndexBuffer indexBuffer)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DrawIndexedNew;

		cmd.DrawIndexedCmdDesc.VertexBuffer = &Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.DrawIndexedCmdDesc.IndexBuffer = &Rdata->IndexBuffers[indexBuffer.Identifier];
		cmd.DrawIndexedCmdDesc.Shader = &Rdata->ShaderPrograms[shader.Identifier];
		cmd.DrawIndexedCmdDesc.DrawingPrimitive = primitive;

		Renderer::PushCommand(cmd);
	}

	void Renderer::ImGuiEndFrame(bool redraw)
	{
		ImGui::Render();
		Rdata->CurrentActiveAPI->ImGuiEndFrame(redraw);
	}

	uint32_t Renderer::GetUsedGPUMemory()
	{
		uint32_t memory = 0;

		memory += std::accumulate(Rdata->VertexBuffers.begin(), Rdata->VertexBuffers.end(), 0,
			[](uint32_t a, const std::pair<uint32_t, VertexBufferDataView>& buffer) -> uint32_t
			{
				return a + buffer.second.Size;
			});
		
		memory += std::accumulate(Rdata->IndexBuffers.begin(), Rdata->IndexBuffers.end(), 0,
			[](uint32_t a, const std::pair<uint32_t, IndexBufferDataView>& buffer) -> uint32_t
			{
				return a + buffer.second.Size;
			});
		
		memory += std::accumulate(Rdata->UniformBuffers.begin(), Rdata->UniformBuffers.end(), 0,
			[](uint32_t a, const std::pair<uint32_t, UniformBufferDataView>& buffer) -> uint32_t
			{
				return a + buffer.second.AlignedSize;
			});
		
		memory += std::accumulate(Rdata->Textures.begin(), Rdata->Textures.end(), 0,
			[](uint32_t a, const std::pair<uint32_t, TextureDataView>& tex) -> uint32_t
			{
				return a + tex.second.Size.x * tex.second.Size.y * GetBytesPerPixel(tex.second.Format);
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
	}

	void Renderer::ClearScreen()
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::Clear;
		PushCommand(cmd);
	}

	void Renderer::Present()
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::Present;
		PushCommand(cmd);
		CleanupDeletedObjects();

		LastFrameDeltaTime = glfwGetTime() - LastFrameFinishTime;
		LastFrameFinishTime = glfwGetTime();
	}

	void Renderer::SleepExtraFrametime()
	{
		LastFrameDeltaTime = glfwGetTime() - LastFrameFinishTime;
		std::this_thread::sleep_for(std::chrono::duration<double>((1 / 60.0) - LastFrameDeltaTime));
		LastFrameFinishTime = glfwGetTime();
	}

	void Renderer::RecreateSwapchain(const glm::vec2& newSwapchainSize)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::RecreateSweapchain;
		cmd.RecreateSweapchainCmdDesc.Width = newSwapchainSize.x;
		cmd.RecreateSweapchainCmdDesc.Height = newSwapchainSize.y;
		PushCommand(cmd);
	}

	void Renderer::Blur(Framebuffer target, float radius)
	{
		Rectangle lastViewport = Rdata->CurrentViewport;
		RenderingBlendMode lastBlendMode = Rdata->m_CurrentBlendMode;

		SetBlendMode(RenderingBlendMode::Screen);
		
		Rectangle viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (int)Renderer::Rdata->Framebuffers[target.Identifier].Size.x;
		viewport.Height = (int)Renderer::Rdata->Framebuffers[target.Identifier].Size.y;
		
		SetViewport(viewport);
		auto& shader = Rdata->ShaderLibrary["BlurShader"];
		shader.BindUniformBuffer(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);
		
		auto resolution = Renderer::Rdata->Framebuffers[target.Identifier].Size;
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
		Rdata->BlurFramebuffer.Resize(Renderer::Rdata->Framebuffers[target.Identifier].Size);
		
		Rdata->BlurFramebuffer.Bind();
		ClearScreen();
		
		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		shader.BindTexture(target, 0, RenderingStage::FragmentShader);
		Draw(Rdata->BlurVertexBuffer, shader, Primitive::Triangles, 6);
		
		//this specifies that we are doing vertical blur
		memcpy(bufferData + 8, &verticalDirection, sizeof(glm::vec2));
		Rdata->BlurUniformBuffer.UpdateData(bufferData, sizeof(bufferData));
		
		//clear the buffer we recieved
		target.Bind();
		ClearScreen();
		
		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		shader.BindTexture(Rdata->BlurFramebuffer, 0, RenderingStage::FragmentShader);
		Draw(Rdata->BlurVertexBuffer, shader, Primitive::Triangles, 6);
		
		SetViewport(lastViewport);
		SetBlendMode(lastBlendMode);

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

			///////////////// Install callbacks

			//install scroll callback
			glfwSetScrollCallback(data->Window, [](GLFWwindow* window, double x, double y)
				{
					ImGuiIO& io = ImGui::GetIO();
					io.MouseWheelH += (float)x;
					io.MouseWheel += (float)y;
				});

			//install keycallback
			auto imguiKeyCallback = [](GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
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
			glfwSetKeyCallback(data->Window, imguiKeyCallback);

			//install char callback
			auto imguiCharCallback = [](GLFWwindow* window, uint32_t c)
			{
				ImGuiIO& io = ImGui::GetIO();
				if (c > 0 && c < 0x10000)
					io.AddInputCharacter((uint16_t)c);
			};
			glfwSetCharCallback(data->Window, imguiCharCallback);

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
		PushCommand([]() 
			{
			Rdata->CurrentActiveAPI->InitImGui();
			});
	}

	void Renderer::SetBlendMode(RenderingBlendMode blendMode)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::SetBlendMode;
		cmd.SetBlendModeCmdDesc.Mode = blendMode;
		PushCommand(cmd);
		Rdata->m_CurrentBlendMode = blendMode;
	}

	//TODO handle depth values
	void Renderer::SetViewport(const Rectangle& viewport)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::SetViewport;
		cmd.SetViewportCmdDesc.X = viewport.X;
		cmd.SetViewportCmdDesc.Y = viewport.Y;
		cmd.SetViewportCmdDesc.Width = viewport.Width;
		cmd.SetViewportCmdDesc.Height = viewport.Height;
		cmd.SetViewportCmdDesc.MinDepth = 0;
		cmd.SetViewportCmdDesc.MaxDepth = 0;
		PushCommand(cmd);
		Rdata->CurrentViewport = viewport;
	}

	void Renderer::SetRenderTargetApplicationWindow()
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::BindBackBufferAsRenderTarget;
		PushCommand(cmd);
	}

	void Renderer::DestroyVertexBuffer(VertexBuffer vb)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DestroyVertexBuffer;
		cmd.DestroyVertexBufferCmdDesc.Buffer = &Rdata->VertexBuffers[vb.Identifier];
		Renderer::PushCommand(cmd);
	}

	IndexBuffer Renderer::CreateIndexBuffer(uint32_t* data, uint32_t count)
	{
		static uint32_t s_IdentifierCounter = 1;
		IndexBuffer bufferHandle;
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
		cmd.CreateIndexBufferCmdDesc.Info = info;
		cmd.CreateIndexBufferCmdDesc.Output = &Rdata->IndexBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	void Renderer::DestroyIndexBuffer(IndexBuffer ib)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DestroyIndexBuffer;
		cmd.DestroyIndexBufferCmdDesc.Buffer = &Rdata->IndexBuffers[ib.Identifier];
		Renderer::PushCommand(cmd);
	}

	VertexBuffer Renderer::CreateVertexBuffer(void* data, uint32_t size, const VertexLayout& layout, ShaderProgram shaderProgram, bool dynamic)
	{
		static uint32_t s_IdentifierCounter = 1;
		VertexBuffer bufferHandle;
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
		info->Shader = &Rdata->ShaderPrograms[shaderProgram.Identifier];
		info->Layout = layout;
		info->Size = size;
		info->Dynamic = dynamic;
		cmd.CreateVertexBufferCmdDesc.Info = info;
		cmd.CreateVertexBufferCmdDesc.Output = &Rdata->VertexBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	UniformBuffer Renderer::CreateUniformBuffer(const std::string& name, uint32_t reg,
		const VertexLayout& layout)
	{
		static uint32_t s_IdentifierCounter = 1;
		UniformBuffer bufferHandle;
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
		cmd.CreateUniformBufferCmdDesc.Info = info;
		cmd.CreateUniformBufferCmdDesc.Output = &Rdata->UniformBuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	void Renderer::DestroyUniformBuffer(UniformBuffer ub)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DestroyUniformBuffer;
		cmd.DestroyUniformBufferCmdDesc.Buffer = &Rdata->UniformBuffers[ub.Identifier];
		Renderer::PushCommand(cmd);
	}

	Framebuffer Renderer::CreateFramebuffer(const glm::vec2& size)
	{
		static uint32_t s_IdentifierCounter = 1;
		Framebuffer bufferHandle;
		bufferHandle.Identifier = s_IdentifierCounter;
		FramebufferDataView view;
		view.Size = size;
		Rdata->Framebuffers[s_IdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateFramebuffer;
		FramebufferCreationInfo* info = new FramebufferCreationInfo;
		info->Size = size;
		cmd.CreateFramebufferCmdDesc.Info = info;
		cmd.CreateFramebufferCmdDesc.Output = &Rdata->Framebuffers[s_IdentifierCounter];

		PushCommand(cmd);
		s_IdentifierCounter++;
		return bufferHandle;
	}

	void Renderer::DestroyFramebuffer(Framebuffer fb)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DestroyFramebuffer;
		cmd.DestroyFramebufferCmdDesc.Buffer = &Rdata->Framebuffers[fb.Identifier];
		PushCommand(cmd);
	}

	static uint32_t s_TextureIdentifierCounter = 1;
	Texture Renderer::CreateTexture(const glm::vec2& size, TextureFormat format, TextureType type, uint8_t* data)
	{
		Texture textureHandle;
		textureHandle.Identifier = s_TextureIdentifierCounter;
		TextureDataView view;
		view.Format = format;
		view.Size = size;
		view.Type = type;
		Rdata->Textures[s_TextureIdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateTexture;
		TextureCreationInfo* info = new TextureCreationInfo;
		info->Size = size;
		info->Format = format;
		info->Type = type;
		int32_t comp = GetBytesPerPixel(info->Format);

		if (data)
		{
			info->InitialData = new uint8_t[size.x * size.y * comp];
			memcpy(info->InitialData, data, sizeof(uint8_t) * size.x * size.y * comp);
		}
		else
			info->InitialData = nullptr;
		cmd.CreateTextureProgramCmdDesc.Info = info;
		cmd.CreateTextureProgramCmdDesc.Output = &Rdata->Textures[s_TextureIdentifierCounter];

		PushCommand(cmd);
		s_TextureIdentifierCounter++;
		return textureHandle;
	}

	Texture Renderer::CreateTexture(Image& img)
	{
		return CreateTexture({ img.m_Width, img.m_Height }, img.Format, TextureType::Texture2D, img.m_Data);
	}

	Texture Renderer::CreateCubemapTexture(std::array<Image, 6>& faces)
	{
		int32_t comp = GetBytesPerPixel(faces[0].Format);
		size_t faceDataSize = faces[0].m_Width * faces[0].m_Height * comp;
		uint8_t* data = new uint8_t[faceDataSize * faces.size()];
		for (size_t i = 0; i < faces.size(); i++)
		{
			if (!faces[i].m_Data)
				AINAN_LOG_FATAL("Missing data when creating cubemap");
			memcpy(data + faceDataSize * i, faces[i].m_Data, faceDataSize);
		}

		Texture textureHandle;
		textureHandle.Identifier = s_TextureIdentifierCounter;
		TextureDataView view;
		view.Format = faces[0].Format;
		view.Size = glm::vec2{ faces[0].m_Width, faces[0].m_Height };
		view.Type = TextureType::Cubemap;
		Rdata->Textures[s_TextureIdentifierCounter] = view;

		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateTexture;
		TextureCreationInfo* info = new TextureCreationInfo;
		info->Size = view.Size;
		info->Format = view.Format;
		info->Type = view.Type;
		info->InitialData = data;

		cmd.CreateTextureProgramCmdDesc.Info = info;
		cmd.CreateTextureProgramCmdDesc.Output = &Rdata->Textures[s_TextureIdentifierCounter];

		PushCommand(cmd);
		s_TextureIdentifierCounter++;
		return textureHandle;
	}

	void Renderer::DestroyTexture(Texture tex)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DestroyTexture;
		cmd.DestroyTextureCmdDesc.Texture = &Rdata->Textures[tex.Identifier];
		PushCommand(cmd);
	}

	ShaderProgram Renderer::CreateShaderProgram(const std::string& vertPath, const std::string& fragPath)
	{
		static uint32_t s_IdentifierCounter = 1;
		ShaderProgram programHandle;
		programHandle.Identifier = s_IdentifierCounter;
		ShaderProgramDataView view;
		Rdata->ShaderPrograms[s_IdentifierCounter] = view;
		
		RenderCommand cmd;
		cmd.Type = RenderCommandType::CreateShaderProgram;
		ShaderProgramCreationInfo* info = new ShaderProgramCreationInfo;
		info->vertPath = vertPath;
		info->fragPath = fragPath;
		cmd.CreateShaderProgramCmdDesc.Info = info;
		cmd.CreateShaderProgramCmdDesc.Output = &Rdata->ShaderPrograms[s_IdentifierCounter];
		
		PushCommand(cmd);
		s_IdentifierCounter++;
		return programHandle;
	}

	void Renderer::Draw(VertexBuffer vertexBuffer, ShaderProgram shader, Primitive primitive, IndexBuffer indexBuffer, uint32_t indexCount)
	{
		RenderCommand cmd;
		cmd.Type = RenderCommandType::DrawIndexedNewWithCustomNumberOfVertices;
		cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.VertexBuffer = &Rdata->VertexBuffers[vertexBuffer.Identifier];
		cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.IndexBuffer = &Rdata->IndexBuffers[indexBuffer.Identifier];
		cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.Shader = &Rdata->ShaderPrograms[shader.Identifier];
		cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.DrawingPrimitive = primitive;
		cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.IndexCount = indexCount;

		Renderer::PushCommand(cmd);
	}

	void Renderer::FlushQuadBatch()
	{
		for (size_t i = 0; i < Rdata->QuadBatchTextureSlotsUsed; i++)
			Rdata->ShaderLibrary["QuadBatchShader"].BindTexture(Rdata->QuadBatchTextures[i], i, RenderingStage::FragmentShader);

		int32_t numVertices = (Rdata->QuadBatchVertexBufferDataPtr - Rdata->QuadBatchVertexBufferDataOrigin);

		Rdata->QuadBatchVertexBuffer.UpdateData(0,
			numVertices * sizeof(QuadVertex),
			Rdata->QuadBatchVertexBufferDataOrigin);

		Draw(Rdata->QuadBatchVertexBuffer, Rdata->ShaderLibrary["QuadBatchShader"], Primitive::Triangles, Rdata->QuadBatchIndexBuffer, (numVertices * 3) / 2);

		Rdata->CurrentNumberOfDrawCalls++;

		//reset data so we can accept the next batch
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;
		Rdata->QuadBatchTextureSlotsUsed = 1;

		for (size_t i = 1; i < c_MaxQuadTexturesPerBatch; i++)
			Rdata->QuadBatchTextures[i].Identifier = std::numeric_limits<uint32_t>::max();
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