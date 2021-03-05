#include "VelocityCustomizer.h"
#include "ColorCustomizer.h"

namespace Ainan {

	void ColorCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Color"))
		{
			ImGui::Text("Starting Color");

			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Color: ");
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			ImGui::ColorEdit4("##Color: ", &StartColor.r);

			ImGui::NextColumn();
			ImGui::Text("Color Over Time");
			ImGui::NextColumn();

			//m_Interpolator.DisplayGUI("Color Over Time Mode");
			DisplayInterpolationTypeSelector(m_InterpolationType, InterpolationSelectorFlags::NoCustomMode, this);

			if (m_InterpolationType != InterpolationType::Fixed)
			{
				ImGui::NextColumn();
				ImGui::Text("End Color: ");
				ImGui::NextColumn();
				ImGui::ColorEdit4("##End Color: ", &EndColor.r);
			}

			ImGui::NextColumn();
			ImGui::TreePop();
		}
	}
}