#include <pch.h>

#include "Renderer.h"

namespace ALZ {

	RendererAPI* Renderer::m_CurrentActiveAPI = nullptr;
	Camera* Renderer::m_CurrentSceneCamera = nullptr;
	glm::mat4 Renderer::m_CurrentViewProjection = glm::mat4(1.0f);

	void Renderer::Init()
	{
		m_CurrentActiveAPI = new OpenGL::OpenGLRendererAPI();
	}

	void Renderer::BeginScene(Camera& camera)
	{
		m_CurrentSceneCamera = &camera;
		m_CurrentViewProjection = camera.ProjectionMatrix * camera.ViewMatrix;
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->Draw(shader, mode, vertexCount);

		vertexArray.Unbind();
		shader.Unbind();
	}

	void Renderer::DrawInstanced(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount, const unsigned int& objectCount)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->DrawInstanced(shader, mode, vertexCount, objectCount);

		vertexArray.Unbind();
		shader.Unbind();
	}

	void Renderer::EndScene()
	{
		m_CurrentSceneCamera = nullptr;
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("u_ViewProjection", m_CurrentViewProjection);

		m_CurrentActiveAPI->Draw(shader, primitive, indexBuffer);

		vertexArray.Unbind();
		shader.Unbind();
	}

	void Renderer::ClearScreen()
	{
		m_CurrentActiveAPI->ClearScreen();
	}

	void Renderer::SetViewport(const Viewport viewport)
	{
		m_CurrentActiveAPI->SetViewport(viewport);
	}

	Viewport Renderer::GetCurrentViewport()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return m_CurrentActiveAPI->GetCurrentViewport();

		default:
			assert(false);
			return Viewport();
		}
	}

	std::unique_ptr<VertexArray> Renderer::CreateVertexArray()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGL::OpenGLVertexArray>();

		default:
			assert(false);
			return nullptr;
		}
	}

	std::unique_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, unsigned int size)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGL::OpenGLVertexBuffer>(data, size);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::unique_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unsigned int* data, const int& count)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGL::OpenGLIndexBuffer>(data, count);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::unique_ptr<ShaderProgram> Renderer::CreateShaderProgram(const std::string& vertPath, const std::string& fragPath)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGL::OpenGLShaderProgram>(vertPath, fragPath);

		default:
			assert(false);
			return nullptr;
		}
	}

	std::unique_ptr<FrameBuffer> Renderer::CreateFrameBuffer()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGL::OpenGLFrameBuffer>();

		default:
			assert(false);
			return nullptr;
		}
	}

	std::unique_ptr<Texture> Renderer::CreateTexture()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGL::OpenGLTexture>();

		default:
			assert(false);
			return nullptr;
		}
	}
}