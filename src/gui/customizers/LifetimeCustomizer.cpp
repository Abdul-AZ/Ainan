#include <pch.h>

#include "LifetimeCustomizer.h"

namespace ALZ {

	LifetimeCustomizer::LifetimeCustomizer() :
		mt(std::random_device{}())
	{}

	void LifetimeCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Starting Lifetime")) {

			ImGui::Checkbox("Random Between 2 Numbers", &RandomLifetime);

			if (RandomLifetime)
			{
				if (maxLifetime < minLifetime)
					maxLifetime = minLifetime;

				ImGui::DragFloat("Minimum Lifetime:", &minLifetime, 0.1f);
				ImGui::DragFloat("Maximum Lifetime:", &maxLifetime, 0.1f);
			}
			else
			{
				ImGui::DragFloat("Lifetime:", &definedLifetime, 0.1f);
			}
			ImGui::TreePop();
		}

		//to make sure scale doesn't go negative
		if (minLifetime < 0.0f)
			minLifetime = 0.0f;
		if (maxLifetime < 0.0f)
			maxLifetime = 0.0f;
		if (definedLifetime < 0.0f)
			definedLifetime = 0.0f;

		//to make sure max scale is always bigger than minscale
		if (minLifetime > maxLifetime)
			minLifetime = maxLifetime;
	}

	float LifetimeCustomizer::GetLifetime()
	{
		if (RandomLifetime) {
			std::uniform_real_distribution<float> dist_time(minLifetime, maxLifetime);
			return dist_time(mt);
		}
		else
			return definedLifetime;
	}
}