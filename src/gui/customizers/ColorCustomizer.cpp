#include <pch.h>

#include "VelocityCustomizer.h"
#include "ColorCustomizer.h"

namespace ALZ {

	ColorCustomizer::ColorCustomizer() :
		mt(std::random_device{}()),
		m_Interpolator(InterpolationType::Fixed, glm::vec4(0, 0, 0, 0), glm::vec4(0, 0, 0, 0))
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

			m_Interpolator.DisplayGUI("Color Over Time Mode");

			if (m_Interpolator.Type != InterpolationType::Fixed)
			{
				ImGui::ColorPicker4("End Color:", &endColor.r);
			}

			ImGui::TreePop();
		}
	}

	InterpolationSelector<glm::vec4>& ColorCustomizer::GetColorInterpolator()
	{
		m_Interpolator.startPoint = definedColor;
		m_Interpolator.endPoint = endColor;

		return m_Interpolator;
	}
}