#pragma once

#include "Renderer.h"

namespace ALZ {

	class RenderSurface {
	public:
		RenderSurface();
		~RenderSurface();

		RenderSurface(const RenderSurface&) = delete;
		RenderSurface operator=(const RenderSurface&) = delete;

		void Render();
		void Render(ShaderProgram& shader);
		void RenderToScreen();

		void SetSize(const glm::vec2& size);
		glm::vec2& GetSize() { return m_Size; }

	public:
		std::unique_ptr<FrameBuffer> m_FrameBuffer;
	
	private:
		unsigned int m_Texture;
		unsigned int m_VertexArray;
		unsigned int m_VertexBuffer;
		glm::vec2 m_Size;
	};
}