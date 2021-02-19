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
			{ VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2) },
			Renderer::ShaderLibrary()["LineShader"]);

		m_IndexBuffer = Renderer::CreateIndexBuffer(indecies.data(), (uint32_t)indecies.size());

		m_TransformUniformBuffer = Renderer::CreateUniformBuffer("ObjectTransform",
			1,
			{ VertexLayoutElement("u_Model", 0, ShaderVariableType::Mat4) }
		);
	}

	Grid::~Grid()
	{
		Renderer::DestroyVertexBuffer(m_VertexBuffer);
		Renderer::DestroyIndexBuffer(m_IndexBuffer);
		Renderer::DestroyUniformBuffer(m_TransformUniformBuffer);
	}

	void Grid::Draw(const Camera& camera)
	{
		auto& shader = Renderer::ShaderLibrary()["GridShader"];

		glm::mat4 model(1.0f);
		glm::vec3 pos(0.0f);

		switch (Orientation)
		{
		case XY:
			pos = glm::vec3(std::round(camera.Position.x / m_UnitLength), std::round(camera.Position.y / m_UnitLength), 0.0f) * m_UnitLength;
			model = glm::translate(model, pos);
			break;

		case XZ:
			pos = glm::vec3(std::round(camera.Position.x / m_UnitLength), 0.0f, std::round(camera.Position.z / m_UnitLength)) * m_UnitLength;
			model = glm::translate(model, pos);
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			break;

		case YZ:
			pos = glm::vec3(0.0f, std::round(camera.Position.y / m_UnitLength), std::round(camera.Position.z / m_UnitLength)) * m_UnitLength;
			model = glm::translate(model, pos);
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		}
		
		m_TransformUniformBuffer.UpdateData(&model, sizeof(glm::mat4));

		shader.BindUniformBuffer(m_TransformUniformBuffer, 1, RenderingStage::VertexShader);
		Renderer::Draw(m_VertexBuffer, shader, Primitive::Lines, m_IndexBuffer);
	}
}