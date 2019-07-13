#include <pch.h>

#include "RadialLight.h"

namespace ALZ {

	static int nameIndextemp = 10000;
	RadialLight::RadialLight()
	{
		Type = InspectorObjectType::RadiaLightType;
		m_Name = "Radial Light## (" + std::to_string(nameIndextemp) + ")";
		ID = nameIndextemp;
		nameIndextemp++;
	}

	void RadialLight::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::Begin(m_Name.c_str(), &EditorOpen);

		ImGui::DragFloat2("Position", &Position.x, 0.001f);
		ImGui::ColorEdit3("Color", &Color.r);
		ImGui::Spacing();
		ImGui::Text("Light Strength :");
		ImGui::DragFloat("Intensity", &Intensity, 0.1f);;

		ImGui::End();
	}
}