#include <pch.h>

#include "Line.h"

namespace ALZ {

	static bool LineBufferInitilized = false;
	static VertexArray* VAO = nullptr;
	static VertexBuffer* VBO = nullptr;

	Line::Line()
	{
		if (!LineBufferInitilized) {

			VAO = Renderer::CreateVertexArray().release();
			VAO->Bind();

			VBO = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec2) * 2).release();
			VBO->SetLayout({ ShaderVariableType::Vec2 });

			VAO->Unbind();

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

		UpdateBufferWithPoints();
	}

	void Line::Render(Camera& camera)
	{
		glLineWidth(Width);
		ShaderProgram& LineShader = ShaderProgram::GetLineShader();
		LineShader.SetUniformVec4("color", Color);
		LineShader.SetUniformMat4("projection", camera.ProjectionMatrix);
		LineShader.SetUniformMat4("view", camera.ViewMatrix);

		LineShader.Bind();
		VAO->Bind();

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

		VBO->UpdateData(0, sizeof(vertices), vertices);
	}
}