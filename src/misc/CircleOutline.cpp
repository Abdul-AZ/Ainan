#include <pch.h>

#include "CircleOutline.h"

namespace ALZ {

	static bool CircleOutlineBuffersInitilized = false;
	static IndexBuffer* EBO = nullptr;
	static VertexBuffer* VBO = nullptr;
	static VertexArray* VAO = nullptr;
	static ShaderProgram* CircleOutlineShader = nullptr;

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

			VBO = Renderer::CreateVertexBuffer(vertices, sizeof(glm::vec2) * vertexCount).release();
			VBO->SetLayout({ ShaderVariableType::Vec2 });

			EBO = Renderer::CreateIndexBuffer(indecies, vertexCount * 2 - 2).release();

			VAO->Unbind();

			CircleOutlineShader = Renderer::CreateShaderProgram("shaders/CircleOutline.vert", "shaders/CircleOutline.frag").release();

			CircleOutlineBuffersInitilized = true;
		}
	}

	void CircleOutline::Draw()
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(Position.x * GlobalScaleFactor, Position.y * GlobalScaleFactor, 0.0f));
		model = glm::scale(model, glm::vec3(Radius * GlobalScaleFactor, Radius * GlobalScaleFactor, Radius * GlobalScaleFactor));

		CircleOutlineShader->Bind();
		CircleOutlineShader->SetUniformMat4("model", model);
		CircleOutlineShader->SetUniformVec4("color", Color);
		VAO->Bind();

		Renderer::Draw(*VAO, *CircleOutlineShader, Primitive::Lines, *EBO);

		VAO->Unbind();
		CircleOutlineShader->Unbind();
	}

	glm::vec2 CircleOutline::GetPointByAngle(const float & angle_in_radians)
	{
		float x = Position.x  * GlobalScaleFactor + Radius * cos(angle_in_radians) * GlobalScaleFactor;
		float y = Position.y  * GlobalScaleFactor + Radius * sin(angle_in_radians) * GlobalScaleFactor;

		return glm::vec2(x, y);
	}
}