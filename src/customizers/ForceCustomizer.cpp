#include <pch.h>
#include "ForceCustomizer.h"

namespace ALZ {
	void ForceCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Forces")) {

			ImGui::Checkbox("Gravity", &m_EnableGravity);

			if (m_EnableGravity)
			{
				ImGui::DragFloat("Gravity Strength", &m_GravityStrength);
				m_Forces["Gravity"] = glm::vec2(0.0f, m_GravityStrength);
			}

			ImGui::TreePop();
		}
	}
}