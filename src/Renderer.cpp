#include <pch.h>

#include "Renderer.h"

namespace ALZ {

	Camera* Renderer::m_Camera = nullptr;
	RendererAPI* Renderer::m_CurrentActiveAPI = nullptr;

	void Renderer::Init()
	{
		m_CurrentActiveAPI = new OpenGLRendererAPI();
	}

	void Renderer::BeginScene(Camera& camera)
	{
		m_Camera = &camera;
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("projection", m_Camera->ProjectionMatrix);
		shader.SetUniformMat4("view", m_Camera->ViewMatrix);

		m_CurrentActiveAPI->Draw(shader, mode, vertexCount);

		vertexArray.Unbind();
		shader.Unbind();
	}

	void Renderer::DrawInstanced(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount, const unsigned int& objectCount)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("projection", m_Camera->ProjectionMatrix);
		shader.SetUniformMat4("view", m_Camera->ViewMatrix);

		m_CurrentActiveAPI->DrawInstanced(shader, mode, vertexCount, objectCount);

		vertexArray.Unbind();
		shader.Unbind();
	}

	void Renderer::EndScene()
	{
		m_Camera = nullptr;
	}

	void Renderer::Draw(const VertexArray& vertexArray, ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
	{
		vertexArray.Bind();
		shader.Bind();

		shader.SetUniformMat4("projection", m_Camera->ProjectionMatrix);
		shader.SetUniformMat4("view", m_Camera->ViewMatrix);

		m_CurrentActiveAPI->Draw(shader, primitive, indexBuffer);

		vertexArray.Unbind();
		shader.Unbind();
	}

	void Renderer::ClearScreen()
	{
		m_CurrentActiveAPI->ClearScreen();
	}

	std::unique_ptr<VertexArray> Renderer::CreateVertexArray()
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGLVertexArray>();

		default:
			return std::make_unique<OpenGLVertexArray>();
		}
	}

	std::unique_ptr<VertexBuffer> Renderer::CreateVertexBuffer(void* data, unsigned int size)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGLVertexBuffer>(data, size);

		default:
			return std::make_unique<OpenGLVertexBuffer>(data, size);
		}
	}

	std::unique_ptr<IndexBuffer> Renderer::CreateIndexBuffer(unsigned int* data, const int& count)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGLIndexBuffer>(data, count);

		default:
			return std::make_unique<OpenGLIndexBuffer>(data, count);
		}
	}

	std::unique_ptr<ShaderProgram> Renderer::CreateShaderProgram(const std::string& vertPath, const std::string& fragPath)
	{
		switch (m_CurrentActiveAPI->GetType())
		{
		case RendererType::OpenGL:
			return std::make_unique<OpenGLShaderProgram>(vertPath, fragPath);

		default:
			return std::make_unique<OpenGLShaderProgram>(vertPath, fragPath);
		}
	}
}