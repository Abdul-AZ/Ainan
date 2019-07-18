#include <pch.h>

#include "SpotLight.h"

namespace ALZ {
	SpotLight::SpotLight()
	{
		Type = InspectorObjectType::SpotLightType;
		m_Name = "Spot Light";
	}

	void SpotLight::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen);

		ImGui::DragFloat2("Position", &Position.x, 0.001f);
		ImGui::ColorEdit3("Color", &Color.r);

		ImGui::DragFloat("Angle", &Angle, 1.0f, 0.0f, 360.0f);
		ImGui::DragFloat("Inner Cutoff Angle", &InnerCutoff, 1.0f, 0.0f, OuterCutoff);
		ImGui::DragFloat("Outer Cutoff Angle", &OuterCutoff, 1.0f, InnerCutoff, 90.0f);
		ImGui::DragFloat("Intensity", &Intensity, 5.0f, 0.1f, 10000.0f);

		ImGui::End();

		ImGui::PopID();
	}
}
