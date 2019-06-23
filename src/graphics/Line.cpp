#include <pch.h>

#include "Line.h"

namespace ALZ {

	static bool LineBufferInitilized = false;
	static unsigned int VBO = 0;
	static unsigned int VAO = 0;

	Line::Line()
	{
		if (!LineBufferInitilized) {

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, 0, GL_DYNAMIC_DRAW);

			glBindVertexArray(0);

			LineBufferInitilized = true;
		}
	}

	void Line::SetPoints(glm::vec2 startPoint, glm::vec2 endPoint)
	{
		m_StartPoint = startPoint * GlobalScaleFactor;
		m_EndPoint = endPoint * GlobalScaleFactor;

		UpdateBufferWithPoints();
	}

	void Line::SetPoints(glm::vec2 linePosition, float lineLength, float lineAngle)
	{
		glm::vec2 pointDispositionFromCenter = lineLength * glm::vec2(cos(lineAngle * 3.14159265 / 180.0f), sin(lineAngle * 3.14159265 / 180.0f));

		m_StartPoint = linePosition + pointDispositionFromCenter;
		m_EndPoint = linePosition - pointDispositionFromCenter;

		m_StartPoint *= GlobalScaleFactor;
		m_EndPoint *= GlobalScaleFactor;
	}

	void Line::Render(Camera& camera)
	{
		glLineWidth(Width);
		ShaderProgram& LineShader = ShaderProgram::GetLineShader();
		LineShader.SetUniformVec4("color", Color);
		LineShader.SetUniformMat4("projection", camera.ProjectionMatrix);
		LineShader.SetUniformMat4("view", camera.ViewMatrix);

		LineShader.Bind();
		glBindVertexArray(VAO);

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

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * 2, vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}