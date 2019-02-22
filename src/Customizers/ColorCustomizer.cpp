#include "VelocityCustomizer.h"
#include "ColorCustomizer.h"
ColorCustomizer::ColorCustomizer() :
	mt(std::random_device{}()),
	m_Interpolator(InterpolatorMode::Fixed, glm::vec4(0,0,0,0), glm::vec4(0, 0, 0, 0))
{}

void ColorCustomizer::DisplayGUI()
{
	if (ImGui::TreeNode("Color")) {

		ImGui::Text("Staring Color");

		ImGui::ColorPicker4("Color:", &definedColor.r);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Color Over Time");

		if (ImGui::BeginCombo("Color Over Time Mode", m_Interpolator.InterpolateModeStr(m_Interpolator.Mode).c_str())) {

			{
				bool is_Active = m_Interpolator.Mode == InterpolatorMode::Fixed;
				if (ImGui::Selectable(m_Interpolator.InterpolateModeStr(InterpolatorMode::Fixed).c_str(), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					m_Interpolator.Mode = InterpolatorMode::Fixed;
				}
			}

			{
				bool is_Active = m_Interpolator.Mode == InterpolatorMode::Linear;
				if (ImGui::Selectable(m_Interpolator.InterpolateModeStr(InterpolatorMode::Linear).c_str(), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					m_Interpolator.Mode = InterpolatorMode::Linear;
				}
			}

			ImGui::EndCombo();
		}

		if (m_Interpolator.Mode == InterpolatorMode::Linear)
		{
			ImGui::ColorPicker4("End Color:", &endColor.r);
		}

		ImGui::TreePop();
	}
}

Interpolator<glm::vec4>& ColorCustomizer::GetColorInterpolator()
{
	m_Interpolator.startPoint = definedColor;
	m_Interpolator.endPoint = endColor;

	return m_Interpolator;
}
