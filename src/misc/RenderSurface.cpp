#include <pch.h>

#include "RenderSurface.h"

namespace ALZ {

	RenderSurface::RenderSurface()
	{
		SurfaceFrameBuffer = Renderer::CreateFrameBuffer();

		SurfaceFrameBuffer->Bind();

		m_Texture = Renderer::CreateTexture();

		m_Size = Window::FramebufferSize;
		
		m_Texture->SetImage(m_Size, 4);

		m_Texture->SetDefaultTextureSettings();
		m_Texture->Unbind();

		SurfaceFrameBuffer->SetActiveTexture(*m_Texture);

		m_VertexArray = Renderer::CreateVertexArray();
		m_VertexArray->Bind();

		// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		float quadVertices[] = { 
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		m_VertexBuffer = Renderer::CreateVertexBuffer(quadVertices, sizeof(quadVertices));
		m_VertexBuffer->SetLayout({ ShaderVariableType::Vec2, ShaderVariableType::Vec2 });

		SurfaceFrameBuffer->Unbind();
	}

	void RenderSurface::Render()
	{
		auto& shader = Renderer::ShaderLibrary["ImageShader"];
		shader->SetUniform1i("u_ScreenTexture", 0);
		m_Texture->Bind();
		Renderer::Draw(*m_VertexArray, *shader, Primitive::Triangles, 6);
	}

	void RenderSurface::Render(ShaderProgram& shader)
	{
		m_Texture->Bind();
		Renderer::Draw(*m_VertexArray, shader, Primitive::Triangles, 6);
	}

	void RenderSurface::RenderToScreen(const Viewport& viewport)
	{
		//nullptr means we are copying to the default render buffer (which is the one being displayed)
		SurfaceFrameBuffer->Blit(nullptr, m_Size, m_Size);

		Viewport screenViewport;
		screenViewport.x = 0;
		screenViewport.y = 0;
		screenViewport.width  = (int)Window::FramebufferSize.x;
		screenViewport.height = (int)Window::FramebufferSize.y;

		Renderer::SetViewport(viewport);
	}

	void RenderSurface::SetSize(const glm::vec2 & size)
	{
		m_Size = size;
		SurfaceFrameBuffer->Bind();
		
		//delete old trexture
		m_Texture.reset();
	
		//create a new one with the window size
		m_Texture = Renderer::CreateTexture();
		m_Texture->SetImage(size, 4);
	
		m_Texture->SetDefaultTextureSettings();
		m_Texture->Unbind();
	
		SurfaceFrameBuffer->SetActiveTexture(*m_Texture);
		SurfaceFrameBuffer->Unbind();
	}
}