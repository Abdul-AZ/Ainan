#include "ForceCustomizer.h"

namespace Ainan {

	ForceCustomizer::ForceCustomizer()
	{
		Force gravityForce;
		gravityForce.DF_Value = glm::vec2(0.0f, -9.81);
		gravityForce.Enabled = false;

		m_Forces["Gravity"] = gravityForce;
	}

	void ForceCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Forces"))
		{
			if(ImGui::Button("Add Force"))
				m_DisplayAddForceSettings = true;

			ImGui::SameLine();
			if (ImGui::Button("Delete Force")) 
			{
				if (m_CurrentSelectedForceName != "")
					m_Forces.erase(m_CurrentSelectedForceName);
				m_CurrentSelectedForceName = "";
			}

			if (m_DisplayAddForceSettings)
			{
				auto spacing = ImGui::GetCursorPosY();
				ImGui::Text("Name of the Force");
				ImGui::NextColumn();
				ImGui::SetCursorPosY(spacing);
				if (ImGui::InputText("##Name of the Force", &m_AddForceInputString, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (m_AddForceInputString != "")
					{
						Force force;
						force.Enabled = true;

						m_Forces[m_AddForceInputString] = force;
					}

					m_AddForceInputString = "";
					m_DisplayAddForceSettings = false;
				}
				ImGui::NextColumn();
			}

			if (ImGui::ListBoxHeader("##Forces Inspector",-1, 5)) {

				for (auto& force : m_Forces)
				{
					ImGui::PushID(&force.second);

					bool isSelected = m_CurrentSelectedForceName == force.first;

					if (ImGui::Selectable(force.first.c_str(), &isSelected)) {
						m_CurrentSelectedForceName = force.first;
					}


					ImGui::PopID();
				}

				ImGui::ListBoxFooter();
			}

			//if we have a force selected. display its properties
			if (m_CurrentSelectedForceName != "")
			{
				//display a dropdown displaying the types of forces
				auto spacing = ImGui::GetCursorPosY();

				IMGUI_DROPDOWN_START_USING_COLUMNS("Force Type", Force::ForceTypeToString(m_Forces[m_CurrentSelectedForceName].Type));
				IMGUI_DROPDOWN_SELECTABLE(m_Forces[m_CurrentSelectedForceName].Type, Force::DirectionalForce, Force::ForceTypeToString(Force::DirectionalForce));
				IMGUI_DROPDOWN_SELECTABLE(m_Forces[m_CurrentSelectedForceName].Type, Force::RelativeForce, Force::ForceTypeToString(Force::RelativeForce));
				IMGUI_DROPDOWN_END();

				ImGui::NextColumn();
				ImGui::Text("Force Enable: ");
				ImGui::NextColumn();
				ImGui::Checkbox("##Force Enabled: ", &m_Forces[m_CurrentSelectedForceName].Enabled);

				if (m_Forces[m_CurrentSelectedForceName].Type == Force::DirectionalForce) 
				{
					ImGui::NextColumn();
					ImGui::Text("Force Value: ");
					ImGui::NextColumn();
					ImGui::DragFloat2("##Force Value: ", &m_Forces[m_CurrentSelectedForceName].DF_Value.x);
				}
				else if ((m_Forces[m_CurrentSelectedForceName].Type == Force::RelativeForce))
				{
					ImGui::NextColumn();
					ImGui::Text("Target Position: ");
					ImGui::NextColumn();
					ImGui::DragFloat2("##Target Position", &m_Forces[m_CurrentSelectedForceName].RF_Target.x, 0.001f);

					ImGui::NextColumn();
					ImGui::Text("Force Strength: ");
					ImGui::NextColumn();
					ImGui::DragFloat("##Force Strength: ", &m_Forces[m_CurrentSelectedForceName].RF_Strength);
				}
			}

			ImGui::NextColumn();
			ImGui::TreePop();
		}
	}
}