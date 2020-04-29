#include <pch.h>

#include "VelocityCustomizer.h"
#include "ColorCustomizer.h"

namespace Ainan {

	ColorCustomizer::ColorCustomizer() :
		mt(std::random_device{}()),
		m_Interpolator(InterpolationType::Fixed, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0, 0, 0, 0), InterpolationSelectorFlags::NoCustomMode)
	{}

	void ColorCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Color")) {

			ImGui::Text("Starting Color");

			ImGui::Text("Color: ");
			ImGui::SameLine();
			ImGui::ColorEdit4("##Color: ", &m_Interpolator.startPoint.r);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Color Over Time");

			m_Interpolator.DisplayGUI("Color Over Time Mode");

			if (m_Interpolator.Type != InterpolationType::Fixed)
			{
				ImGui::Text("End Color: ");
				ImGui::SameLine();
				ImGui::ColorEdit4("##End Color: ", &m_Interpolator.endPoint.r);
			}

			ImGui::TreePop();
		}
	}

	InterpolationSelector<glm::vec4> ColorCustomizer::GetColorInterpolator()
	{
		return m_Interpolator;
	}
}