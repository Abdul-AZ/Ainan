#include <pch.h>

#include "Renderer.h"

namespace Ainan {

	RendererAPI* Renderer::m_CurrentActiveAPI = nullptr;
	Camera* Renderer::m_CurrentSceneCamera = nullptr;
	glm::mat4 Renderer::m_CurrentViewProjection = glm::mat4(1.0f);
	unsigned int Renderer::NumberOfDrawCallsLastScene = 0;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> Renderer::ShaderLibrary;

	std::vector<std::weak_ptr<Texture>> Renderer::m_ReservedTextures;
	std::vector<std::weak_ptr<VertexBuffer>> Renderer::m_ReservedVertexBuffers;
	std::vector<std::weak_ptr<IndexBuffer>> Renderer::m_ReservedIndexBuffers;

	std::vector<std::shared_ptr<VertexBuffer>> Renderer::m_QuadBatchVertexBuffers = {};
	std::vector<std::shared_ptr<VertexArray>> Renderer::m_QuadBatchVertexArrays;
	std::shared_ptr<IndexBuffer> Renderer::m_QuadBatchIndexBuffer = nullptr;

	static unsigned int s_CurrentNumberOfDrawCalls = 0;

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
		{ "ParticleSystemShader", "shaders/ParticleSystem.vert", "shaders/ParticleSystem.frag" },
		{ "BackgroundShader"    , "shaders/Background.vert"    , "shaders/Background.frag"     },
		{ "SpriteShader"        , "shaders/Sprite.vert"        , "shaders/Sprite.frag"         },
		{ "CircleOutlineShader" , "shaders/CircleOutline.vert" , "shaders/FlatColor.frag"      },
		{ "LineShader"          , "shaders/Line.vert"          , "shaders/FlatColor.frag"      },
		{ "BlurShader"          , "shaders/Image.vert"         , "shaders/Blur.frag"           },
		{ "GizmoShader"         , "shaders/Gizmo.vert"         , "shaders/FlatColor.frag"      },
		{ "ImageShader"         , "shaders/Image.vert"         , "shaders/Image.frag"          },
		{ "QuadBatchShader"     , "shaders/QuadBatch.vert"     , "shaders/QuadBatch.frag"      }
	};

	void Renderer::Init()
	{
		//initilize the renderer api
		m_CurrentActiveAPI = new OpenGL::OpenGLRendererAPI();

		//load shaders
		for (auto& shaderInfo : CompileOnInit)
		{
			ShaderLibrary[shaderInfo.Name] = Renderer::CreateShaderProgram(shaderInfo.VertexCodePath, shaderInfo.FragmentCodePath);
		}

		//setup batch renderer
		m_QuadBatchVertexArrays.push_back(CreateVertexArray());
		m_QuadBatchVertexArrays[0]->Bind();
		m_QuadBatchVertexBuffers.push_back(CreateVertexBuffer(nullptr, c_MaxQuadVerticesPerBatch * sizeof(QuadVertex)));
		m_QuadBatchVertexBuffers[0]->Bind();
		unsigned int* indicies = new unsigned int[c_MaxQuadsPerBatch * 6];
		for (size_t i = 0; i < c_MaxQuadsPerBatch * 6; i+=6)
		{
			indicies[i + 0] = 0;
			indicies[i + 1] = 1;
			indicies[i + 2] = 2;

			indicies[i + 3] = 0;
			indicies[i + 4] = 2;
			indicies[i + 5] = 3;
		}
		m_QuadBatchIndexBuffer = CreateIndexBuffer(indicies, c_MaxQuadsPerBatch * 6);
		m_QuadBatchVertexArrays[0]->Unbind();
		delete[] indicies;
	}

	void Renderer::Terminate()
	{
		ShaderLibrary.erase(ShaderLibrary.begin(), ShaderLibrary.end());

		//batch renderer data
		m_QuadBatchVertexBuffers.erase(m_QuadBatchVertexBuffers.begin(), m_QuadBatchVertexBuffers.end());
		m_QuadBatchVertexArrays.erase(m_QuadBatchVertexArrays.begin(), m_QuadBatchVertexArrays.end());
		m_QuadBatchIndexBuffer.reset();
	}

	void Renderer::BeginScene(Camera& camera)
	{
		m_CurrentSceneCamera = &camera;
		m_CurrentViewProjection = camera.ProjectionMatrix * camera.ViewMatrix;
		s_CurrentNumberOfDrawCalls = 0;

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

	void Renderer::DrawInstanced(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount, const unsigned int& objectCount)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->DrawInstanced(shader, mode, vertexCount, objectCount);

		vertexArray.Unbind();
		shader.Unbind();

		s_CurrentNumberOfDrawCalls++;
	}

	void Renderer::EndScene()
	{
		m_CurrentSceneCamera = nullptr;
		NumberOfDrawCallsLastScene = s_CurrentNumberOfDrawCalls;
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

	void Renderer::ClearScreen()
	{
		m_CurrentActiveAPI->ClearScreen();
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

	std::shared_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, unsigned int size)
	{
		std::shared_ptr<VertexBuffer> buffer;

		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			buffer = std::make_shared<OpenGL::OpenGLVertexBuffer>(data, size);
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
}