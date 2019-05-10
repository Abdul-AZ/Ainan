#include <pch.h>
#include "VelocityCustomizer.h"

namespace ALZ {

	VelocityCustomizer::VelocityCustomizer() :
		mt(std::random_device{}())
	{}

	void VelocityCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Starting Velocity")) {

			ImGui::Checkbox("Random Between 2 Numbers", &m_RandomVelocity);

			if (m_RandomVelocity)
			{
				if (m_MaxVelocity.x < m_MinVelocity.x)
					m_MaxVelocity.x = m_MinVelocity.x;

				if (m_MaxVelocity.y < m_MinVelocity.y)
					m_MaxVelocity.y = m_MinVelocity.y;

				ImGui::DragFloat2("Minimum Velocity:", &m_MinVelocity.x);
				ImGui::DragFloat2("Maximum Velocity:", &m_MaxVelocity.x);
			}
			else
			{
				ImGui::DragFloat2("Velocity:", &m_DefinedVelocity.x);
			}

			ImGui::TreePop();
		}
	}

	glm::vec2 VelocityCustomizer::GetVelocity()
	{

		if (m_RandomVelocity) {
			if (m_MinVelocity.x > m_MaxVelocity.x)
				m_MinVelocity.x = m_MaxVelocity.x;
			if (m_MinVelocity.y > m_MaxVelocity.y)
				m_MinVelocity.y = m_MaxVelocity.y;

			std::uniform_real_distribution<float> dist_velocity_x(m_MinVelocity.x, m_MaxVelocity.x);
			std::uniform_real_distribution<float> dist_velocity_y(m_MinVelocity.y, m_MaxVelocity.y);
			return glm::vec2(dist_velocity_x(mt), dist_velocity_y(mt));
		}
		else
			return m_DefinedVelocity;
	}
}