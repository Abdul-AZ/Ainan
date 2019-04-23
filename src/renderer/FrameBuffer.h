#pragma once

#include "environment/Window.h"
#include "ShaderProgram.h"

namespace ALZ {

	class FrameBuffer {
	public:
		FrameBuffer();
		~FrameBuffer();

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer operator=(const FrameBuffer&) = delete;

		void Render();
		void Render(ShaderProgram& shader);
		void RenderToScreen();

		void SetSize(const glm::vec2& size);
		glm::vec2& GetSize() { return m_Size; }

		void Bind() const;
		void Unbind() const;

	public:
		unsigned int RendererID;
	
	private:
		unsigned int m_Texture;
		unsigned int m_VertexArray;
		unsigned int m_VertexBuffer;
		glm::vec2 m_Size;
	};
}