#pragma once

#include "renderer/ShaderProgram.h"
#include "environment/EnvironmentObjectInterface.h"

namespace Ainan {

	class RadialLight : public EnvironmentObjectInterface
	{
	public:
		RadialLight();

		void DisplayGuiControls() override;
		int32_t GetAllowedGizmoOperation(ImGuizmo::OPERATION operation) override;

		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		ObjSpace Space = OBJ_SPACE_2D;
		
		float Intensity = 1.0f;
	};
}