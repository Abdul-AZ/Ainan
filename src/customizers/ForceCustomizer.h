#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class Environment;
	class ParticleSystem;
}

namespace ALZ {

	class ForceCustomizer
	{
	public:
		void DisplayGUI();

	private:
		std::map<std::string, glm::vec2> m_Forces;

		bool m_EnableGravity = false;
		float m_GravityStrength = -9.81f;


		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, int objectOrder);
		friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);
		friend class ParticleSystem;
	};
}