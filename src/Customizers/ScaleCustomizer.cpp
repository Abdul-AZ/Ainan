#include "ScaleCustomizer.h"

ScaleCustomizer::ScaleCustomizer() :
	mt(std::random_device{}()),
	m_Interpolator(InterpolatorMode::Fixed, 0.0f, 0.0f)
{}

void ScaleCustomizer::DisplayGUI()
{
	if (ImGui::TreeNode("Scale")) {

		ImGui::Text("Staring Scale");

		ImGui::Checkbox("Random Between 2 Numbers", &RandomScale);

		if (RandomScale)
		{
			if (maxScale < minScale)
				maxScale = minScale;

			ImGui::SliderFloat("Minimum Scale:", &minScale, 0.1f, 25.0f);
			ImGui::SliderFloat("Maximum Scale:", &maxScale, 0.1f, 25.0f);
		}
		else
		{
			ImGui::SliderFloat("Scale:", &definedScale, 0.1f, 25.0f);
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Scale Over Time");

		if(ImGui::BeginCombo("Scale Over Time Mode", m_Interpolator.InterpolateModeStr(m_Interpolator.Mode).c_str())){

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
			ImGui::SliderFloat("End Scale:", &endScale, 0.1f, 25.0f);
		}

		ImGui::TreePop();
	}
}

Interpolator<float>& ScaleCustomizer::GetScaleInterpolator()
{
	if (RandomScale) {
		std::uniform_real_distribution<float> dist_scale(minScale, maxScale);

		m_Interpolator.startPoint = dist_scale(mt);
		m_Interpolator.endPoint = endScale;

		return m_Interpolator;
	}
	else {

		m_Interpolator.startPoint = definedScale;
		m_Interpolator.endPoint = endScale;

		return m_Interpolator;
	}
}
