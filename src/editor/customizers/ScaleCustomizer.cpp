#include "ScaleCustomizer.h"

#include "environment/EnvironmentObjectInterface.h"

namespace Ainan {

	void ScaleCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Scale"))
		{
			ImGui::Text("Starting Scale");

			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Random Between 2 Numbers");
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			ImGui::Checkbox("Random Between 2 Numbers", &m_RandomScale);
			ImGui::NextColumn();

			if (m_RandomScale)
			{
				ImGui::Text("Minimum Scale: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Minimum Scale: ", &m_MinScale, c_ObjectScaleDragControlSpeed);

				ImGui::NextColumn();
				ImGui::Text("Maximum Scale: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Maximum Scale: ", &m_MaxScale, c_ObjectScaleDragControlSpeed);
			}
			else
			{
				ImGui::Text("Scale: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Scale: ", &m_DefinedScale, c_ObjectScaleDragControlSpeed);
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

			ImGui::NextColumn();
			ImGui::Text("Scale Over Time");
			ImGui::NextColumn();

			//m_Interpolator.DisplayGUI("Scale Over Time Mode");
			DisplayInterpolationTypeSelector(m_InterpolationType, InterpolationSelectorFlags::None, this);

			if (m_InterpolationType != InterpolationType::Fixed)
			{
				ImGui::NextColumn();
				ImGui::Text("End Scale: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##End Scale: ", &m_EndScale, 0.1f);
			}

			//to make sure end scale doesn't go lower than 0
			if (m_EndScale < 0.0f)
				m_EndScale = 0.0f;

			ImGui::NextColumn();
			ImGui::Text("Curve: ");
			ImGui::NextColumn();
			m_Curve.Type = m_InterpolationType;
			m_Curve.DisplayInCurrentWindow({ 100,75 });

			ImGui::NextColumn();
			ImGui::TreePop();
		}
	}
}