#pragma once

#include "renderer/Renderer.h"

namespace Ainan {

	class RenderSurface {
	public:
		RenderSurface();

		RenderSurface(const RenderSurface&) = delete;
		RenderSurface operator=(const RenderSurface&) = delete;

		void Render();
		void Render(ShaderProgram& shader);
		void RenderToScreen(const Rectangle& viewport);

		void SetSize(const glm::vec2& size);
	public:
		std::shared_ptr<FrameBuffer> SurfaceFrameBuffer;
	
	private:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
	};
}