#include "RadialLight.h"

#include "renderer/Renderer.h"

namespace Ainan {

	RadialLight::RadialLight()
	{
		Type = EnvironmentObjectType::RadialLightType;
		Space = OBJ_SPACE_2D;
		m_Name = "Radial Light";
	}

	void RadialLight::DisplayGuiControls()
	{
		DisplayTransformationControls();

		ImGui::NextColumn();
		ImGui::Text("Color: ");
		ImGui::NextColumn();
		ImGui::ColorEdit4("##Color: ", &Color.r);

		ImGui::NextColumn();
		ImGui::Text("Intensity: ");
		ImGui::NextColumn();
		ImGui::DragFloat("##Intensity: ", &Intensity, 0.1f);;
	}

	int32_t RadialLight::GetAllowedGizmoOperation(ImGuizmo::OPERATION operation)
	{
		if (Space == OBJ_SPACE_2D)
		{
			if (operation == ImGuizmo::OPERATION::TRANSLATE)
				return ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
			else if (operation == ImGuizmo::OPERATION::ROTATE || operation == ImGuizmo::OPERATION::SCALE)
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