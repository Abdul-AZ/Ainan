#pragma once

#include "Window.h"
#include <glad/glad.h>

#include "ShaderProgram.h"

class FrameBuffer {
public:
	FrameBuffer();
	~FrameBuffer();

	FrameBuffer(const FrameBuffer&) = delete;
	FrameBuffer operator=(const FrameBuffer&) = delete;

	void Render();
	void Render(ShaderProgram& shader);

	void SetSize(const glm::vec2& size);

	void Bind() const;
	void Unbind() const;
	unsigned int m_RendererID;
private:
	unsigned int texture;
	unsigned int vertexArray;
	unsigned int vertexBuffer;
	ShaderProgram imageShader;
};
