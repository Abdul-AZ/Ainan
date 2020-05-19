#include <pch.h>

#include "Grid.h"

#define VERTICES_PER_AXIS 75
#define GRID_SCALE        5.0f

namespace Ainan {

	static bool GridBufferInitilized = false;
	static std::shared_ptr<VertexArray> VAO = nullptr;
	static std::shared_ptr<VertexBuffer> VBO = nullptr;
	static std::shared_ptr<IndexBuffer> EBO = nullptr;

	static std::vector<glm::vec2> vertices;
	static std::vector<unsigned int> indecies;

	Grid::Grid()
	{
		if (!GridBufferInitilized)
		{
			//reserve vector space for vertices
			vertices.reserve(VERTICES_PER_AXIS * 4 + 2);
			indecies.reserve(VERTICES_PER_AXIS * 4 + 4);

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

			VAO = Renderer::CreateVertexArray();
			VAO->Bind();

			VBO = Renderer::CreateVertexBuffer(vertices.data(),(unsigned int)sizeof(glm::vec2) * (unsigned int)vertices.size());
			VertexLayout layout(1);
			layout[0] = { "aPos", ShaderVariableType::Vec2 };
			VBO->SetLayout(layout, Renderer::ShaderLibrary["LineShader"]);

			EBO = Renderer::CreateIndexBuffer(indecies.data(), (unsigned int)indecies.size());

			VAO->Unbind();

			GridBufferInitilized = true;
		}
	}

	void Grid::Draw()
	{
		VAO->Bind();

		auto& shader = Renderer::ShaderLibrary["LineShader"];
		shader->Bind();

		shader->SetUniformVec4("u_Color", glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));

		Renderer::Draw(*VAO, *shader, Primitive::Lines, *EBO);

		VAO->Unbind();
		shader->Unbind();
	}
}