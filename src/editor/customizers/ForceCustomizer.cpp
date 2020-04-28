#include <pch.h>
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
				ImGui::Text("Name of the Force");
				ImGui::SameLine();
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

				//display a dropdown displaying the types of forces
				ImGui::Text("Force Type: ");
				ImGui::SameLine();
				if (ImGui::BeginCombo("Force Type", Force::ForceTypeToString(m_Forces[m_CurrentSelectedForceName].Type)))
				{
					{
						bool isSelected = m_Forces[m_CurrentSelectedForceName].Type == Force::DirectionalForce;
						if (ImGui::Selectable(Force::ForceTypeToString(Force::DirectionalForce), &isSelected))
							m_Forces[m_CurrentSelectedForceName].Type = Force::DirectionalForce;
					}

					{
						bool isSelected = m_Forces[m_CurrentSelectedForceName].Type == Force::RelativeForce;
						if (ImGui::Selectable(Force::ForceTypeToString(Force::RelativeForce), &isSelected))
							m_Forces[m_CurrentSelectedForceName].Type = Force::RelativeForce;
					}

					ImGui::EndCombo();
				}

				ImGui::Text("Force Enable: ");
				ImGui::SameLine();
				ImGui::Checkbox("##Force Enabled: ", &m_Forces[m_CurrentSelectedForceName].Enabled);

				if (m_Forces[m_CurrentSelectedForceName].Type == Force::DirectionalForce) 
				{
					ImGui::Text("Force Value: ");
					ImGui::SameLine();
					ImGui::DragFloat2("##Force Value: ", &m_Forces[m_CurrentSelectedForceName].DF_Value.x);
				}
				else if ((m_Forces[m_CurrentSelectedForceName].Type == Force::RelativeForce))
				{
					ImGui::Text("Target Position: ");
					ImGui::SameLine();
					ImGui::DragFloat2("##Target Position", &m_Forces[m_CurrentSelectedForceName].RF_Target.x, 0.001f);

					ImGui::Text("Force Strength: ");
					ImGui::SameLine();
					ImGui::DragFloat("##Force Strength: ", &m_Forces[m_CurrentSelectedForceName].RF_Strength);
				}
			}

			ImGui::TreePop();
		}
	}
}