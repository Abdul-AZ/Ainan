#include <pch.h>

#include "LifetimeCustomizer.h"

namespace ALZ {

	LifetimeCustomizer::LifetimeCustomizer() :
		mt(std::random_device{}())
	{}

	void LifetimeCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Starting Lifetime")) {

			ImGui::Checkbox("Random Between 2 Numbers", &m_RandomLifetime);

			if (m_RandomLifetime)
			{
				if (m_MaxLifetime < m_MinLifetime)
					m_MaxLifetime = m_MinLifetime;

				ImGui::DragFloat("Minimum Lifetime:", &m_MinLifetime, 0.1f);
				ImGui::DragFloat("Maximum Lifetime:", &m_MaxLifetime, 0.1f);
			}
			else
			{
				ImGui::DragFloat("Lifetime:", &m_DefinedLifetime, 0.1f);
			}
			ImGui::TreePop();
		}

		//to make sure scale doesn't go negative
		if (m_MinLifetime < 0.0f)
			m_MinLifetime = 0.0f;
		if (m_MaxLifetime < 0.0f)
			m_MaxLifetime = 0.0f;
		if (m_DefinedLifetime < 0.0f)
			m_DefinedLifetime = 0.0f;

		//to make sure max scale is always bigger than minscale
		if (m_MinLifetime > m_MaxLifetime)
			m_MinLifetime = m_MaxLifetime;
	}

	float LifetimeCustomizer::GetLifetime()
	{
		if (m_RandomLifetime) {
			std::uniform_real_distribution<float> dist_time(m_MinLifetime, m_MaxLifetime);
			return dist_time(mt);
		}
		else
			return m_DefinedLifetime;
	}
}