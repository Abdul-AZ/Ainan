#include <pch.h>
#include "VelocityCustomizer.h"

VelocityCustomizer::VelocityCustomizer() :
	mt(std::random_device{}())
{}

void VelocityCustomizer::DisplayGUI()
{
	if (ImGui::TreeNode("Starting Velocity")) {

		ImGui::Checkbox("Random Between 2 Numbers", &RandomVelocity);

		if (RandomVelocity)
		{
			if (maxVelocity.x < minVelocity.x)
				maxVelocity.x = minVelocity.x;

			if (maxVelocity.y < minVelocity.y)
				maxVelocity.y = minVelocity.y;

			ImGui::SliderFloat2("Minimum Velocity:", &minVelocity.x, -500.0f, 500.0f);
			ImGui::SliderFloat2("Maximum Velocity:", &maxVelocity.x, -500.0f, 500.0f);
		}
		else
		{
			ImGui::SliderFloat2("Velocity:", &definedVelocity.x, -500.0f, 500.0f);
		}

		ImGui::TreePop();
	}
}

glm::vec2 VelocityCustomizer::GetVelocity()
{

	if (RandomVelocity) {
		std::uniform_real_distribution<float> dist_velocity_x(minVelocity.x, maxVelocity.x);
		std::uniform_real_distribution<float> dist_velocity_y(minVelocity.y, maxVelocity.y);
		return glm::vec2(dist_velocity_x(mt), dist_velocity_y(mt));
	}
	else
		return definedVelocity;
}
