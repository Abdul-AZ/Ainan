#pragma once

#include "renderer/Renderer.h"

namespace Ainan {

	class RenderSurface {
	public:
		RenderSurface();
		~RenderSurface();

		RenderSurface(const RenderSurface&) = delete;
		RenderSurface operator=(const RenderSurface&) = delete;

		void Render();
		void Render(ShaderProgram shader);

		void SetSize(const glm::vec2& size);
	public:
		Framebuffer SurfaceFramebuffer;
	
	private:
		VertexBuffer m_VertexBuffer;
	};
}