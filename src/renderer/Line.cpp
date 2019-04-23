#include <pch.h>

#include "Line.h"

namespace ALZ {

	Line::Line()
	{
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

	void Line::Render(Camera& camera)
	{
		glLineWidth(m_Width);
		ShaderProgram& LineShader = ShaderProgram::GetLineShader();
		LineShader.SetUniformVec4("color", m_Color);
		LineShader.SetUniformMat4("projection", camera.GetProjectionMatrix());
		LineShader.SetUniformMat4("view", camera.GetViewMatrix());
		LineShader.SetUniformMat4("model", camera.GetViewMatrix());

		LineShader.Bind();
		glBindVertexArray(m_VertexArrayBuffer);

		glDrawArrays(GL_LINES, 0, 2);

		glBindVertexArray(0);
		LineShader.Unbind();
	}

	float Line::GetSlope()
	{
		float deltaY = m_EndPoint.y - m_StartPoint.y;
		float deltaX = m_EndPoint.x - m_StartPoint.x;
		return deltaY / deltaX;
	}

	float Line::GetYIntercept()
	{
		float slope = GetSlope();

		return m_EndPoint.y - m_EndPoint.x * slope;
	}

	glm::vec2 Line::GetPointInLine(const float & t)
	{
		float slope = GetSlope();
		float yIntercept = GetYIntercept();

		float x = m_StartPoint.x + t * (m_EndPoint.x - m_StartPoint.x);
		float y = slope * x + yIntercept;

		return glm::vec2(x, y);
	}

	void Line::UpdateBufferWithPoints()
	{
		glm::vec2 vertices[] = { m_StartPoint, m_EndPoint };

		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * 2, vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}
}