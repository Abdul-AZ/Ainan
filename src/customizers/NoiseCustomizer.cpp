#include <pch.h>

#include "NoiseCustomizer.h"

namespace ALZ {

	void NoiseCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Noise")) {

			ImGui::Checkbox("Enabled :", &m_NoiseEnabled);
			if (m_NoiseEnabled)
			{
				ImGui::DragFloat("Strength :", &m_NoiseStrength, 0.5f, 0.0f);
			}

			ImGui::TreePop();
		}
	}
}