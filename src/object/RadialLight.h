#pragma once

#include "renderer/ShaderProgram.h"
#include "misc/EnvironmentObjectInterface.h"

namespace ALZ {

	class RadialLight : public EnvironmentObjectInterface
	{
	public:
		RadialLight();

		void DisplayGUI() override;

		glm::vec2& GetPositionRef() override { return Position; }

		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);
		
		float Intensity = 50.0f;
	};
}