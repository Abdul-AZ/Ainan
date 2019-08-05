#include <pch.h>

#include "Grid.h"

#define VERTICES_PER_AXIS 75
#define GRID_SCALE        5.0f

namespace ALZ {

	static bool GridBufferInitilized = false;
	static VertexArray* VAO = nullptr;
	static VertexBuffer* VBO = nullptr;
	static IndexBuffer* EBO = nullptr;
	static ShaderProgram* LineShader = nullptr;

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
				vertices.push_back(glm::vec2(-GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * GlobalScaleFactor, -GlobalScaleFactor) * GRID_SCALE);
			}

			//top line vertices
			for (size_t i = 0; i <= VERTICES_PER_AXIS; i++)
			{
				vertices.push_back(glm::vec2(-GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * GlobalScaleFactor, GlobalScaleFactor) * GRID_SCALE);
			}

			//left line vertices
			for (size_t i = 0; i < VERTICES_PER_AXIS; i++)
			{
				vertices.push_back(glm::vec2(-GlobalScaleFactor, -GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * GlobalScaleFactor) * GRID_SCALE);
			}

			//right line vertices
			for (size_t i = 0; i <= VERTICES_PER_AXIS; i++)
			{
				vertices.push_back(glm::vec2(GlobalScaleFactor, -GlobalScaleFactor + i * (1.0f / VERTICES_PER_AXIS) * 2 * GlobalScaleFactor) * GRID_SCALE);
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

			VAO = Renderer::CreateVertexArray().release();
			VAO->Bind();

			VBO = Renderer::CreateVertexBuffer(vertices.data(),(unsigned int)sizeof(glm::vec2) * (unsigned int)vertices.size()).release();
			VBO->SetLayout({ ShaderVariableType::Vec2 });

			EBO = Renderer::CreateIndexBuffer(indecies.data(), (unsigned int)indecies.size()).release();

			VAO->Unbind();

			LineShader = Renderer::CreateShaderProgram("shaders/Line.vert", "shaders/FlatColor.frag").release();

			GridBufferInitilized = true;
		}
	}

	void Grid::Draw()
	{
		VAO->Bind();
		LineShader->Bind();

		LineShader->SetUniformVec4("u_Color", glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));

		Renderer::Draw(*VAO, *LineShader, Primitive::Lines, *EBO);

		VAO->Unbind();
		LineShader->Unbind();
	}
}