#include <pch.h>
#include "VelocityCustomizer.h"

namespace ALZ {

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

				ImGui::DragFloat2("Minimum Velocity:", &minVelocity.x);
				ImGui::DragFloat2("Maximum Velocity:", &maxVelocity.x);
			}
			else
			{
				ImGui::DragFloat2("Velocity:", &definedVelocity.x);
			}

			ImGui::TreePop();
		}
	}

	glm::vec2 VelocityCustomizer::GetVelocity()
	{

		if (RandomVelocity) {
			if (minVelocity.x > maxVelocity.x)
				minVelocity.x = maxVelocity.x;
			if (minVelocity.y > maxVelocity.y)
				minVelocity.y = maxVelocity.y;

			std::uniform_real_distribution<float> dist_velocity_x(minVelocity.x, maxVelocity.x);
			std::uniform_real_distribution<float> dist_velocity_y(minVelocity.y, maxVelocity.y);
			return glm::vec2(dist_velocity_x(mt), dist_velocity_y(mt));
		}
		else
			return definedVelocity;
	}
}