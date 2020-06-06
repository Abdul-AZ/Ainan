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

			Rdata->QuadBatchVertexBuffer = CreateVertexBuffer(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex),
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
		Rdata->QuadBatchIndexBuffer = CreateIndexBuffer(indicies, c_MaxQuadsPerBatch * 6);
		delete[] indicies;

		Rdata->QuadBatchVertexBufferDataOrigin = new QuadVertex[c_MaxQuadVerticesPerBatch];
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;

		Rdata->QuadBatchTextures[0] = CreateTexture(glm::vec2(1,1), TextureFormat::RGBA);
		Rdata->QuadBatchTextures[0]->SetDefaultTextureSettings();
		Image img;
		img.m_Width = 1;
		img.m_Height = 1;
		img.Format = TextureFormat::RGBA;
		img.m_Data = new unsigned char[4];
		memset(img.m_Data, (unsigned char)255, 4);
		Rdata->QuadBatchTextures[0]->SetImage(img);

		//setup postprocessing
		Rdata->BlurFrameBuffer = CreateFrameBuffer(Window::FramebufferSize);

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
			Rdata->BlurVertexBuffer = CreateVertexBuffer(quadVertices, sizeof(quadVertices), layout, Rdata->ShaderLibrary["BlurShader"]);
		}
		{
			VertexLayout bufferLayout =
			{
				{ "u_Resolution", ShaderVariableType::Vec2 },
				{ "u_BlurDirection", ShaderVariableType::Vec2 },
				{ "u_Radius", ShaderVariableType::Float }
			};
			Rdata->BlurUniformBuffer = CreateUniformBuffer("BlurData", 1, bufferLayout, nullptr);
			Rdata->ShaderLibrary["BlurShader"]->BindUniformBuffer(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);
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

		auto type = Rdata->CurrentActiveAPI->GetContext()->GetType();

		delete Rdata->CurrentActiveAPI;

		//batch renderer data
		Rdata->QuadBatchVertexBuffer.reset();
		Rdata->QuadBatchIndexBuffer.reset();
		delete[] Rdata->QuadBatchVertexBufferDataOrigin;
		Rdata->QuadBatchTextures[0].reset();

		delete Rdata;
	}

	void Renderer::BeginScene(const SceneDescription& desc)
	{
		assert(desc.SceneDrawTarget);

		Rdata->CurrentSceneDescription = desc;
		Rdata->CurrentViewProjection = desc.SceneCamera.ProjectionMatrix * desc.SceneCamera.ViewMatrix;
		Rdata->CurrentNumberOfDrawCalls = 0;

		(*Rdata->CurrentSceneDescription.SceneDrawTarget)->Bind();
		ClearScreen();

		//update the per-frame uniform buffer
		Rdata->SceneUniformbuffer->UpdateData(&Rdata->CurrentViewProjection);

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
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
	{
		vertexBuffer.Bind();

		Rdata->CurrentActiveAPI->Draw(shader, mode, vertexCount);

		vertexBuffer.Unbind();

		Rdata->CurrentNumberOfDrawCalls++;
	}

	void Renderer::EndScene()
	{
		if(Rdata->QuadBatchVertexBufferDataPtr != Rdata->QuadBatchVertexBufferDataOrigin)
			FlushQuadBatch();

		if (Rdata->CurrentSceneDescription.Blur && Rdata->m_CurrentBlendMode != RenderingBlendMode::Screen)
		{
			Blur(*Rdata->CurrentSceneDescription.SceneDrawTarget, Rdata->CurrentSceneDescription.BlurRadius);
		}

		SetRenderTargetApplicationWindow();

		memset(&Rdata->CurrentSceneDescription, 0, sizeof(SceneDescription));
		Rdata->NumberOfDrawCallsLastScene = Rdata->CurrentNumberOfDrawCalls;
	}

	void Ainan::Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, std::shared_ptr<Texture> texture)
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
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, std::shared_ptr<Texture> texture)
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
	}

	void Renderer::DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, std::shared_ptr<Texture> texture)
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
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
	{
		vertexBuffer.Bind();
		indexBuffer.Bind();

		Rdata->CurrentActiveAPI->Draw(shader, primitive, indexBuffer);

		vertexBuffer.Unbind();
		indexBuffer.Unbind();

		Rdata->CurrentNumberOfDrawCalls++;
	}

	void Renderer::Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer, int vertexCount)
	{
		vertexBuffer.Bind();
		indexBuffer.Bind();

		Rdata->CurrentActiveAPI->Draw(shader, primitive, indexBuffer, vertexCount);

		vertexBuffer.Unbind();
		indexBuffer.Unbind();

		Rdata->CurrentNumberOfDrawCalls++;
	}

	void Renderer::ClearScreen()
	{
		Rdata->CurrentActiveAPI->ClearScreen();
	}

	void Renderer::Present()
	{
		Rdata->CurrentActiveAPI->Present();
	}

	void Renderer::RecreateSwapchain(const glm::vec2& newSwapchainSize)
	{
		Rdata->CurrentActiveAPI->RecreateSwapchain(newSwapchainSize);
	}

	void Renderer::Blur(std::shared_ptr<FrameBuffer>& target, float radius)
	{
		Rectangle lastViewport = Renderer::GetCurrentViewport();
		RenderingBlendMode lastBlendMode = Rdata->m_CurrentBlendMode;
		SetBlendMode(RenderingBlendMode::Screen);

		Rectangle viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (int)target->GetSize().x;
		viewport.Height = (int)target->GetSize().y;

		Renderer::SetViewport(viewport);
		auto& shader = Rdata->ShaderLibrary["BlurShader"];
		shader->BindUniformBuffer(Rdata->BlurUniformBuffer, 1, RenderingStage::FragmentShader);

		auto resolution = target->GetSize();
		//make a buffer for all the uniform data
		uint8_t bufferData[20];
		glm::vec2 horizonatlDirection = glm::vec2(1.0f, 0.0f);
		glm::vec2 verticalDirection = glm::vec2(0.0f, 1.0f);
		memset(bufferData, 0, 20);

		memcpy(bufferData, &resolution, sizeof(glm::vec2));
		memcpy(bufferData + 8, &horizonatlDirection, sizeof(glm::vec2));
		memcpy(bufferData + 16, &radius, sizeof(float));
		Rdata->BlurUniformBuffer->UpdateData(bufferData);

		//Horizontal blur
		Rdata->BlurFrameBuffer->Resize(target->GetSize());

		Rdata->BlurFrameBuffer->Bind();
		ClearScreen();

		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		shader->BindTexture(target, 0, RenderingStage::FragmentShader);
		Draw(*Rdata->BlurVertexBuffer, *shader, Primitive::Triangles, 6);

		//this specifies that we are doing vertical blur
		memcpy(bufferData + 8, &verticalDirection, sizeof(glm::vec2));
		Rdata->BlurUniformBuffer->UpdateData(bufferData);

		//clear the buffer we recieved
		target->Bind();
		Renderer::ClearScreen();

		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		shader->BindTexture(Rdata->BlurFrameBuffer, 0, RenderingStage::FragmentShader);
		Draw(*Rdata->BlurVertexBuffer, *shader, Primitive::Triangles, 6);

		Renderer::SetViewport(lastViewport);
		SetBlendMode(lastBlendMode);
	}

	void Renderer::SetBlendMode(RenderingBlendMode blendMode)
	{
		Rdata->CurrentActiveAPI->SetBlendMode(blendMode);
		Rdata->m_CurrentBlendMode = blendMode;
	}

	void Renderer::SetViewport(const Rectangle& viewport)
	{
		Rdata->CurrentActiveAPI->SetViewport(viewport);
	}

	Rectangle Renderer::GetCurrentViewport()
	{
		return Rdata->CurrentActiveAPI->GetCurrentViewport();
	}

	void Renderer::SetScissor(const Rectangle& scissor)
	{
		Rdata->CurrentActiveAPI->SetScissor(scissor);
	}

	Rectangle Renderer::GetCurrentScissor()
	{
		return Rdata->CurrentActiveAPI->GetCurrentScissor();
	}

	void Renderer::SetRenderTargetApplicationWindow()
	{
		Rdata->CurrentActiveAPI->SetRenderTargetApplicationWindow();
	}

	std::shared_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, unsigned int size,
		const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
		bool dynamic)
	{
		std::shared_ptr<VertexBuffer> buffer;

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

		return buffer;
	}

	std::shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unsigned int* data, const int& count)
	{
		std::shared_ptr<IndexBuffer> buffer;

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
		switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
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
		switch (Rdata->CurrentActiveAPI->GetContext()->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLFrameBuffer>(size);

#ifdef PLATFORM_WINDOWS

		case RendererType::D3D11:
			return std::make_shared<D3D11::D3D11FrameBuffer>(size, Rdata->CurrentActiveAPI->GetContext());
#endif

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<Texture> Renderer::CreateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data)
	{
		std::shared_ptr<Texture> texture;

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

		return texture;
	}

	std::shared_ptr<Texture> Renderer::CreateTexture(Image& img)
	{
		std::shared_ptr<Texture> texture;

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

		return texture;
	}

	std::shared_ptr<UniformBuffer> Renderer::CreateUniformBuffer(const std::string& name, uint32_t reg, const VertexLayout& layout, void* data)
	{
		std::shared_ptr<UniformBuffer> buffer;

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

		Rdata->QuadBatchVertexBuffer->UpdateData(0,
			numVertices * sizeof(QuadVertex),
			Rdata->QuadBatchVertexBufferDataOrigin);

		Renderer::Draw(*Rdata->QuadBatchVertexBuffer,
			*Rdata->ShaderLibrary["QuadBatchShader"],
			Primitive::Triangles,
			*Rdata->QuadBatchIndexBuffer,
			(numVertices * 3) / 2);

		//reset data so we can accept the next batch
		Rdata->QuadBatchVertexBufferDataPtr = Rdata->QuadBatchVertexBufferDataOrigin;
		Rdata->QuadBatchTextureSlotsUsed = 1;
	}
}