#pragma once

#include "renderer/ShaderProgram.h"
#include "environment/EnvironmentObjectInterface.h"

namespace Ainan {

	class RadialLight : public EnvironmentObjectInterface
	{
	public:
		RadialLight();

		void DisplayGUI() override;

		glm::vec2* GetPositionRef() override { return &Position; }

		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		
		float Intensity = 50.0f;
	};
}