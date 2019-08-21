#include <pch.h>

#include "RadialLight.h"

namespace ALZ {

	RadialLight::RadialLight()
	{
		Type = InspectorObjectType::RadialLightType;
		m_Name = "Radial Light";
	}

	void RadialLight::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen);

		ImGui::DragFloat2("Position", &Position.x, 0.001f);
		ImGui::ColorEdit3("Color", &Color.r);
		ImGui::Spacing();
		ImGui::Text("Light Strength :");
		ImGui::DragFloat("Intensity", &Intensity, 0.1f);;

		ImGui::End();
		
		ImGui::PopID();
	}
}