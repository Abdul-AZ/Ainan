#pragma once

#include "ShaderProgram.h"

class Line 
{
public:
	Line();

	void SetPoints(glm::vec2 startPoint, glm::vec2 endPoint);
	void Render();
	void SetColor(const glm::vec4& color) { m_Color = color; };
	void SetWidth(const float& width) { m_Width = width; };
private:
	void UpdateBufferWithPoints();

	unsigned int m_VertexBuffer;
	unsigned int m_VertexArrayBuffer;
	float m_Width = 2.0f;
	glm::vec4 m_Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glm::vec2 m_StartPoint;
	glm::vec2 m_EndPoint;
};