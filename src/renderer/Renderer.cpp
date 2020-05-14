#include <pch.h>

#include "Renderer.h"

#include "opengl/OpenGLRendererAPI.h"
#include "opengl/OpenGLShaderProgram.h"
#include "opengl/OpenGLVertexArray.h"
#include "opengl/OpenGLVertexBuffer.h"
#include "opengl/OpenGLIndexBuffer.h"
#include "opengl/OpenGLTexture.h"
#include "opengl/OpenGLFrameBuffer.h"

#ifdef PLATFORM_WINDOWS

#include "d3d11/D3D11RendererAPI.h"

#endif // PLATFORM_WINDOWS


namespace Ainan {

	RendererAPI* Renderer::m_CurrentActiveAPI = nullptr;
	//Camera* Renderer::m_CurrentSceneCamera = nullptr;
	SceneDescription Renderer::m_CurrentSceneDescription = {};
	glm::mat4 Renderer::m_CurrentViewProjection = glm::mat4(1.0f);
	unsigned int Renderer::NumberOfDrawCallsLastScene = 0;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> Renderer::ShaderLibrary;

	//profiler data
	std::vector<std::weak_ptr<Texture>> Renderer::m_ReservedTextures;
	std::vector<std::weak_ptr<VertexBuffer>> Renderer::m_ReservedVertexBuffers;
	std::vector<std::weak_ptr<IndexBuffer>> Renderer::m_ReservedIndexBuffers;
	static unsigned int s_CurrentNumberOfDrawCalls = 0;

	//batch renderer data
	std::shared_ptr<VertexBuffer> Renderer::m_QuadBatchVertexBuffer = nullptr;
	std::shared_ptr<VertexArray> Renderer::m_QuadBatchVertexArray = nullptr;
	std::shared_ptr<IndexBuffer> Renderer::m_QuadBatchIndexBuffer = nullptr;
	QuadVertex* Renderer::m_QuadBatchVertexBufferDataOrigin = nullptr;
	QuadVertex* Renderer::m_QuadBatchVertexBufferDataPtr = nullptr;
	std::array<std::shared_ptr<Texture>, c_MaxQuadTexturesPerBatch> Renderer::m_QuadBatchTextures;
	int Renderer::m_QuadBatchTextureSlotsUsed = 1;

	//postprocessing data
	std::shared_ptr<Texture>      Renderer::m_BlurTexture = nullptr;
	std::shared_ptr<FrameBuffer>  Renderer::m_BlurFrameBuffer = nullptr;
	std::shared_ptr<VertexArray>  Renderer::m_BlurVertexArray = nullptr;
	std::shared_ptr<VertexBuffer> Renderer::m_BlurVertexBuffer = nullptr;

