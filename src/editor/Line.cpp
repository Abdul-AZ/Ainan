#include <pch.h>

#include "Line.h"

namespace Ainan {

	Line::Line()
	{
		VAO = Renderer::CreateVertexArray();
		VAO->Bind();

		VertexLayout layout(1);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		VBO = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec2) * 2, layout, Renderer::ShaderLibrary["LineShader"]);

		VAO->Unbind();
	}

	void Line::SetVertices(const std::vector<glm::vec2> vertices)
	{
		//resize VBO if the number of new vertices are bigger
		if (vertices.size() > m_Vertices.size()) 
		{
			VBO.reset();

			VAO->Bind();

			VertexLayout layout(1);
			layout[0] = { "aPos", ShaderVariableType::Vec2 };
			VBO = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec2) * vertices.size(), layout, Renderer::ShaderLibrary["LineShader"]);

			VAO->Unbind();
		}
		m_Vertices = vertices;

		//transform to world space
		for (auto& vertex : m_Vertices)
			vertex *= c_GlobalScaleFactor;
	}

	void Line::Draw()
	{
		VBO->UpdateData(0, sizeof(glm::vec2) * m_Vertices.size(), m_Vertices.data());

		auto& shader = Renderer::ShaderLibrary["LineShader"];

		shader->SetUniformVec4("u_Color", Color);
		shader->Bind();
		VAO->Bind();

		Renderer::Draw(*VAO, *shader, Primitive::Lines, m_Vertices.size());

		VAO->Unbind();
		shader->Unbind();
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