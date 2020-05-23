#include <pch.h>

#include "Grid.h"

#define VERTICES_PER_AXIS 75
#define GRID_SCALE        5.0f

namespace Ainan {

	Grid::Grid()
	{
		//reserve vector space for vertices
		std::vector<glm::vec2> vertices;
		std::vector<uint32_t> indecies;
		vertices.reserve(VERTICES_PER_AXIS * 4 + 2);
		indecies.reserve(VERTICES_PER_AXIS * 4 + 2);

		//generate vertices
		//these are the vertices that are on each side

		//bottom line vertices
		for (size_t i = 0; i < VERTICES_PER_AXIS; i++)
		{
			vertices.push_back(glm::vec2(-c_GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * c_GlobalScaleFactor, -c_GlobalScaleFactor) * GRID_SCALE);
		}

		//top line vertices
		for (size_t i = 0; i <= VERTICES_PER_AXIS; i++)
		{
			vertices.push_back(glm::vec2(-c_GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * c_GlobalScaleFactor, c_GlobalScaleFactor) * GRID_SCALE);
		}

		//left line vertices
		for (size_t i = 0; i < VERTICES_PER_AXIS; i++)
		{
			vertices.push_back(glm::vec2(-c_GlobalScaleFactor, -c_GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * c_GlobalScaleFactor) * GRID_SCALE);
		}

		//right line vertices
		for (size_t i = 0; i <= VERTICES_PER_AXIS; i++)
		{
			vertices.push_back(glm::vec2(c_GlobalScaleFactor, -c_GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * c_GlobalScaleFactor) * GRID_SCALE);
		}

		//vertical line indecies
		for (size_t i = 0; i <= VERTICES_PER_AXIS; i++)
		{
			indecies.push_back((unsigned int)i);
			indecies.push_back((unsigned int)i + VERTICES_PER_AXIS);
		}

		//horizontal line indecies
		for (size_t i = 0; i <= VERTICES_PER_AXIS; i++)
		{
			indecies.push_back((unsigned int)i + VERTICES_PER_AXIS * 2 + 1);
			indecies.push_back((unsigned int)i + VERTICES_PER_AXIS * 3 + 1);
		}

		VertexLayout layout(1);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		VBO = Renderer::CreateVertexBuffer(vertices.data(),(unsigned int)sizeof(glm::vec2) * (unsigned int)vertices.size(),
			layout, Renderer::ShaderLibrary["LineShader"]);

		EBO = Renderer::CreateIndexBuffer(indecies.data(), (unsigned int)indecies.size());

		ColorUniformBuffer = Renderer::CreateUniformBuffer("ObjectColor", 1, { {"u_Color", ShaderVariableType::Vec4} }, nullptr);
	}

	void Grid::Draw()
	{
		auto& shader = Renderer::ShaderLibrary["LineShader"];
		shader->Bind();

		shader->BindUniformBuffer("ObjectColor", 1);
		ColorUniformBuffer->Bind(1, RenderingStage::FragmentShader);
		auto color = glm::vec4(1.0f, 1.0f, 1.0f, 0.3f);
		ColorUniformBuffer->UpdateData(&color);

		Renderer::Draw(*VBO, *shader, Primitive::Lines, *EBO);

		shader->Unbind();
	}
}