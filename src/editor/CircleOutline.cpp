#include <pch.h>

#include "CircleOutline.h"

namespace Ainan {

	static const int vertexCount = 60;
	CircleOutline::CircleOutline()
	{
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

		VertexLayout layout(1);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		VBO = Renderer::CreateVertexBuffer(vertices, sizeof(glm::vec2) * vertexCount, layout, Renderer::ShaderLibrary["CircleOutlineShader"]);

		EBO = Renderer::CreateIndexBuffer(indecies, vertexCount * 2 - 2);

		TransformUniformBuffer = Renderer::CreateUniformBuffer("ObjectTransform", 1, { {"u_Model", ShaderVariableType::Mat4} }, nullptr);
		ColorUniformBuffer = Renderer::CreateUniformBuffer("ObjectColor", 2, { {"u_Color", ShaderVariableType::Vec4} }, nullptr);
	}

	void CircleOutline::Draw()
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(Position.x * c_GlobalScaleFactor, Position.y * c_GlobalScaleFactor, 0.0f));
		model = glm::scale(model, glm::vec3(Radius * c_GlobalScaleFactor, Radius * c_GlobalScaleFactor, Radius * c_GlobalScaleFactor));

		auto& shader = Renderer::ShaderLibrary["CircleOutlineShader"];

		shader->BindUniformBuffer(TransformUniformBuffer, 1, RenderingStage::VertexShader);
		TransformUniformBuffer->UpdateData(&model);

		shader->BindUniformBuffer(ColorUniformBuffer, 2, RenderingStage::FragmentShader);
		ColorUniformBuffer->UpdateData(&Color);

		Renderer::Draw(*VBO, *shader, Primitive::Lines, *EBO);

		shader->Unbind();
	}

	glm::vec2 CircleOutline::GetPointByAngle(float angle_in_radians)
	{
		float x = Position.x  * c_GlobalScaleFactor + Radius * cos(angle_in_radians) * c_GlobalScaleFactor;
		float y = Position.y  * c_GlobalScaleFactor + Radius * sin(angle_in_radians) * c_GlobalScaleFactor;

		return glm::vec2(x, y);
	}
}