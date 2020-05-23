#pragma once

#include "renderer/Renderer.h"

namespace Ainan {

	class CircleOutline {
	public:
		CircleOutline();

		void Draw();
		glm::vec2 GetPointByAngle(float angle_in_radians);

	public:
		float Radius = 0.25f;
		float Width = 3.0f;
		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec2 Position = { 0.0f, 0.0f };

	private:
		std::shared_ptr<IndexBuffer> EBO = nullptr;
		std::shared_ptr<VertexBuffer> VBO = nullptr;
		//two different buffers because one is in the vertex shader and the other is in the fragment shader
		std::shared_ptr<UniformBuffer> ColorUniformBuffer = nullptr;
		std::shared_ptr<UniformBuffer> TransformUniformBuffer = nullptr;
	};

}