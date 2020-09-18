#include "VelocityCustomizer.h"

namespace Ainan {

	std::string LimitTypeToString(VelocityCustomizer::VelocityLimitType type) 
	{
		switch (type)
		{
		case VelocityCustomizer::NoLimit:
			return "No Limit";
		case VelocityCustomizer::NormalLimit:
			return "Limit";
		case VelocityCustomizer::PerAxisLimit:
			return "Per Axis Limit";
		default:
			assert(false);
			return "";
		}
	}

	VelocityCustomizer::VelocityLimitType StringToLimitType(const std::string& type) 
	{

		if (type == "No Limit")
			return VelocityCustomizer::NoLimit;
		else if (type == "Limit")
			return VelocityCustomizer::NormalLimit;
		else if (type == "Per Axis Limit")
			return VelocityCustomizer::PerAxisLimit;

		assert(false);
		return VelocityCustomizer::NoLimit;
	}

	VelocityCustomizer::VelocityCustomizer() :
		mt(std::random_device{}())
	{}

	void VelocityCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		if (ImGui::TreeNode("Velocity"))
		{

			ImGui::Text("Starting Velocity");

			ImGui::Text("Random Between 2 Numbers");
			ImGui::SameLine();
			ImGui::Checkbox("##Random Between 2 Numbers", &m_RandomVelocity);

			if (m_RandomVelocity)
			{
				if (m_MaxVelocity.x < m_MinVelocity.x)
					m_MaxVelocity.x = m_MinVelocity.x;

				if (m_MaxVelocity.y < m_MinVelocity.y)
					m_MaxVelocity.y = m_MinVelocity.y;

				ImGui::Text("Minimum Velocity: ");
				ImGui::SameLine();
				float xPos = ImGui::GetCursorPosX();
				ImGui::DragFloat2("##Minimum Velocity : ", &m_MinVelocity.x);

				ImGui::Text("Maximum Velocity: ");
				ImGui::SameLine();
				ImGui::SetCursorPosX(xPos);
				ImGui::DragFloat2("##Maximum Velocity : ", &m_MaxVelocity.x);
			}
			else
			{
				ImGui::Text("Velocity: ");
				ImGui::SameLine();
				ImGui::DragFloat2("##Velocity: ", &m_DefinedVelocity.x);
			}


			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Velocity Limit");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Velocity Limit", LimitTypeToString(CurrentVelocityLimitType).c_str()))
			{
				{
					bool is_active = CurrentVelocityLimitType == NoLimit;
					if (ImGui::Selectable(LimitTypeToString(NoLimit).c_str(), &is_active)) {
						ImGui::SetItemDefaultFocus();
						CurrentVelocityLimitType = NoLimit;
					}
				}

				{
					bool is_active = CurrentVelocityLimitType == NormalLimit;
					if (ImGui::Selectable(LimitTypeToString(NormalLimit).c_str(), &is_active)) {
						ImGui::SetItemDefaultFocus();
						CurrentVelocityLimitType = NormalLimit;
					}
				}

				{
					bool is_active = CurrentVelocityLimitType == PerAxisLimit;
					if (ImGui::Selectable(LimitTypeToString(PerAxisLimit).c_str(), &is_active)) {
						ImGui::SetItemDefaultFocus();
						CurrentVelocityLimitType = PerAxisLimit;
					}
				}

				ImGui::EndCombo();
			}

			if (CurrentVelocityLimitType == NormalLimit)
			{
				ImGui::Text("Minimum Velocity\n Length: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Minimum Velocity\n Length: ", &m_MinNormalVelocityLimit);

				ImGui::Text("Maximum Velocity\n Length: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Maximum Velocity\n Length: ", &m_MaxNormalVelocityLimit);

				//clamp eveything so that the maximum is always bigger than the minimum and the opposite
				m_MaxNormalVelocityLimit = std::clamp(m_MaxNormalVelocityLimit, m_MinNormalVelocityLimit, 100000.0f);
				m_MinNormalVelocityLimit = std::clamp(m_MinNormalVelocityLimit, 0.0f, m_MaxNormalVelocityLimit);
			}
			else if (CurrentVelocityLimitType == PerAxisLimit)
			{
				ImGui::Text("Minimum Velocity: ");
				ImGui::SameLine();
				ImGui::DragFloat2("##Minimum Velocity: ", &m_MinPerAxisVelocityLimit.x);

				ImGui::Text("Maximum Velocity: ");
				ImGui::SameLine();
				ImGui::DragFloat2("##Maximum Velocity: ", &m_MaxPerAxisVelocityLimit.x);

				//clamp eveything so that the maximum is always bigger than the minimum and the opposite
				m_MinPerAxisVelocityLimit.x = std::clamp(m_MinPerAxisVelocityLimit.x, -100000.0f, m_MaxPerAxisVelocityLimit.x);
				m_MinPerAxisVelocityLimit.y = std::clamp(m_MinPerAxisVelocityLimit.y, -100000.0f, m_MaxPerAxisVelocityLimit.y);
				m_MaxPerAxisVelocityLimit.x = std::clamp(m_MaxPerAxisVelocityLimit.x, m_MinPerAxisVelocityLimit.x, 100000.0f);
				m_MaxPerAxisVelocityLimit.y = std::clamp(m_MaxPerAxisVelocityLimit.y, m_MinPerAxisVelocityLimit.y, 100000.0f);
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