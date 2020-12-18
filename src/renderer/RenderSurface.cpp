#include "RenderSurface.h"

namespace Ainan {

	RenderSurface::RenderSurface()
	{
		SurfaceFrameBuffer = Renderer::CreateFrameBufferNew(Window::FramebufferSize);

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
		m_VertexBuffer = Renderer::CreateVertexBufferNew(quadVertices, sizeof(quadVertices), layout, Renderer::ShaderLibraryNew()["ImageShader"]);

		Renderer::SetRenderTargetApplicationWindow();
	}

	void RenderSurface::Render()
	{
		auto& shader = Renderer::ShaderLibraryNew()["ImageShader"];
		shader.BindTexture(SurfaceFrameBuffer, 0, RenderingStage::FragmentShader);
		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}

	void RenderSurface::Render(ShaderProgramNew shader)
	{
		shader.BindTexture(SurfaceFrameBuffer, 0, RenderingStage::FragmentShader);
		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}

	void RenderSurface::RenderToScreen(const Rectangle& viewport)
	{
		//nullptr means we are copying to the default render buffer (which is the one being displayed)
		SurfaceFrameBuffer.Blit(nullptr, SurfaceFrameBuffer.GetSize(), SurfaceFrameBuffer.GetSize());

		Rectangle screenViewport;
		screenViewport.X = 0;
		screenViewport.Y = 0;
		screenViewport.Width  = (int)Window::FramebufferSize.x;
		screenViewport.Height = (int)Window::FramebufferSize.y;

		Renderer::SetViewport(viewport);
	}

	void RenderSurface::SetSize(const glm::vec2& size)
	{
		SurfaceFrameBuffer.Resize(size);
	}
}