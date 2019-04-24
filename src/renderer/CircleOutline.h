#pragma once

#include "ShaderProgram.h"
#include "environment/Camera.h"

namespace ALZ {

	class CircleOutline {
	public:
		CircleOutline();

		void Render(Camera& camera);
		glm::vec2 GetPointByAngle(const float& angle_in_radians);

	public:
		float Radius = 0.25f;
		float Width = 3.0f;
		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec2 Position = { 0.0f, 0.0f };
	};

}