#include "LifetimeCustomizer.h"
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

			ImGui::SliderFloat("Minimum Lifetime:", &minLifetime, 0.1f, 5.0f);
			ImGui::SliderFloat("Maximum Lifetime:", &maxLifetime, 0.1f, 5.0f);
		}
		else
		{
			ImGui::SliderFloat("Lifetime:", &definedLifetime, 0.1f, 5.0f);
		}
		ImGui::TreePop();
	}
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
