#include "RenderSurface.h"

namespace Ainan {

	RenderSurface::RenderSurface()
	{
		SurfaceFramebuffer = Renderer::CreateFramebuffer(Window::FramebufferSize);

		float quadVertices[] = { 
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		VertexLayout layout(2);
		layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
		layout[1] = VertexLayoutElement("NORMAL", 0, ShaderVariableType::Vec2);
		m_VertexBuffer = Renderer::CreateVertexBuffer(quadVertices, sizeof(quadVertices), layout, Renderer::ShaderLibrary()["ImageShader"]);

		Renderer::SetRenderTargetApplicationWindow();
	}

	RenderSurface::~RenderSurface()
	{
		Renderer::DestroyVertexBuffer(m_VertexBuffer);
		Renderer::DestroyFramebuffer(SurfaceFramebuffer);
	}

	void RenderSurface::Render()
	{
		auto& shader = Renderer::ShaderLibrary()["ImageShader"];
		shader.BindTexture(SurfaceFramebuffer, 0, RenderingStage::FragmentShader);
		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}

	void RenderSurface::Render(ShaderProgram shader)
	{
		shader.BindTexture(SurfaceFramebuffer, 0, RenderingStage::FragmentShader);
		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}

	void RenderSurface::SetSize(const glm::vec2& size)
	{
		if (size.x == 0 || size.y == 0)
			return;

		if(size != SurfaceFramebuffer.GetSize())
			SurfaceFramebuffer.Resize(size);
	}
}