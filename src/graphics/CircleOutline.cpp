#include <pch.h>

#include "CircleOutline.h"

namespace ALZ {

	static bool CircleOutlineBuffersInitilized = false;
	static unsigned int VBO = 0;
	static unsigned int EBO = 0;
	static VertexArray* VAO = 0;

	static const int vertexCount = 60;
	CircleOutline::CircleOutline()
	{
		if (!CircleOutlineBuffersInitilized){

			VAO = Renderer::CreateVertexArray().release();
			VAO->Bind();

			glm::vec2 vertices[vertexCount];
			unsigned int indecies[vertexCount * 2 - 2];

			float degreesBetweenVertices = 360 / vertexCount;

			for (int i = 0; i < vertexCount; i++)
			{
				float angle = i * degreesBetweenVertices;
				vertices[i].x = (float)cos(angle * (PI / 180.0));
				vertices[i].y = (float)sin(angle * (PI / 180.0));

				if (i == vertexCount - 1)
					continue;

				indecies[i * 2] = i;
				indecies[i * 2 + 1] = i + 1;
			}
			vertices[vertexCount - 1] = vertices[0];

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

			glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertexCount, vertices, GL_STATIC_DRAW);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * vertexCount * 2 - 2, indecies, GL_STATIC_DRAW);

			glBindVertexArray(0);

			CircleOutlineBuffersInitilized = true;
		}
	}

	void CircleOutline::Render(Camera& camera)
	{
		ShaderProgram& CircleOutlineShader = ShaderProgram::GetCircleOutlineShader();

		glLineWidth(3.0f);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(Position.x * GlobalScaleFactor, Position.y * GlobalScaleFactor, 0.0f));
		model = glm::scale(model, glm::vec3(Radius * GlobalScaleFactor, Radius * GlobalScaleFactor, Radius * GlobalScaleFactor));

		CircleOutlineShader.Bind();
		CircleOutlineShader.SetUniformMat4("model", model);
		CircleOutlineShader.SetUniformMat4("view", camera.ViewMatrix);
		CircleOutlineShader.SetUniformMat4("projection", camera.ProjectionMatrix);
		CircleOutlineShader.SetUniformVec4("color", Color);
		VAO->Bind();

		glDrawElements(GL_LINES, vertexCount * 2 - 2, GL_UNSIGNED_INT, nullptr);

		VAO->Unbind();
		CircleOutlineShader.Unbind();
	}

	glm::vec2 CircleOutline::GetPointByAngle(const float & angle_in_radians)
	{
		float x = Position.x  * GlobalScaleFactor + Radius * cos(angle_in_radians) * GlobalScaleFactor;
		float y = Position.y  * GlobalScaleFactor + Radius * sin(angle_in_radians) * GlobalScaleFactor;

		return glm::vec2(x, y);
	}
}