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
		void Render(ShaderProgramNew shader);

		void SetSize(const glm::vec2& size);
	public:
		FrameBufferNew SurfaceFrameBuffer;
	
	private:
		VertexBufferNew m_VertexBuffer;
	};
}