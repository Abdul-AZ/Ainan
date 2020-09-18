#include "LifetimeCustomizer.h"

namespace Ainan {

	LifetimeCustomizer::LifetimeCustomizer() :
		mt(std::random_device{}())
	{}

	void LifetimeCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		if (ImGui::TreeNode("Starting Lifetime"))
		{

			ImGui::Text("Random Between 2 Numbers");
			ImGui::SameLine();
			ImGui::Checkbox("##Random Between 2 Numbers", &m_RandomLifetime);

			if (m_RandomLifetime)
			{
				if (m_MaxLifetime < m_MinLifetime)
					m_MaxLifetime = m_MinLifetime;

				ImGui::Text("Minimum Lifetime: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Minimum Lifetime : ", &m_MinLifetime, 0.1f);

				ImGui::Text("Maximum Lifetime: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Maximum Lifetime : ", &m_MaxLifetime, 0.1f);
			}
			else
			{
				ImGui::Text("Lifetime: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Lifetime: ", &m_DefinedLifetime, 0.1f);
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