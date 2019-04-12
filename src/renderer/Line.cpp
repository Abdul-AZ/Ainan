#include <pch.h>

#include "Line.h"

static ShaderProgram LineShader;
static bool ShaderInitilized = false;

Line::Line()
{
	if (!ShaderInitilized) {
		LineShader.Init("shaders/Line.vert", "shaders/Line.frag");
		ShaderInitilized = true;
	}

	glGenVertexArrays(1, &m_VertexArrayBuffer);
	glBindVertexArray(m_VertexArrayBuffer);
	glGenBuffers(1, &m_VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, 0, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}

void Line::SetPoints(glm::vec2 startPoint, glm::vec2 endPoint)
{
	m_StartPoint = startPoint;
	m_EndPoint = endPoint;

	UpdateBufferWithPoints();
}

void Line::Render()
{
	glLineWidth(m_Width);
	LineShader.setUniformVec4("color", m_Color);

	LineShader.Bind();
	glBindVertexArray(m_VertexArrayBuffer);

	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
	LineShader.Unbind();
}

void Line::UpdateBufferWithPoints()
{
	glm::vec2 vertices[] = { m_StartPoint, m_EndPoint };

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * 2, vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
