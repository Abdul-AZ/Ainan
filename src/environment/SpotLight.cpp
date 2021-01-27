#include "SpotLight.h"

#include "renderer/Renderer.h"

namespace Ainan {

	SpotLight::SpotLight()
	{
		Type = EnvironmentObjectType::SpotLightType;
		m_Name = "Spot Light";
	}

	void SpotLight::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::SetNextWindowSizeConstraints(ImVec2(350.0f, 250.0f), ImVec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().max()));

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen, ImGuiWindowFlags_NoSavedSettings);
		const float xPos = 100.0f;

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat2("##Position: ", &Position.x, c_ObjectPositionDragControlSpeed);

		ImGui::Text("Color: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::ColorEdit4("##Color: ", &Color.r);

		ImGui::Text("Angle: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Angle: ", &Angle, 1.0f, 0.0f, 360.0f);

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

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();

		ImGui::PopID();
	}
}
