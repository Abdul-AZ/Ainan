#include "RadialLight.h"

#include "renderer/Renderer.h"

namespace Ainan {

	RadialLight::RadialLight()
	{
		Type = EnvironmentObjectType::RadialLightType;
		m_Name = "Radial Light";
	}

	void RadialLight::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f, 200.0f), ImVec2(std::numeric_limits<float>().max(), std::numeric_limits<float>().max()));

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen, ImGuiWindowFlags_NoSavedSettings);

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

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
		
		ImGui::PopID();
	}
}