#include <pch.h>

#include "Renderer.h"

namespace Ainan {

	RendererAPI* Renderer::m_CurrentActiveAPI = nullptr;
	Camera* Renderer::m_CurrentSceneCamera = nullptr;
	glm::mat4 Renderer::m_CurrentViewProjection = glm::mat4(1.0f);
	unsigned int Renderer::NumberOfDrawCallsLastScene = 0;
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> Renderer::ShaderLibrary;

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
		{ "ImageShader"         , "shaders/Image.vert"         , "shaders/Image.frag"          }
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
	}

	void Renderer::Terminate()
	{
		ShaderLibrary.erase(ShaderLibrary.begin(), ShaderLibrary.end());
	}

	void Renderer::BeginScene(Camera& camera)
	{
		m_CurrentSceneCamera = &camera;
		m_CurrentViewProjection = camera.ProjectionMatrix * camera.ViewMatrix;
		s_CurrentNumberOfDrawCalls = 0;
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

	void Renderer::SetViewport(const Rectangle viewport)
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
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLVertexBuffer>(data, size);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::shared_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unsigned int* data, const int& count)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLIndexBuffer>(data, count);

		default:
			assert(false);
			return nullptr;
		}
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
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_shared<OpenGL::OpenGLTexture>();

		default:
			assert(false);
			return nullptr;
		}
	}
}