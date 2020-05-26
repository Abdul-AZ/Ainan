#include <pch.h>

#include "RenderSurface.h"

namespace Ainan {

	RenderSurface::RenderSurface()
	{
		SurfaceFrameBuffer = Renderer::CreateFrameBuffer();

		SurfaceFrameBuffer->Bind();

		m_Texture = Renderer::CreateTexture(Window::FramebufferSize);

		//m_Size = Window::FramebufferSize;
		//
		//m_Texture->SetImage(m_Size, 3);
		//
		m_Texture->SetDefaultTextureSettings();
		//m_Texture->Unbind();

		SurfaceFrameBuffer->SetActiveTexture(*m_Texture);

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

		VertexLayout layout(2);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		layout[1] = { "aTexCoords", ShaderVariableType::Vec2 };
		m_VertexBuffer = Renderer::CreateVertexBuffer(quadVertices, sizeof(quadVertices), layout, Renderer::ShaderLibrary["ImageShader"]);

		SurfaceFrameBuffer->Unbind();
	}

	void RenderSurface::Render()
	{
		auto& shader = Renderer::ShaderLibrary["ImageShader"];
		shader->SetUniform1i("u_ScreenTexture", 0);
		m_Texture->Bind();
		Renderer::Draw(*m_VertexBuffer, *shader, Primitive::Triangles, 6);
	}

	void RenderSurface::Render(ShaderProgram& shader)
	{
		m_Texture->Bind();
		Renderer::Draw(*m_VertexBuffer, shader, Primitive::Triangles, 6);
	}

	void RenderSurface::RenderToScreen(const Rectangle& viewport)
	{
		//nullptr means we are copying to the default render buffer (which is the one being displayed)
		SurfaceFrameBuffer->Blit(nullptr, m_Size, m_Size);

		Rectangle screenViewport;
		screenViewport.X = 0;
		screenViewport.Y = 0;
		screenViewport.Width  = (int)Window::FramebufferSize.x;
		screenViewport.Height = (int)Window::FramebufferSize.y;

		Renderer::SetViewport(viewport);
	}

	void RenderSurface::SetSize(const glm::vec2 & size)
	{
		m_Size = size;
		SurfaceFrameBuffer->Bind();
		
		//delete old trexture
		m_Texture.reset();
	
		//create a new one with the window size
		m_Texture = Renderer::CreateTexture(size);
		//m_Texture->SetImage(size, 3);
	
		m_Texture->SetDefaultTextureSettings();
		m_Texture->Unbind();
	
		SurfaceFrameBuffer->SetActiveTexture(*m_Texture);
		SurfaceFrameBuffer->Unbind();
	}
}