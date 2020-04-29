#include <pch.h>
#include "ScaleCustomizer.h"

namespace Ainan {

	ScaleCustomizer::ScaleCustomizer() :
		mt(std::random_device{}())
		//m_Interpolator(InterpolationType::Fixed, 0.0f, 0.0f)
	{}

	void ScaleCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Scale")) {

			ImGui::Text("Starting Scale");

			ImGui::Text("Random Between 2 Numbers");
			ImGui::SameLine();
			ImGui::Checkbox("Random Between 2 Numbers", &m_RandomScale);

			if (m_RandomScale)
			{
				ImGui::Text("Minimum Scale: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Minimum Scale: ", &m_MinScale, 0.1f);

				ImGui::Text("Maximum Scale: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Maximum Scale: ", &m_MaxScale, 0.1f);
			}
			else
			{
				ImGui::Text("Scale: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Scale: ", &m_DefinedScale, 0.1f);
			}

			//to make sure scale doesn't go negative
			if (m_MinScale < 0.0f)
				m_MinScale = 0.0f;
			if (m_MaxScale < 0.0f)
				m_MaxScale = 0.0f;

			//to make sure max scale is always bigger than minscale
			if (m_MinScale > m_MaxScale)
				m_MinScale = m_MaxScale;

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Scale Over Time");

			//m_Interpolator.DisplayGUI("Scale Over Time Mode");
			DisplayInterpolationTypeSelector(m_InterpolationType, InterpolationSelectorFlags::None, this);

			if (m_InterpolationType != InterpolationType::Fixed)
			{
				ImGui::Text("End Scale: ");
				ImGui::SameLine();
				ImGui::DragFloat("##End Scale: ", &m_EndScale, 0.1f);
			}

			//to make sure end scale doesn't go lower than 0
			if (m_EndScale < 0.0f)
				m_EndScale = 0.0f;

			m_Curve.Type = m_InterpolationType;
			m_Curve.DisplayInCurrentWindow({ 100,75 });

			ImGui::TreePop();
		}
	}

	//InterpolationSelector<float> ScaleCustomizer::GetScaleInterpolator()
	//{
	//	if (m_RandomScale) {
	//		std::uniform_real_distribution<float> dist_scale(m_MinScale, m_MaxScale);
	//
	//		m_Interpolator.startPoint = dist_scale(mt);
	//		m_Interpolator.endPoint = m_EndScale;
	//
	//		return m_Interpolator;
	//	}
	//	else {
	//
	//		m_Interpolator.startPoint = m_DefinedScale;
	//		m_Interpolator.endPoint = m_EndScale;
	//
	//		return m_Interpolator;
	//	}
	//}
}