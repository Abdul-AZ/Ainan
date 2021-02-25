#include "SpotLight.h"

#include "renderer/Renderer.h"

namespace Ainan {

	SpotLight::SpotLight()
	{
		Type = EnvironmentObjectType::SpotLightType;
		m_Name = "Spot Light";
	}

	void SpotLight::DisplayGuiControls()
	{
		const float xPos = 100.0f;

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(Model, scale, rotation, translation, skew, perspective);

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat2("##Position: ", &Model[3][0], c_ObjectPositionDragControlSpeed);

		ImGui::Text("Color: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::ColorEdit4("##Color: ", &Color.r);

		ImGui::Text("Angle: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		float rotEular = glm::eulerAngles(rotation).z * 180.0f / glm::pi<float>();
		if(ImGui::DragFloat("##Angle: ", &rotEular, 1.0f, -180.0f, 180.0f));
		{
			//reconstruct model with new rotation
			Model = glm::mat4(1.0f);
			Model = glm::translate(Model, translation);
			Model = glm::rotate(Model, rotEular * glm::pi<float>() / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
			Model = glm::scale(Model, scale);
		}

		ImGui::Text("Inner Cutoff: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Inner Cutoff: ", &InnerCutoff, 1.0f, 0.0f, OuterCutoff);

		ImGui::Text("Outer Cutoff: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Outer Cutoff: ", &OuterCutoff, 1.0f, InnerCutoff, 90.0f);

		ImGui::Text("Intensity: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
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
