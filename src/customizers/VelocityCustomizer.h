#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class Environment;
	class ParticleSystem;
}

namespace ALZ {

	enum class VelocityLimitType {
		NoLimit,
		NormalLimit,
		PerAxisLimit
	};

	class VelocityCustomizer
	{
	public:
		VelocityCustomizer();
		void DisplayGUI();

		glm::vec2 GetVelocity();

	private:
		bool m_RandomVelocity = true;
		glm::vec2 m_MinVelocity = { -100, -100 };
		glm::vec2 m_MaxVelocity = { 100, 100 };
		glm::vec2 m_DefinedVelocity = { 100, -100 };

		VelocityLimitType m_LimitType = VelocityLimitType::NoLimit;

		//random number generator
		std::mt19937 mt;

		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, int objectOrder);
		friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);
	};
}