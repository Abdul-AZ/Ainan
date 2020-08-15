#include <pch.h>

#include "Grid.h"

namespace Ainan {

	Grid::Grid(float unitLength, int32_t numLinesPerAxis) : 
		m_UnitLength(unitLength)
	{
		//reserve vector space for vertices
		std::vector<glm::vec2> vertices;
		std::vector<uint32_t> indecies;

		vertices.reserve((size_t)numLinesPerAxis * 4);
		indecies.reserve((size_t)numLinesPerAxis * 4);

		glm::vec2 bottomLeftVertex = { -(numLinesPerAxis - 1) * unitLength / 2.0f, -(numLinesPerAxis - 1) * unitLength / 2.0f };

		//generate vertices
		for (size_t i = 0; i < numLinesPerAxis; i++) //bottom
			vertices.push_back({ bottomLeftVertex.x + i * unitLength, bottomLeftVertex.y });

		for (size_t i = 0; i < numLinesPerAxis; i++) //top
			vertices.push_back({ bottomLeftVertex.x + i * unitLength, -bottomLeftVertex.y });

		for (size_t i = 0; i < numLinesPerAxis; i++) //left
			vertices.push_back({ bottomLeftVertex.x, bottomLeftVertex.y + i * unitLength });

		for (size_t i = 0; i < numLinesPerAxis; i++) //right
			vertices.push_back({ -bottomLeftVertex.x, bottomLeftVertex.y + i * unitLength });

		//generate indecies
		for (size_t i = 0; i < numLinesPerAxis; i++) //bottom to top lines
		{
			indecies.push_back(i);
			indecies.push_back((uint32_t)i + numLinesPerAxis);
		}

		for (size_t i = 0; i < numLinesPerAxis; i++) //left to right lines
		{
			indecies.push_back((uint32_t)numLinesPerAxis * 2 + i);
			indecies.push_back((uint32_t)numLinesPerAxis * 2 + i + numLinesPerAxis);
		}

		m_VertexBuffer = Renderer::CreateVertexBuffer(vertices.data(),
			(uint32_t)sizeof(glm::vec2) * vertices.size(),
			{ { "aPos", ShaderVariableType::Vec2 } },
			Renderer::ShaderLibrary()["LineShader"]);

		m_IndexBuffer = Renderer::CreateIndexBuffer(indecies.data(), (uint32_t)indecies.size());

		m_TransformUniformBuffer = Renderer::CreateUniformBuffer("ObjectTransform",
			1,
			{ {"u_Model", ShaderVariableType::Mat4} },
			nullptr);
	}

	void Grid::Draw(const Camera& camera)
	{
		auto& shader = Renderer::ShaderLibrary()["GridShader"];

		glm::vec2 pos = glm::vec2(std::round(camera.Position.x / m_UnitLength), std::round(camera.Position.y / m_UnitLength)) * m_UnitLength;

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3{ -pos, 0.0f });
		m_TransformUniformBuffer->UpdateData(&model);
		Renderer::WaitUntilRendererIdle();

		shader->BindUniformBuffer(m_TransformUniformBuffer, 1, RenderingStage::VertexShader);
		Renderer::Draw(*m_VertexBuffer, *shader, Primitive::Lines, *m_IndexBuffer);
	}
}