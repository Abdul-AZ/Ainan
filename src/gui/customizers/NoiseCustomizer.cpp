#include <pch.h>

#include "NoiseCustomizer.h"

namespace ALZ {

	void NoiseCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Noise")) {

			ImGui::Checkbox("Enabled :", &NoiseEnabled);
			if (NoiseEnabled)
			{
				ImGui::DragFloat("Strength :", &NoiseStrength, 0.5f, 0.0f);
			}

			ImGui::TreePop();
		}
	}
}