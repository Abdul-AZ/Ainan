#pragma once

#include "environment/EnvironmentObjectInterface.h"

namespace Ainan {

	class SpotLight : public EnvironmentObjectInterface
	{
	public:
		SpotLight();

		void DisplayGUI() override;

		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		float Intensity = 1.0f;
		float Angle = 0.0f; //in degrees
		float InnerCutoff = 30.0f; //in degrees
		float OuterCutoff = 40.0f; //in degrees
	};

}