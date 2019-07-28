#pragma once

#include "renderer/Renderer.h"

namespace ALZ {

	class RenderSurface {
	public:
		RenderSurface();

		RenderSurface(const RenderSurface&) = delete;
		RenderSurface operator=(const RenderSurface&) = delete;

		void Render();
		void Render(ShaderProgram& shader);
		void RenderToScreen();

		void SetSize(const glm::vec2& size);
		glm::vec2 GetSize() { return m_Size; }

	public:
		std::unique_ptr<FrameBuffer> m_FrameBuffer;
	
	private:
		std::unique_ptr<Texture> m_Texture;
		std::unique_ptr<VertexArray> m_VertexArray;
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		glm::vec2 m_Size;
	};
}