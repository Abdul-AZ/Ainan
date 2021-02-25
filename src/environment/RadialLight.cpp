#include "RadialLight.h"

#include "renderer/Renderer.h"

namespace Ainan {

	RadialLight::RadialLight()
	{
		Type = EnvironmentObjectType::RadialLightType;
		m_Name = "Radial Light";
	}

	void RadialLight::DisplayGuiControls()
	{
		ImGui::Text("Position: ");
		ImGui::SameLine();
		float xPos = ImGui::GetCursorPosX();
		ImGui::DragFloat2("##Position: ", &Model[3][0], c_ObjectPositionDragControlSpeed);

		ImGui::Text("Color: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::ColorEdit4("##Color: ", &Color.r);

		ImGui::Spacing();

		ImGui::Text("Intensity: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
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