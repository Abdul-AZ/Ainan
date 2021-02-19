#pragma once

#include "renderer/ShaderProgram.h"
#include "environment/EnvironmentObjectInterface.h"

namespace Ainan {

	class RadialLight : public EnvironmentObjectInterface
	{
	public:
		RadialLight();

		void DisplayGUI() override;

		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		
		float Intensity = 1.0f;
	};
}