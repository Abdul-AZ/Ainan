#pragma once

#include "renderer/ShaderProgram.h"
#include "misc/InspectorInterface.h"

namespace ALZ {

	class RadialLight : public InspectorInterface
	{
	public:
		RadialLight();

		void DisplayGUI(Camera& camera) override;

		glm::vec2& GetPositionRef() override { return Position; }

		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		float Constant = 50.0f;
		float Linear = 0.045f;
		float Quadratic = 0.0075f;
		float Intensity = 50.0f;
	};
}