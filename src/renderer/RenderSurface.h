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
		glm::vec2 GetSize() { return m_Size; }

	public:
		std::shared_ptr<FrameBuffer> SurfaceFrameBuffer;
	
		std::shared_ptr<Texture> m_Texture;
	private:
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		glm::vec2 m_Size;
	};
}