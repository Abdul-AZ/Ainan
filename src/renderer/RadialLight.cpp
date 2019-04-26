#include <pch.h>

#include "RadialLight.h"

namespace ALZ {

	static int nameIndextemp = 0;
	RadialLight::RadialLight()
	{
		Type = InspectorObjectType::RadiaLightType;
		m_Name = "Radial Light (" + std::to_string(nameIndextemp) + ")";
		m_ID = nameIndextemp;
		nameIndextemp++;
	}

	void RadialLight::DisplayGUI(Camera& camera)
	{
		if (!m_EditorOpen)
			return;

		ImGui::Begin(m_Name.c_str(), &m_EditorOpen);

		ImGui::DragFloat2("Position", &Position.x, 0.01f);
		ImGui::ColorEdit4("Color", &Color.r);
		ImGui::Spacing();
		ImGui::Text("Light Strength :");
		ImGui::DragFloat("Constant", &Constant, 0.01f);
		ImGui::DragFloat("Linear", &Linear, 0.01f);
		ImGui::DragFloat("Quadratic", &Quadratic, 0.01f);

		Constant = std::clamp(Constant, 0.0f, 10000.0f);
		Linear = std::clamp(Linear, 0.0f, 10000.0f);
		Quadratic = std::clamp(Quadratic, 0.0f, 10000.0f);

		ImGui::End();
	}
}