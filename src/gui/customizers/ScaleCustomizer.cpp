#include <pch.h>
#include "ScaleCustomizer.h"

namespace ALZ {

	ScaleCustomizer::ScaleCustomizer() :
		mt(std::random_device{}()),
		m_Interpolator(InterpolationType::Fixed, 0.0f, 0.0f)
	{}

	void ScaleCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Scale")) {

			ImGui::Text("Starting Scale");

			ImGui::Checkbox("Random Between 2 Numbers", &RandomScale);

			if (RandomScale)
			{
				ImGui::DragFloat("Minimum Scale:", &minScale, 0.1f);
				ImGui::DragFloat("Maximum Scale:", &maxScale, 0.1f);
			}
			else
			{
				ImGui::DragFloat("Scale:", &definedScale, 0.1f);
			}

			//to make sure scale doesn't go negative
			if (minScale < 0.0f)
				minScale = 0.0f;
			if (maxScale < 0.0f)
				maxScale = 0.0f;

			//to make sure max scale is always bigger than minscale
			if (minScale > maxScale)
				minScale = maxScale;

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Scale Over Time");

			m_Interpolator.DisplayGUI("Scale Over Time Mode");

			if (m_Interpolator.Type != InterpolationType::Fixed)
			{
				ImGui::DragFloat("End Scale:", &endScale, 0.1f);
			}
			//to make sure end scale doesn't go lower than 0
			if (endScale < 0.0f)
				endScale = 0.0f;

			ImGui::TreePop();
		}
	}

	InterpolationSelector<float>& ScaleCustomizer::GetScaleInterpolator()
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
}