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

		glm::vec3 GetVelocity();

		VelocityLimitType CurrentVelocityLimitType = NoLimit;
	private:

		//starting velocity data
		bool m_RandomVelocity = true;
		glm::vec3 m_MinVelocity = { -1, -1, -1 };
		glm::vec3 m_MaxVelocity = { 1, 1, 1 };
		glm::vec3 m_DefinedVelocity = { 1, -1, 0 };
		//-------------------------------------

		//velocity limit data
		float m_MinNormalVelocityLimit = 0.1f;
		float m_MaxNormalVelocityLimit = 20.0f;

		glm::vec3 m_MinPerAxisVelocityLimit = { -1.0f, -1.0f, -1.0f };
		glm::vec3 m_MaxPerAxisVelocityLimit = { 1.0f, 1.0f, 1.0f };
		//-------------------------------------

		//random number generator
		std::mt19937 mt;

		EXPOSE_CUSTOMIZER_TO_JSON
	};

	std::string LimitTypeToString(VelocityCustomizer::VelocityLimitType type);
	VelocityCustomizer::VelocityLimitType StringToLimitType(const std::string& type);
}