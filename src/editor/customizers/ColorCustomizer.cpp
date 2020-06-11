#include <pch.h>

#include "VelocityCustomizer.h"
#include "ColorCustomizer.h"

namespace Ainan {

	void ColorCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		if (ImGui::TreeNode("Color"))
		{

			ImGui::Text("Starting Color");

			ImGui::Text("Color: ");
			ImGui::SameLine();
			ImGui::ColorEdit4("##Color: ", &StartColor.r);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Color Over Time");

			//m_Interpolator.DisplayGUI("Color Over Time Mode");
			DisplayInterpolationTypeSelector(m_InterpolationType, InterpolationSelectorFlags::NoCustomMode, this);

			if (m_InterpolationType != InterpolationType::Fixed)
			{
				ImGui::Text("End Color: ");
				ImGui::SameLine();
				ImGui::ColorEdit4("##End Color: ", &EndColor.r);
			}

			ImGui::TreePop();
		}
	}
}