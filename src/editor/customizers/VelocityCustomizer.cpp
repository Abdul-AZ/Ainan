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
			AINAN_LOG_ERROR("Invalid limit enum");
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

		AINAN_LOG_ERROR("Invalid limit string");
		return VelocityCustomizer::NoLimit;
	}

	VelocityCustomizer::VelocityCustomizer() :
		mt(std::random_device{}())
	{}

	void VelocityCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Velocity"))
		{
			ImGui::Text("Starting Velocity");
			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Random Between 2 Numbers");
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			ImGui::Checkbox("##Random Between 2 Numbers", &m_RandomVelocity);
			ImGui::NextColumn();

			if (m_RandomVelocity)
			{
				if (m_MaxVelocity.x < m_MinVelocity.x)
					m_MaxVelocity.x = m_MinVelocity.x;

				if (m_MaxVelocity.y < m_MinVelocity.y)
					m_MaxVelocity.y = m_MinVelocity.y;

				if (m_MaxVelocity.z < m_MinVelocity.z)
					m_MaxVelocity.z = m_MinVelocity.z;

				ImGui::Text("Minimum Velocity: ");
				ImGui::NextColumn();
				float xPos = ImGui::GetCursorPosX();
				ImGui::DragFloat3("##Minimum Velocity : ", &m_MinVelocity.x);
				ImGui::NextColumn();
				ImGui::Text("Maximum Velocity: ");
				ImGui::NextColumn();
				ImGui::DragFloat3("##Maximum Velocity : ", &m_MaxVelocity.x);
				ImGui::NextColumn();
			}
			else
			{
				ImGui::Text("Velocity: ");
				ImGui::NextColumn();
				ImGui::DragFloat3("##Velocity: ", &m_DefinedVelocity.x);
				ImGui::NextColumn();
			}

			IMGUI_DROPDOWN_START_USING_COLUMNS("Velocity Limit", LimitTypeToString(CurrentVelocityLimitType).c_str());
			IMGUI_DROPDOWN_SELECTABLE(CurrentVelocityLimitType, NoLimit, LimitTypeToString(NoLimit).c_str());
			IMGUI_DROPDOWN_SELECTABLE(CurrentVelocityLimitType, NormalLimit, LimitTypeToString(NormalLimit).c_str());
			IMGUI_DROPDOWN_SELECTABLE(CurrentVelocityLimitType, PerAxisLimit, LimitTypeToString(PerAxisLimit).c_str());
			IMGUI_DROPDOWN_END();

			if (CurrentVelocityLimitType == NormalLimit)
			{
				ImGui::NextColumn();
				ImGui::Text("Minimum Velocity\n Magnitude: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Minimum Velocity\n Length: ", &m_MinNormalVelocityLimit);

				ImGui::NextColumn();
				ImGui::Text("Maximum Velocity\n Magnitude: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Maximum Velocity\n Length: ", &m_MaxNormalVelocityLimit);

				//clamp eveything so that the maximum is always bigger than the minimum and the opposite
				m_MaxNormalVelocityLimit = std::clamp(m_MaxNormalVelocityLimit, m_MinNormalVelocityLimit, 100000.0f);
				m_MinNormalVelocityLimit = std::clamp(m_MinNormalVelocityLimit, 0.0f, m_MaxNormalVelocityLimit);
			}
			else if (CurrentVelocityLimitType == PerAxisLimit)
			{
				ImGui::NextColumn();
				ImGui::Text("Minimum Velocity: ");
				ImGui::NextColumn();
				ImGui::DragFloat3("##Minimum Velocity: ", &m_MinPerAxisVelocityLimit.x);

				ImGui::NextColumn();
				ImGui::Text("Maximum Velocity: ");
				ImGui::NextColumn();
				ImGui::DragFloat3("##Maximum Velocity: ", &m_MaxPerAxisVelocityLimit.x);

				//clamp eveything so that the maximum is always bigger than the minimum and the opposite
				m_MinPerAxisVelocityLimit.x = std::clamp(m_MinPerAxisVelocityLimit.x, -100000.0f, m_MaxPerAxisVelocityLimit.x);
				m_MinPerAxisVelocityLimit.y = std::clamp(m_MinPerAxisVelocityLimit.y, -100000.0f, m_MaxPerAxisVelocityLimit.y);
				m_MaxPerAxisVelocityLimit.x = std::clamp(m_MaxPerAxisVelocityLimit.x, m_MinPerAxisVelocityLimit.x, 100000.0f);
				m_MaxPerAxisVelocityLimit.y = std::clamp(m_MaxPerAxisVelocityLimit.y, m_MinPerAxisVelocityLimit.y, 100000.0f);
			}

			ImGui::NextColumn();
			ImGui::TreePop();
		}
	}

	glm::vec3 VelocityCustomizer::GetVelocity()
	{
		if (m_RandomVelocity)
		{
			if (m_MinVelocity.x > m_MaxVelocity.x)
				m_MinVelocity.x = m_MaxVelocity.x;
			if (m_MinVelocity.y > m_MaxVelocity.y)
				m_MinVelocity.y = m_MaxVelocity.y;
			if (m_MinVelocity.z > m_MaxVelocity.z)
				m_MinVelocity.z = m_MaxVelocity.z;

			std::uniform_real_distribution<float> dist_velocity_x(m_MinVelocity.x, m_MaxVelocity.x);
			std::uniform_real_distribution<float> dist_velocity_y(m_MinVelocity.y, m_MaxVelocity.y);
			std::uniform_real_distribution<float> dist_velocity_z(m_MinVelocity.z, m_MaxVelocity.z);
			return glm::vec3(dist_velocity_x(mt), dist_velocity_y(mt), dist_velocity_z(mt));
		}
		else
			return m_DefinedVelocity;
	}
}