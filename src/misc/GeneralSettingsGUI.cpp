#include <pch.h>
#include "GeneralSettingsGUI.h"

namespace Ainan {

	void GeneralSettingsGUI::DisplayGUI()
	{
		if (!GeneralSettingsWindowOpen)
			return;

		ImGui::Begin("Settings", &GeneralSettingsWindowOpen);

		if (ImGui::TreeNode("Blend Settings:"))
		{
			ImGui::Text("Mode");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Mode", (m_ActiveBlendMode == BlendMode::Additive)? "Additive" : "Screen"))
			{
				{
					bool is_Active = m_ActiveBlendMode == BlendMode::Additive;
					if (ImGui::Selectable("Additive", &is_Active)) {
					
						ImGui::SetItemDefaultFocus();
						m_ActiveBlendMode = BlendMode::Additive;
					}
				}

				{
					bool is_Active = m_ActiveBlendMode == BlendMode::Screen;
					if (ImGui::Selectable("Screen", &is_Active)) {
					
						ImGui::SetItemDefaultFocus();
						m_ActiveBlendMode = BlendMode::Screen;
					}
				}
				glEnable(GL_BLEND);

				if (m_ActiveBlendMode == BlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				else if (m_ActiveBlendMode == BlendMode::Screen)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

				ImGui::EndCombo();

			}

			ImGui::TreePop();
		}
		ImGui::Text("Show Grid");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Show Grid", &ShowGrid);

		ImGui::Text("Blur");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Blur", &BlurEnabled);

		if (BlurEnabled) {
			if (ImGui::TreeNode("Blur Settings: ")) {

				ImGui::Text("Blur Radius: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Blur Radius: ", &BlurRadius,0.01f, 0.0f, 5.0f);

				ImGui::TreePop();
			}
		}

		ImGui::End();
	}
}