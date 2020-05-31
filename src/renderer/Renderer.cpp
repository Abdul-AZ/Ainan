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
		{ "ImageShader"         , "shaders/Image"         , "shaders/Image"          },
		{ "QuadBatchShader"     , "shaders/QuadBatch"     , "shaders/QuadBatch"      }
	};

	void Renderer::Init(RendererType api)
	{
		Rdata = new RendererData();

		//initilize the renderer api
		switch (api)
		{
		case RendererType::D3D11:
			Rdata->m_CurrentActiveAPI = new D3D11::D3D11RendererAPI();
			break;

		case RendererType::OpenGL:
			Rdata->m_CurrentActiveAPI = new OpenGL::OpenGLRendererAPI();
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

			Rdata->m_QuadBatchVertexBuffer = CreateVertexBuffer(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex),
				layout, Rdata->ShaderLibrary["QuadBatchShader"], true);
		}

		unsigned int* indicies = new unsigned int[c_MaxQuadsPerBatch * 6];
		int u = 0;
		for (size_t i = 0; i < c_MaxQuadsPerBatch * 6; i+=6)
		{
			indicies[i + 0] = 0 + u;
			indicies[i + 1] = 1 + u;
			indicies[i + 2] = 2 + u;

			indicies[i + 3] = 0 + u;
			indicies[i + 4] = 2 + u;
			indicies[i + 5] = 3 + u;
			u += 4;
		}
		Rdata->m_QuadBatchIndexBuffer = CreateIndexBuffer(indicies, c_MaxQuadsPerBatch * 6);
		delete[] indicies;

		Rdata->m_QuadBatchVertexBufferDataOrigin = new QuadVertex[c_MaxQuadVerticesPerBatch];
		Rdata->m_QuadBatchVertexBufferDataPtr = Rdata->m_QuadBatchVertexBufferDataOrigin;

		Rdata->m_QuadBatchTextures[0] = CreateTexture(glm::vec2(1,1), TextureFormat::RGBA);
		Rdata->m_QuadBatchTextures[0]->SetDefaultTextureSettings();
		Image img;
		img.m_Width = 1;
		img.m_Height = 1;
		img.Format = TextureFormat::RGBA;
		img.m_Data = new unsigned char[4];
		memset(img.m_Data, (unsigned char)255, 4);
		Rdata->m_QuadBatchTextures[0]->SetImage(img);

		//setup postprocessing
		Rdata->m_BlurFrameBuffer = CreateFrameBuffer(Window::FramebufferSize);

		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		{
			VertexLayout layout(2);
			layout[0] = { "aPos", ShaderVariableType::Vec2 };
			layout[1] = { "aTexCoords", ShaderVariableType::Vec2 };
			Rdata->m_BlurVertexBuffer = CreateVertexBuffer(quadVertices, sizeof(quadVertices), layout, Rdata->ShaderLibrary["BlurShader"]);
		}
		{
			VertexLayout bufferLayout =
			{
				{ "u_Resolution", ShaderVariableType::Vec2 },
				{ "u_BlurDirection", ShaderVariableType::Vec2 },
				{ "u_Radius", ShaderVariableType::Float }
			};
			Rdata->m_BlurUniformBuffer = CreateUniformBuffer("BlurData", 1, bufferLayout, nullptr);
			Rdata->ShaderLibrary["BlurShader"]->BindUniformBuffer(Rdata->m_BlurUniformBuffer, 1, RenderingStage::FragmentShader);
		}

		SetBlendMode(Rdata->m_CurrentBlendMode);

		{
			VertexLayout bufferLayout =
			{
				{ "u_ViewProjection", ShaderVariableType::Mat4 }
			};
			Rdata->SceneUniformbuffer = CreateUniformBuffer("FrameData", 0, bufferLayout, nullptr);
			for (auto& shaderTuple : Rdata->ShaderLibrary)
			{
				shaderTuple.second->BindUniformBuffer(Rdata->SceneUniformbuffer, 0, RenderingStage::VertexShader);
			}
		}
	}

	void Renderer::Terminate()
	{
		Rdata->ShaderLibrary.erase(Rdata->ShaderLibrary.begin(), Rdata->ShaderLibrary.end());

		auto type = Rdata->m_CurrentActiveAPI->GetContext()->GetType();

		delete Rdata->m_CurrentActiveAPI;

		//batch renderer data
		Rdata->m_QuadBatchVertexBuffer.reset();
		Rdata->m_QuadBatchIndexBuffer.reset();
		delete[] Rdata->m_QuadBatchVertexBufferDataOrigin;
		Rdata->m_QuadBatchTextures[0].reset();

		delete Rdata;
	}

	void Renderer::BeginScene(const SceneDescription& desc)
	{
		assert(desc.SceneDrawTarget);

		Rdata->m_CurrentSceneDescription = desc;
		Rdata->m_CurrentViewProjection = desc.SceneCamera.ProjectionMatrix * desc.SceneCamera.ViewMatrix;
		Rdata->CurrentNumberOfDrawCalls = 0;

		Rdata->m_CurrentSceneDescription.SceneDrawTarget->Bind();
		ClearScreen();

		//update the per-frame uniform buffer
		Rdata->SceneUniformbuffer->UpdateData(&Rdata->m_CurrentViewProjection);

		//update diagnostics stuff
		for (size_t i = 0; i < Rdata->m_ReservedTextures.size(); i++)
			if (Rdata->m_ReservedTextures[i].expired())
			{
				Rdata->m_ReservedTextures.erase(Rdata->m_ReservedTextures.begin() + i);
				i = 0;
			}
		for (size_t i = 0; i < Rdata->m_ReservedVertexBuffers.size(); i++)
			if (Rdata->m_ReservedVertexBuffers[i].expired())
			{
				Rdata->m_ReservedVertexBuffers.erase(Rdata->m_ReservedVertexBuffers.begin() + i);
				i = 0;
			}
		for (size_t i = 0; i < Rdata->m_ReservedIndexBuffers.size(); i++)
			if (Rdata->m_ReservedIndexBuffers[i].expired())
			{
				Rdata->m_ReservedIndexBuffers.erase(Rdata->m_ReservedIndexBuffers.begin() + i);
				i = 0;
			}
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
	{
		vertexBuffer.Bind();

		Rdata->m_CurrentActiveAPI->Draw(shader, mode, vertexCount);

		vertexBuffer.Unbind();

		Rdata->CurrentNumberOfDrawCalls++;
	}

	void Renderer::EndScene()
	{
		if(Rdata->m_QuadBatchVertexBufferDataPtr != Rdata->m_QuadBatchVertexBufferDataOrigin)
			FlushQuadBatch();

		if (Rdata->m_CurrentSceneDescription.Blur && Rdata->m_CurrentBlendMode != RenderingBlendMode::Screen)
		{
			Blur(Rdata->m_CurrentSceneDescription.SceneDrawTarget, Rdata->m_CurrentSceneDescription.BlurRadius);
		}

		SetRenderTargetApplicationWindow();

		memset(&Rdata->m_CurrentSceneDescription, 0, sizeof(SceneDescription));
		Rdata->NumberOfDrawCallsLastScene = Rdata->CurrentNumberOfDrawCalls;
	}

	void Ainan::Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, std::shared_ptr<Texture> texture)
	{
		if ((Rdata->m_QuadBatchVertexBufferDataPtr - Rdata->m_QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
			Rdata->m_QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture == nullptr)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < Rdata->m_QuadBatchTextureSlotsUsed; i++)
			{
				if (Rdata->m_QuadBatchTextures[i]->GetRendererID() == texture->GetRendererID())
				{
					foundTexture = true;
					textureSlot = i;
					break;
				}
			}
			
			if (!foundTexture)
			{
				Rdata->m_QuadBatchTextures[Rdata->m_QuadBatchTextureSlotsUsed] = texture;
				textureSlot = Rdata->m_QuadBatchTextureSlotsUsed;
				Rdata->m_QuadBatchTextureSlotsUsed++;
			}
		}

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(0.0f, 1.0f) * scale;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(1.0f, 1.0f) * scale;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(1.0f, 0.0f) * scale;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, std::shared_ptr<Texture> texture)
	{
		if ((Rdata->m_QuadBatchVertexBufferDataPtr - Rdata->m_QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
			Rdata->m_QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture == nullptr)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < Rdata->m_QuadBatchTextureSlotsUsed; i++)
			{
				if (Rdata->m_QuadBatchTextures[i]->GetRendererID() == texture->GetRendererID())
				{
					foundTexture = true;
					textureSlot = i;
					break;
				}
			}

			if (!foundTexture)
			{
				Rdata->m_QuadBatchTextures[Rdata->m_QuadBatchTextureSlotsUsed] = texture;
				textureSlot = Rdata->m_QuadBatchTextureSlotsUsed;
				Rdata->m_QuadBatchTextureSlotsUsed++;
			}
		}

		float distance = 0.5f * scale;
		float sine = std::sin(rotationInRadians);
		float cosine = std::cos(rotationInRadians);

		glm::vec2 relPosV0 = glm::vec2((-distance) * cosine - (-distance) * sine, (-distance) * sine + (-distance) * cosine);
		glm::vec2 relPosV1 = glm::vec2((-distance) * cosine - (+distance) * sine, (-distance) * sine + (+distance) * cosine);
		glm::vec2 relPosV2 = glm::vec2((+distance) * cosine - (+distance) * sine, (+distance) * sine + (+distance) * cosine);
		glm::vec2 relPosV3 = glm::vec2((+distance) * cosine - (-distance) * sine, (+distance) * sine + (-distance) * cosine);

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + relPosV0;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + relPosV1;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + relPosV2;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;

		Rdata->m_QuadBatchVertexBufferDataPtr->Position = position + relPosV3;
		Rdata->m_QuadBatchVertexBufferDataPtr->Color = color;
		Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
		Rdata->m_QuadBatchVertexBufferDataPtr++;
	}

	void Renderer::DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, std::shared_ptr<Texture> texture)
	{
		assert(count < c_MaxQuadsPerBatch);

		if ((Rdata->m_QuadBatchVertexBufferDataPtr - Rdata->m_QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > count * 4 ||
			Rdata->m_QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture == nullptr)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < Rdata->m_QuadBatchTextureSlotsUsed; i++)
			{
				if (Rdata->m_QuadBatchTextures[i]->GetRendererID() == texture->GetRendererID())
				{
					foundTexture = true;
					textureSlot = i;
					break;
				}
			}

			if (!foundTexture)
			{
				Rdata->m_QuadBatchTextures[Rdata->m_QuadBatchTextureSlotsUsed] = texture;
				textureSlot = Rdata->m_QuadBatchTextureSlotsUsed;
				Rdata->m_QuadBatchTextureSlotsUsed++;
			}
		}

		for (size_t i = 0; i < count; i++)
		{
			Rdata->m_QuadBatchVertexBufferDataPtr->Position = position[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Color = color[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
			Rdata->m_QuadBatchVertexBufferDataPtr++;

			Rdata->m_QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(0.0f, 1.0f) * scale[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Color = color[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
			Rdata->m_QuadBatchVertexBufferDataPtr++;

			Rdata->m_QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(1.0f, 1.0f) * scale[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Color = color[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
			Rdata->m_QuadBatchVertexBufferDataPtr++;

			Rdata->m_QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(1.0f, 0.0f) * scale[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Color = color[i];
			Rdata->m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			Rdata->m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
			Rdata->m_QuadBatchVertexBufferDataPtr++;
		}
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
	{
		vertexBuffer.Bind();
		indexBuffer.Bind();

		Rdata->m_CurrentActiveAPI->Draw(shader, primitive, indexBuffer);

		vertexBuffer.Unbind();

		Rdata->CurrentNumberOfDrawCalls++;
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer, int vertexCount)
	{
		vertexBuffer.Bind();

		Rdata->m_CurrentActiveAPI->Draw(shader, primitive, indexBuffer, vertexCount);

		vertexBuffer.Unbind();

		Rdata->CurrentNumberOfDrawCalls++;
	}

	void Renderer::ClearScreen()
	{
		Rdata->m_CurrentActiveAPI->ClearScreen();
	}

	void Renderer::Present()
	{
		Rdata->m_CurrentActiveAPI->Present();
	}

	void Renderer::RecreateSwapchain(const glm::vec2& newSwapchainSize)
	{
		Rdata->m_CurrentActiveAPI->RecreateSwapchain(newSwapchainSize);
	}

	void Renderer::Blur(std::shared_ptr<FrameBuffer>& target, float radius)
	{
		Rectangle lastViewport = Renderer::GetCurrentViewport();

		Rectangle viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (int)target->GetSize().x;
		viewport.Height = (int)target->GetSize().y;

		Renderer::SetViewport(viewport);
		auto& shader = Rdata->ShaderLibrary["BlurShader"];
		shader->BindUniformBuffer(Rdata->m_BlurUniformBuffer, 1, RenderingStage::FragmentShader);

		auto resolution = target->GetSize();
		//make a buffer for all the uniform data
		uint8_t bufferData[20];
		glm::vec2 horizonatlDirection = glm::vec2(1.0f, 0.0f);
		glm::vec2 verticalDirection = glm::vec2(0.0f, 1.0f);
		memset(bufferData, 0, 20);

		memcpy(bufferData, &resolution, sizeof(glm::vec2));
		memcpy(bufferData + 8, &horizonatlDirection, sizeof(glm::vec2));
		memcpy(bufferData + 16, &radius, sizeof(float));
		Rdata->m_BlurUniformBuffer->UpdateData(bufferData);

		//Horizontal blur
		Rdata->m_BlurFrameBuffer->Resize(target->GetSize());

		Rdata->m_BlurFrameBuffer->Bind();
		ClearScreen();

		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		shader->BindTexture(target, 0, RenderingStage::FragmentShader);
		Draw(*Rdata->m_BlurVertexBuffer, *shader, Primitive::Triangles, 6);

		//this specifies that we are doing vertical blur
		memcpy(bufferData + 8, &verticalDirection, sizeof(glm::vec2));
		Rdata->m_BlurUniformBuffer->UpdateData(bufferData);

		//clear the buffer we recieved
		target->Bind();
		Renderer::ClearScreen();

		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		shader->BindTexture(Rdata->m_BlurFrameBuffer, 0, RenderingStage::FragmentShader);
		Draw(*Rdata->m_BlurVertexBuffer, *shader, Primitive::Triangles, 6);

		Renderer::SetViewport(lastViewport);
	}

	void Renderer::SetBlendMode(RenderingBlendMode blendMode)
	{
		Rdata->m_CurrentActiveAPI->SetBlendMode(blendMode);
		Rdata->m_CurrentBlendMode = blendMode;
	}

	void Renderer::SetViewport(const Rectangle& viewport)
	{
		Rdata->m_CurrentActiveAPI->SetViewport(viewport);
	}

	Rectangle Renderer::GetCurrentViewport()
	{
		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return Rdata->m_CurrentActiveAPI->GetCurrentViewport();

		case RendererType::D3D11:
			return Rdata->m_CurrentActiveAPI->GetCurrentViewport();

		default:
			assert(false);
			return Rectangle();
		}
	}

	void Renderer::SetScissor(const Rectangle& scissor)
	{
		Rdata->m_CurrentActiveAPI->SetScissor(scissor);
	}

	Rectangle Renderer::GetCurrentScissor()
	{
		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return Rdata->m_CurrentActiveAPI->GetCurrentScissor();

		default:
			assert(false);
			return Rectangle();
		}
	}

	void Renderer::SetRenderTargetApplicationWindow()
	{
		Rdata->m_CurrentActiveAPI->SetRenderTargetApplicationWindow();
	}

	std::shared_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, unsigned int size,
		const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
		bool dynamic)
	{
		std::shared_ptr<VertexBuffer> buffer;

		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			buffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(data, size, layout, dynamic);
			break;

#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
			buffer = std::make_shared<D3D11::D3D11VertexBuffer>(data, size, layout, shaderProgram, dynamic, Rdata->m_CurrentActiveAPI->GetContext());
			break;
#endif // PLATFORM_WINDOWS

		default:
			assert(false);
		}

		Rdata->m_ReservedVertexBuffers.push_back(buffer);

		return buffer;
	}

	std::shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unsigned int* data, const int& count)
	{
		std::shared_ptr<IndexBuffer> buffer;

		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			buffer = std::make_shared<OpenGL::OpenGLIndexBuffer>(data, count);
			break;

		case RendererType::D3D11:
			buffer = std::make_shared<D3D11::D3D11IndexBuffer>(data, count, Rdata->m_CurrentActiveAPI->GetContext());
			break;

		default:
			assert(false);
		}

		Rdata->m_ReservedIndexBuffers.push_back(buffer);

		return buffer;
	}

	std::shared_ptr<ShaderProgram> Renderer::CreateShaderProgram(const std::string& vertPath, const std::string& fragPath)
	{
		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLShaderProgram>(vertPath, fragPath);

		case RendererType::D3D11:
			return std::make_shared<D3D11::D3D11ShaderProgram>(vertPath, fragPath, Rdata->m_CurrentActiveAPI->GetContext());

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<ShaderProgram> Renderer::CreateShaderProgramRaw(const std::string& vertSrc, const std::string& fragSrc)
	{
		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return OpenGL::OpenGLShaderProgram::CreateRaw(vertSrc, fragSrc);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<FrameBuffer> Renderer::CreateFrameBuffer(const glm::vec2& size)
	{
		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLFrameBuffer>(size);

#ifdef PLATFORM_WINDOWS

		case RendererType::D3D11:
			return std::make_shared<D3D11::D3D11FrameBuffer>(Rdata->m_CurrentActiveAPI->GetContext());
#endif

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<Texture> Renderer::CreateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data)
	{
		std::shared_ptr<Texture> texture;

		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			texture = std::make_shared<OpenGL::OpenGLTexture>(size, format, data);
			break;

#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
			texture = std::make_shared<D3D11::D3D11Texture>(size, data, Rdata->m_CurrentActiveAPI->GetContext());
			break;
#endif // PLATFORM_WINDOWS

		default:
			assert(false);
		}

		Rdata->m_ReservedTextures.push_back(texture);

		return texture;
	}

	std::shared_ptr<Texture> Renderer::CreateTexture(Image& img)
	{
		std::shared_ptr<Texture> texture;

		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			texture = std::make_shared<OpenGL::OpenGLTexture>(glm::vec2(img.m_Width, img.m_Height), img.Format, img.m_Data);
			break;

#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
			texture = std::make_shared<D3D11::D3D11Texture>(glm::vec2(img.m_Width, img.m_Height), img.m_Data, Rdata->m_CurrentActiveAPI->GetContext());
			break;
#endif // PLATFORM_WINDOWS

		default:
			assert(false);
		}

		Rdata->m_ReservedTextures.push_back(texture);

		return texture;
	}

	std::shared_ptr<UniformBuffer> Renderer::CreateUniformBuffer(const std::string& name, uint32_t reg, const VertexLayout& layout, void* data)
	{
		std::shared_ptr<UniformBuffer> buffer;

		switch (Rdata->m_CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			buffer = std::make_shared<OpenGL::OpenGLUniformBuffer>(name, layout, data);
			break;

#ifdef PLATFORM_WINDOWS
		case RendererType::D3D11:
			buffer = std::make_shared<D3D11::D3D11UniformBuffer>(name, reg, layout, data, Rdata->m_CurrentActiveAPI->GetContext());
			break;
#endif // PLATFORM_WINDOWS

		default:
			assert(false);
		}

		return buffer;
	}

	void Ainan::Renderer::FlushQuadBatch()
	{
		for (size_t i = 0; i < Rdata->m_QuadBatchTextureSlotsUsed; i++)
			Rdata->ShaderLibrary["QuadBatchShader"]->BindTexture(Rdata->m_QuadBatchTextures[i], i, RenderingStage::FragmentShader);

		int numVertices = (Rdata->m_QuadBatchVertexBufferDataPtr - Rdata->m_QuadBatchVertexBufferDataOrigin);

		Rdata->m_QuadBatchVertexBuffer->UpdateData(0,
			numVertices * sizeof(QuadVertex),
			Rdata->m_QuadBatchVertexBufferDataOrigin);

		Renderer::Draw(*Rdata->m_QuadBatchVertexBuffer,
			*Rdata->ShaderLibrary["QuadBatchShader"],
			Primitive::Triangles,
			*Rdata->m_QuadBatchIndexBuffer,
			(numVertices * 3) / 2);

		//reset data so we can accept the next batch
		Rdata->m_QuadBatchVertexBufferDataPtr = Rdata->m_QuadBatchVertexBufferDataOrigin;
		Rdata->m_QuadBatchTextureSlotsUsed = 1;
	}
}