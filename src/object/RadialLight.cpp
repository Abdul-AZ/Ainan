#include <pch.h>

#include "RadialLight.h"

namespace ALZ {

	static int nameIndextemp = 10000;
	RadialLight::RadialLight()
	{
		Type = InspectorObjectType::RadiaLightType;
		m_Name = "Radial Light (" + std::to_string(nameIndextemp) + ")";
		ID = nameIndextemp;
		nameIndextemp++;
	}

	void RadialLight::DisplayGUI(Camera& camera)
	{
		if (!EditorOpen)
			return;

		ImGui::Begin(m_Name.c_str(), &EditorOpen);

		ImGui::DragFloat2("Position", &Position.x, 0.001f);
		ImGui::ColorEdit3("Color", &Color.r);
		ImGui::Spacing();
		ImGui::Text("Light Strength :");
		ImGui::DragFloat("Constant", &Constant, 0.01f);
		ImGui::DragFloat("Linear", &Linear, 0.0001f);
		ImGui::DragFloat("Quadratic", &Quadratic, 0.00001f);
		ImGui::DragFloat("Intensity", &Intensity, 0.1f);;

		Constant = std::clamp(Constant, 0.0f, 10000.0f);
		Linear = std::clamp(Linear, 0.0f, 10000.0f);
		Quadratic = std::clamp(Quadratic, 0.0f, 10000.0f);
		Intensity = std::clamp(Intensity, 0.0f, 10000.0f);

		ImGui::End();
	}
}