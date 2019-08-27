#pragma once

#include "misc/EnvironmentObjectInterface.h"

namespace ALZ {

	class SpotLight : public EnvironmentObjectInterface
	{
	public:
		SpotLight();

		void DisplayGUI() override;
		glm::vec2& GetPositionRef() override { return Position; }

		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0f, 1.0f, 1.0f);

		float Intensity = 50.0f;
		float Angle = 0.0f; //in degrees
		float InnerCutoff = 30.0f; //in degrees
		float OuterCutoff = 40.0f; //in degrees
	};

}