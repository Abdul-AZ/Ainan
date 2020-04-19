#pragma once

#include "environment/ExposeToJson.h"

namespace Ainan {

	class VelocityCustomizer
	{
	public:
		enum VelocityLimitType {
			NoLimit,
			NormalLimit,
			PerAxisLimit
		};

	public:
		VelocityCustomizer();
		void DisplayGUI();

		glm::vec2 GetVelocity();

		VelocityLimitType CurrentVelocityLimitType = NoLimit;
	private:

		//starting velocity data
		bool m_RandomVelocity = true;
		glm::vec2 m_MinVelocity = { -100, -100 };
		glm::vec2 m_MaxVelocity = { 100, 100 };
		glm::vec2 m_DefinedVelocity = { 100, -100 };
		//-------------------------------------

		//velocity limit data
		float m_MinNormalVelocityLimit = 10.0f;
		float m_MaxNormalVelocityLimit = 200.0f;

		//DO NOT change the order of these, because we use ImGui DragFloat2 to display them
		glm::vec2 m_MinPerAxisVelocityLimit = { -100.0f, -100.0f };
		glm::vec2 m_MaxPerAxisVelocityLimit = { 100.0f, 100.0f };
		//-------------------------------------

		//random number generator
		std::mt19937 mt;

		EXPOSE_CUSTOMIZER_TO_JSON
	};

	std::string LimitTypeToString(VelocityCustomizer::VelocityLimitType type);
	VelocityCustomizer::VelocityLimitType StringToLimitType(const std::string& type);
}