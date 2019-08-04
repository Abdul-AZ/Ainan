#include <pch.h>
#include "ForceCustomizer.h"

namespace ALZ {

	ForceCustomizer::ForceCustomizer()
	{
		m_Forces["Gravity"] = Force{ glm::vec2(0.0f, -9.81), false };
	}

	void ForceCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Forces")) {

			if(ImGui::Button("Add Force"))
				m_DisplayAddForceSettings = true;

			ImGui::SameLine();
			if (ImGui::Button("Delete Force")) {
				if (m_CurrentSelectedForceName != "")
					m_Forces.erase(m_CurrentSelectedForceName);
				m_CurrentSelectedForceName = "";
			}

			if (m_DisplayAddForceSettings)
			{
				if (ImGui::InputText("Name of the Force", &m_AddForceInputString, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (m_AddForceInputString != "")
					{
						m_Forces[m_AddForceInputString] = { {0.0,0.0}, true };
					}

					m_AddForceInputString = "";
					m_DisplayAddForceSettings = false;
				}
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
				ImGui::Text(m_CurrentSelectedForceName.c_str());
				ImGui::Checkbox("Force Enabled", &m_Forces[m_CurrentSelectedForceName].Enabled);
				ImGui::DragFloat2("Force Value", &m_Forces[m_CurrentSelectedForceName].Value.x);
			}


			ImGui::TreePop();
		}
	}
}