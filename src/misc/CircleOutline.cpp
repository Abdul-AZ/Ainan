#include <pch.h>

#include "CircleOutline.h"

namespace ALZ {

	static bool CircleOutlineBuffersInitilized = false;
	static std::shared_ptr<IndexBuffer> EBO = nullptr;
	static std::shared_ptr<VertexBuffer> VBO = nullptr;
	static std::shared_ptr<VertexArray> VAO = nullptr;
	static std::shared_ptr<ShaderProgram> CircleOutlineShader = nullptr;

	static const int vertexCount = 60;
	CircleOutline::CircleOutline()
	{
		if (!CircleOutlineBuffersInitilized){

			VAO = Renderer::CreateVertexArray();
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

			VBO = Renderer::CreateVertexBuffer(vertices, sizeof(glm::vec2) * vertexCount);
			VBO->SetLayout({ ShaderVariableType::Vec2 });

			EBO = Renderer::CreateIndexBuffer(indecies, vertexCount * 2 - 2);

			VAO->Unbind();

			CircleOutlineShader = Renderer::CreateShaderProgram("shaders/CircleOutline.vert", "shaders/FlatColor.frag");

			CircleOutlineBuffersInitilized = true;
		}
	}

	void CircleOutline::Draw()
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(Position.x * GlobalScaleFactor, Position.y * GlobalScaleFactor, 0.0f));
		model = glm::scale(model, glm::vec3(Radius * GlobalScaleFactor, Radius * GlobalScaleFactor, Radius * GlobalScaleFactor));

		CircleOutlineShader->Bind();
		CircleOutlineShader->SetUniformMat4("u_Model", model);
		CircleOutlineShader->SetUniformVec4("u_Color", Color);
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