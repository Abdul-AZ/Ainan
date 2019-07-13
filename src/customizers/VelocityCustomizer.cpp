#include <pch.h>
#include "VelocityCustomizer.h"

namespace ALZ {

	static constexpr const char* LimitTypeToString(VelocityLimitType type) {
		switch (type)
		{
		case VelocityLimitType::NoLimit:
			return "No Limit";
		case VelocityLimitType::NormalLimit:
			return "Limit";
		case VelocityLimitType::PerAxisLimit:
			return "Per Axis Limit";
		default:
			assert(false);
			return "";
		}
	}

	VelocityCustomizer::VelocityCustomizer() :
		mt(std::random_device{}())
	{}

	void VelocityCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Velocity")) {

			ImGui::Text("Starting Velocity");

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


			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::BeginCombo("Velocity Limit", LimitTypeToString(m_LimitType)))
			{
				{
					bool is_active = m_LimitType == VelocityLimitType::NoLimit;
					if (ImGui::Selectable(LimitTypeToString(VelocityLimitType::NoLimit), &is_active)) {
						ImGui::SetItemDefaultFocus();
						m_LimitType = VelocityLimitType::NoLimit;
					}
				}

				{
					bool is_active = m_LimitType == VelocityLimitType::NormalLimit;
					if (ImGui::Selectable(LimitTypeToString(VelocityLimitType::NormalLimit), &is_active)) {
						ImGui::SetItemDefaultFocus();
						m_LimitType = VelocityLimitType::NormalLimit;
					}
				}

				{
					bool is_active = m_LimitType == VelocityLimitType::PerAxisLimit;
					if (ImGui::Selectable(LimitTypeToString(VelocityLimitType::PerAxisLimit), &is_active)) {
						ImGui::SetItemDefaultFocus();
						m_LimitType = VelocityLimitType::PerAxisLimit;
					}
				}

				ImGui::EndCombo();
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