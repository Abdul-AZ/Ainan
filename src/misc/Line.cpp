#include <pch.h>

#include "Line.h"

namespace ALZ {

	Line::Line()
	{
		VAO = Renderer::CreateVertexArray();
		VAO->Bind();

		VBO = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec2) * 2);
		VBO->SetLayout({ ShaderVariableType::Vec2 });

		VAO->Unbind();
		LineShader = Renderer::CreateShaderProgram("shaders/Line.vert", "shaders/FlatColor.frag");
	}

	void Line::SetVertices(const std::vector<glm::vec2> vertices)
	{
		//resize VBO if the number of new vertices are bigger
		if (vertices.size() > m_Vertices.size()) 
		{
			VBO.reset();

			VAO->Bind();

			VBO = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec2) * vertices.size());
			VBO->SetLayout({ ShaderVariableType::Vec2 });

			VAO->Unbind();
		}
		m_Vertices = vertices;

		//transform to world space
		for (auto& vertex : m_Vertices)
			vertex *= GlobalScaleFactor;
	}

	void Line::Draw()
	{
		VBO->UpdateData(0, sizeof(glm::vec2) * m_Vertices.size(), m_Vertices.data());
		LineShader->SetUniformVec4("u_Color", Color);
		LineShader->Bind();
		VAO->Bind();

		Renderer::Draw(*VAO, *LineShader, Primitive::Lines, m_Vertices.size());

		VAO->Unbind();
		LineShader->Unbind();
	}

	float Line::GetSlope(size_t startVertex, size_t endVertex)
	{
		float deltaY = m_Vertices[endVertex].y - m_Vertices[startVertex].y;
		float deltaX = m_Vertices[endVertex].x - m_Vertices[startVertex].x;
		return deltaY / deltaX;
	}

	float Line::GetYIntercept(size_t startVertex, size_t endVertex)
	{
		float slope = GetSlope();

		return m_Vertices[endVertex].y - m_Vertices[endVertex].x * slope;
	}

	glm::vec2 Line::GetPointInLine(const float t, size_t startVertex, size_t endVertex)
	{
		float slope = GetSlope();
		float yIntercept = GetYIntercept();

		float x = m_Vertices[startVertex].x + t * (m_Vertices[endVertex].x - m_Vertices[startVertex].x);
		float y = slope * x + yIntercept;

		return glm::vec2(x, y);
	}
}