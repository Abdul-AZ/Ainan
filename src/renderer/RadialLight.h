#pragma once

#include "ShaderProgram.h"
#include "environment/InspectorInterface.h"

namespace ALZ {

	class RadialLight : public InspectorInterface
	{
	public:
		RadialLight();

		void DisplayGUI(Camera& camera) override;

		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		float Constant = 1.0f;
		float Linear = 1.2f;
		float Quadratic = 1000.0f;
	};
}