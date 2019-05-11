#include <pch.h>

#include "Grid.h"

#define VERTICES_PER_AXIS 75
#define GRID_SCALE        5.0f

namespace ALZ {

	static bool GridBufferInitilized = false;
	static unsigned int VAO = 0;
	static unsigned int VBO = 0;
	static unsigned int EBO = 0;

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

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indecies.size(), indecies.data(), GL_DYNAMIC_DRAW);

			glBindVertexArray(0);

			GridBufferInitilized = true;
		}
	}

	void Grid::Render(Camera& camera)
	{
		glBindVertexArray(VAO);
		ShaderProgram& lineShader = ShaderProgram::GetLineShader();
		lineShader.Bind();

		lineShader.SetUniformMat4("projection", camera.ProjectionMatrix);
		lineShader.SetUniformMat4("view", camera.ViewMatrix);
		lineShader.SetUniformVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));

		glDrawElements(GL_LINES, 400 + 2, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);
		lineShader.Unbind();
	}
}