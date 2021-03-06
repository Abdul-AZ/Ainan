#include "SpotLight.h"

#include "renderer/Renderer.h"

namespace Ainan {

	SpotLight::SpotLight()
	{
		Type = EnvironmentObjectType::SpotLightType;
		Space = OBJ_SPACE_2D;
		m_Name = "Spot Light";
	}

	void SpotLight::DisplayGuiControls()
	{
		DisplayTransformationControls();

		ImGui::NextColumn();
		ImGui::Text("Color: ");
		ImGui::NextColumn();
		ImGui::ColorEdit4("##Color: ", &Color.r);

		ImGui::NextColumn();
		ImGui::Text("Inner Cutoff Angle: ");
		ImGui::NextColumn();
		ImGui::DragFloat("##Inner Cutoff Angle: ", &InnerCutoff, 1.0f, 0.0f, OuterCutoff);

		ImGui::NextColumn();
		ImGui::Text("Outer Cutoff Angle: ");
		ImGui::NextColumn();
		ImGui::DragFloat("##Outer Cutoff Angle: ", &OuterCutoff, 1.0f, InnerCutoff, 90.0f);

		ImGui::NextColumn();
		ImGui::Text("Intensity: ");
		ImGui::NextColumn();
		ImGui::DragFloat("##Intensity: ", &Intensity, 5.0f, 0.1f, 10000.0f);
	}

	int32_t SpotLight::GetAllowedGizmoOperation(ImGuizmo::OPERATION operation)
	{
		if (Space == OBJ_SPACE_2D)
		{
			if (operation == ImGuizmo::OPERATION::TRANSLATE)
				return ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
			else if (operation == ImGuizmo::OPERATION::ROTATE)
				return ImGuizmo::OPERATION::ROTATE_Z;
			else if (operation == ImGuizmo::OPERATION::SCALE)
				return ImGuizmo::OPERATION::BOUNDS; //work around for not allowing any operation
		}
		else if (Space == OBJ_SPACE_3D)
		{
			return operation;
		}

		AINAN_LOG_ERROR("Invalid Gizmo Operation Given");
		return -1;
	}
}