	RenderingBlendMode Renderer::m_CurrentBlendMode = RenderingBlendMode::Additive;

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
		{ "BackgroundShader"    , "shaders/Background.vert"    , "shaders/Background.frag"     },
		{ "CircleOutlineShader" , "shaders/CircleOutline.vert" , "shaders/FlatColor.frag"      },
		{ "LineShader"          , "shaders/Line.vert"          , "shaders/FlatColor.frag"      },
		{ "BlurShader"          , "shaders/Image.vert"         , "shaders/Blur.frag"           },
		{ "GizmoShader"         , "shaders/Gizmo.vert"         , "shaders/FlatColor.frag"      },
		{ "ImageShader"         , "shaders/Image.vert"         , "shaders/Image.frag"          },
		{ "QuadBatchShader"     , "shaders/QuadBatch.vert"     , "shaders/QuadBatch.frag"      }
	};

	void Renderer::Init(RendererType api)
	{
		//initilize the renderer api
		switch (api)
		{
		case RendererType::D3D11:
			m_CurrentActiveAPI = new D3D11::D3D11RendererAPI();
			break;

		case RendererType::OpenGL:
			m_CurrentActiveAPI = new OpenGL::OpenGLRendererAPI();
			break;
		}
		//TEMP
		if (api == RendererType::D3D11)
			return;

		//load shaders
		for (auto& shaderInfo : CompileOnInit)
		{
			ShaderLibrary[shaderInfo.Name] = Renderer::CreateShaderProgram(shaderInfo.VertexCodePath, shaderInfo.FragmentCodePath);
		}

		//setup batch renderer
		m_QuadBatchVertexArray = CreateVertexArray();
		m_QuadBatchVertexArray->Bind();

		m_QuadBatchVertexBuffer = CreateVertexBuffer(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex), true);
		m_QuadBatchVertexBuffer->Bind();
		m_QuadBatchVertexBuffer->SetLayout(
			{
				ShaderVariableType::Vec2,  //Position
				ShaderVariableType::Vec4,  //Color;
				ShaderVariableType::Float, //Texture;
				ShaderVariableType::Vec2   //Texture Coordinates;
			}
		);

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
		m_QuadBatchIndexBuffer = CreateIndexBuffer(indicies, c_MaxQuadsPerBatch * 6);
		m_QuadBatchVertexArray->Unbind();
		delete[] indicies;

		m_QuadBatchVertexBufferDataOrigin = new QuadVertex[c_MaxQuadVerticesPerBatch];
		m_QuadBatchVertexBufferDataPtr = m_QuadBatchVertexBufferDataOrigin;

		m_QuadBatchTextures[0] = CreateTexture();
		m_QuadBatchTextures[0]->SetDefaultTextureSettings();
		Image img;
		img.m_Width = 1;
		img.m_Height = 1;
		img.m_Comp = 4;
		img.m_Data = new unsigned char[4];
		memset(img.m_Data, (unsigned char)255, 4);
		m_QuadBatchTextures[0]->SetImage(img);

		for (size_t i = 0; i < c_MaxQuadTexturesPerBatch; i++) {
			std::string name = "u_Textures[" + std::to_string(i) + "]";
			ShaderLibrary["QuadBatchShader"]->SetUniform1i(name.c_str(), i);
		}

		//setup postprocessing
		m_BlurTexture = CreateTexture();
		m_BlurFrameBuffer = CreateFrameBuffer();
		m_BlurVertexArray = CreateVertexArray();
		m_BlurVertexArray->Bind();

		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		m_BlurVertexBuffer = CreateVertexBuffer(quadVertices, sizeof(quadVertices));
		m_BlurVertexBuffer->SetLayout({ ShaderVariableType::Vec2, ShaderVariableType::Vec2 });
		m_BlurVertexArray->Unbind();

		SetBlendMode(m_CurrentBlendMode);
	}

	void Renderer::Terminate()
	{
		//TEMP
		if (m_CurrentActiveAPI->GetType() == RendererType::D3D11)
			return;

		ShaderLibrary.erase(ShaderLibrary.begin(), ShaderLibrary.end());

		//batch renderer data
		m_QuadBatchVertexBuffer.reset();
		m_QuadBatchVertexArray.reset();
		m_QuadBatchIndexBuffer.reset();
		delete[] m_QuadBatchVertexBufferDataOrigin;
		m_QuadBatchTextures[0].reset();
	}

	void Renderer::BeginScene(const SceneDescription& desc)
	{
		assert(desc.SceneDrawTarget);

		m_CurrentSceneDescription = desc;
		m_CurrentViewProjection = desc.SceneCamera.ProjectionMatrix * desc.SceneCamera.ViewMatrix;
		s_CurrentNumberOfDrawCalls = 0;

		m_CurrentSceneDescription.SceneDrawTarget->Bind();
		ClearScreen();


		//update diagnostics stuff
		for (size_t i = 0; i < m_ReservedTextures.size(); i++)
			if (m_ReservedTextures[i].expired())
			{
				m_ReservedTextures.erase(m_ReservedTextures.begin() + i);
				i = 0;
			}
		for (size_t i = 0; i < m_ReservedVertexBuffers.size(); i++)
			if (m_ReservedVertexBuffers[i].expired())
			{
				m_ReservedVertexBuffers.erase(m_ReservedVertexBuffers.begin() + i);
				i = 0;
			}
		for (size_t i = 0; i < m_ReservedIndexBuffers.size(); i++)
			if (m_ReservedIndexBuffers[i].expired())
			{
				m_ReservedIndexBuffers.erase(m_ReservedIndexBuffers.begin() + i);
				i = 0;
			}
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
	{
		vertexArray.Bind();
		shader.Bind();
		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->Draw(shader, mode, vertexCount);

		vertexArray.Unbind();
		shader.Unbind();

		s_CurrentNumberOfDrawCalls++;
	}

	void Renderer::EndScene()
	{
		if(m_QuadBatchVertexBufferDataPtr != m_QuadBatchVertexBufferDataOrigin)
			FlushQuadBatch();

		if (m_CurrentSceneDescription.Blur && m_CurrentBlendMode != RenderingBlendMode::Screen)
			Blur(m_CurrentSceneDescription.SceneDrawTarget, m_CurrentSceneDescription.SceneDrawTargetTexture, m_CurrentSceneDescription.BlurRadius);

		m_CurrentSceneDescription.SceneDrawTarget->Unbind();

		memset(&m_CurrentSceneDescription, 0, sizeof(SceneDescription));
		NumberOfDrawCallsLastScene = s_CurrentNumberOfDrawCalls;
	}

	void Ainan::Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, std::shared_ptr<Texture> texture)
	{
		if ((m_QuadBatchVertexBufferDataPtr - m_QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
			m_QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture == nullptr)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < m_QuadBatchTextureSlotsUsed; i++)
			{
				if (m_QuadBatchTextures[i]->GetRendererID() == texture->GetRendererID())
				{
					foundTexture = true;
					textureSlot = i;
					break;
				}
			}
			
			if (!foundTexture)
			{
				m_QuadBatchTextures[m_QuadBatchTextureSlotsUsed] = texture;
				textureSlot = m_QuadBatchTextureSlotsUsed;
				m_QuadBatchTextureSlotsUsed++;
			}
		}

		m_QuadBatchVertexBufferDataPtr->Position = position;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
		m_QuadBatchVertexBufferDataPtr++;

		m_QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(0.0f, 1.0f) * scale;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
		m_QuadBatchVertexBufferDataPtr++;

		m_QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(1.0f, 1.0f) * scale;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
		m_QuadBatchVertexBufferDataPtr++;

		m_QuadBatchVertexBufferDataPtr->Position = position + glm::vec2(1.0f, 0.0f) * scale;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
		m_QuadBatchVertexBufferDataPtr++;
	}

	void Renderer::DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, std::shared_ptr<Texture> texture)
	{
		if ((m_QuadBatchVertexBufferDataPtr - m_QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > 4 ||
			m_QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture == nullptr)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < m_QuadBatchTextureSlotsUsed; i++)
			{
				if (m_QuadBatchTextures[i]->GetRendererID() == texture->GetRendererID())
				{
					foundTexture = true;
					textureSlot = i;
					break;
				}
			}

			if (!foundTexture)
			{
				m_QuadBatchTextures[m_QuadBatchTextureSlotsUsed] = texture;
				textureSlot = m_QuadBatchTextureSlotsUsed;
				m_QuadBatchTextureSlotsUsed++;
			}
		}

		float distance = 0.5f * scale;
		float sine = std::sin(rotationInRadians);
		float cosine = std::cos(rotationInRadians);

		glm::vec2 relPosV0 = glm::vec2((-distance) * cosine - (-distance) * sine, (-distance) * sine + (-distance) * cosine);
		glm::vec2 relPosV1 = glm::vec2((-distance) * cosine - (+distance) * sine, (-distance) * sine + (+distance) * cosine);
		glm::vec2 relPosV2 = glm::vec2((+distance) * cosine - (+distance) * sine, (+distance) * sine + (+distance) * cosine);
		glm::vec2 relPosV3 = glm::vec2((+distance) * cosine - (-distance) * sine, (+distance) * sine + (-distance) * cosine);

		m_QuadBatchVertexBufferDataPtr->Position = position + relPosV0;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
		m_QuadBatchVertexBufferDataPtr++;

		m_QuadBatchVertexBufferDataPtr->Position = position + relPosV1;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
		m_QuadBatchVertexBufferDataPtr++;

		m_QuadBatchVertexBufferDataPtr->Position = position + relPosV2;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
		m_QuadBatchVertexBufferDataPtr++;

		m_QuadBatchVertexBufferDataPtr->Position = position + relPosV3;
		m_QuadBatchVertexBufferDataPtr->Color = color;
		m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
		m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
		m_QuadBatchVertexBufferDataPtr++;
	}

	void Renderer::DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, std::shared_ptr<Texture> texture)
	{
		assert(count < c_MaxQuadsPerBatch);

		if ((m_QuadBatchVertexBufferDataPtr - m_QuadBatchVertexBufferDataOrigin) / sizeof(QuadVertex) > count * 4 ||
			m_QuadBatchTextureSlotsUsed == c_MaxQuadTexturesPerBatch)
			FlushQuadBatch();

		float textureSlot;
		if (texture == nullptr)
			textureSlot = 0.0f;
		else
		{
			bool foundTexture = false;
			//check if texture is already used
			for (size_t i = 0; i < m_QuadBatchTextureSlotsUsed; i++)
			{
				if (m_QuadBatchTextures[i]->GetRendererID() == texture->GetRendererID())
				{
					foundTexture = true;
					textureSlot = i;
					break;
				}
			}

			if (!foundTexture)
			{
				m_QuadBatchTextures[m_QuadBatchTextureSlotsUsed] = texture;
				textureSlot = m_QuadBatchTextureSlotsUsed;
				m_QuadBatchTextureSlotsUsed++;
			}
		}

		for (size_t i = 0; i < count; i++)
		{
			m_QuadBatchVertexBufferDataPtr->Position = position[i];
			m_QuadBatchVertexBufferDataPtr->Color = color[i];
			m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 0.0f };
			m_QuadBatchVertexBufferDataPtr++;

			m_QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(0.0f, 1.0f) * scale[i];
			m_QuadBatchVertexBufferDataPtr->Color = color[i];
			m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 0.0f, 1.0f };
			m_QuadBatchVertexBufferDataPtr++;

			m_QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(1.0f, 1.0f) * scale[i];
			m_QuadBatchVertexBufferDataPtr->Color = color[i];
			m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 1.0f };
			m_QuadBatchVertexBufferDataPtr++;

			m_QuadBatchVertexBufferDataPtr->Position = position[i] + glm::vec2(1.0f, 0.0f) * scale[i];
			m_QuadBatchVertexBufferDataPtr->Color = color[i];
			m_QuadBatchVertexBufferDataPtr->Texture = textureSlot;
			m_QuadBatchVertexBufferDataPtr->TextureCoordinates = { 1.0f, 0.0f };
			m_QuadBatchVertexBufferDataPtr++;
		}
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->Draw(shader, primitive, indexBuffer);

		vertexArray.Unbind();
		shader.Unbind();

		s_CurrentNumberOfDrawCalls++;
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer, int vertexCount)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->Draw(shader, primitive, indexBuffer, vertexCount);

		vertexArray.Unbind();
		shader.Unbind();

		s_CurrentNumberOfDrawCalls++;
	}

	void Renderer::ClearScreen()
	{
		m_CurrentActiveAPI->ClearScreen();
	}

	void Renderer::Present()
	{
		m_CurrentActiveAPI->Present();
	}

	void Renderer::Blur(std::shared_ptr<FrameBuffer>& target, std::shared_ptr<Texture>& targetTexture,  float radius)
	{
		Rectangle lastViewport = Renderer::GetCurrentViewport();

		Rectangle viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (int)target->GetSize().x;
		viewport.Height = (int)target->GetSize().y;

		Renderer::SetViewport(viewport);
		auto& shader = Renderer::ShaderLibrary["BlurShader"];

		shader->SetUniformVec2("u_Resolution", target->GetSize());
		shader->SetUniform1f("u_Radius", radius);
		shader->SetUniform1i("u_BlurTarget", 0);

		//Horizontal blur

		m_BlurTexture->SetImage(target->GetSize());
		m_BlurTexture->SetDefaultTextureSettings();
		m_BlurFrameBuffer->Bind();
		m_BlurFrameBuffer->SetActiveTexture(*m_BlurTexture);

		m_BlurFrameBuffer->Bind();
		ClearScreen();

		//this specifies that we are doing horizontal blur
		shader->SetUniformVec2("u_BlurDirection", glm::vec2(1.0f, 0.0f));

		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		targetTexture->Bind();
		Draw(*m_BlurVertexArray, *shader, Primitive::Triangles, 6);

		//this specifies that we are doing vertical blur
		shader->SetUniformVec2("u_BlurDirection", glm::vec2(0.0f, 1.0f));

		//clear the buffer we recieved
		target->Bind();
		Renderer::ClearScreen();

		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		m_BlurTexture->Bind();
		Draw(*m_BlurVertexArray, *shader, Primitive::Triangles, 6);

		Renderer::SetViewport(lastViewport);
	}

	void Renderer::SetBlendMode(RenderingBlendMode blendMode)
	{
		m_CurrentActiveAPI->SetBlendMode(blendMode);
		m_CurrentBlendMode = blendMode;
	}

	void Renderer::SetViewport(const Rectangle& viewport)
	{
		m_CurrentActiveAPI->SetViewport(viewport);
	}

	Rectangle Renderer::GetCurrentViewport()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return m_CurrentActiveAPI->GetCurrentViewport();

		default:
			assert(false);
			return Rectangle();
		}
	}

	void Renderer::SetScissor(const Rectangle& scissor)
	{
		m_CurrentActiveAPI->SetScissor(scissor);
	}

	Rectangle Renderer::GetCurrentScissor()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return m_CurrentActiveAPI->GetCurrentScissor();

		default:
			assert(false);
			return Rectangle();
		}
	}

	std::shared_ptr<VertexArray> Renderer::CreateVertexArray()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLVertexArray>();

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, unsigned int size, bool dynamic)
	{
		std::shared_ptr<VertexBuffer> buffer;

		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			buffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(data, size, dynamic);
			break;

		default:
			assert(false);
		}

		m_ReservedVertexBuffers.push_back(buffer);

		return buffer;
	}

	std::shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unsigned int* data, const int& count)
	{
		std::shared_ptr<IndexBuffer> buffer;

		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			buffer = std::make_shared<OpenGL::OpenGLIndexBuffer>(data, count);
			break;

		default:
			assert(false);
		}

		m_ReservedIndexBuffers.push_back(buffer);

		return buffer;
	}

	std::shared_ptr<ShaderProgram> Renderer::CreateShaderProgram(const std::string& vertPath, const std::string& fragPath)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLShaderProgram>(vertPath, fragPath);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<ShaderProgram> Renderer::CreateShaderProgramRaw(const std::string& vertSrc, const std::string& fragSrc)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return OpenGL::OpenGLShaderProgram::CreateRaw(vertSrc, fragSrc);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<FrameBuffer> Renderer::CreateFrameBuffer()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLFrameBuffer>();

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<Texture> Renderer::CreateTexture()
	{
		std::shared_ptr<Texture> texture;

		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			texture = std::make_shared<OpenGL::OpenGLTexture>();
			break;

		default:
			assert(false);
		}

		m_ReservedTextures.push_back(texture);

		return texture;
	}

	void Ainan::Renderer::FlushQuadBatch()
	{
		for (size_t i = 0; i < m_QuadBatchTextureSlotsUsed; i++)
			m_QuadBatchTextures[i]->Bind(i);

		int numVertices = (m_QuadBatchVertexBufferDataPtr - m_QuadBatchVertexBufferDataOrigin);

		m_QuadBatchVertexBuffer->UpdateData(0,
			numVertices * sizeof(QuadVertex),
			m_QuadBatchVertexBufferDataOrigin);

		Renderer::Draw(*m_QuadBatchVertexArray,
			*ShaderLibrary["QuadBatchShader"],
			Primitive::Triangles,
			*m_QuadBatchIndexBuffer,
			(numVertices * 3) / 2);

		//reset data so we can accept the next batch
		m_QuadBatchVertexBufferDataPtr = m_QuadBatchVertexBufferDataOrigin;
		m_QuadBatchTextureSlotsUsed = 1;
	}
}