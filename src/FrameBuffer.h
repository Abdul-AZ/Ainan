#pragma once

#include "Window.h"
#include "ShaderProgram.h"

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
	unsigned int m_RendererID;
private:
	unsigned int texture;
	unsigned int vertexArray;
	unsigned int vertexBuffer;
	ShaderProgram imageShader;
	glm::vec2 m_Size;
};
