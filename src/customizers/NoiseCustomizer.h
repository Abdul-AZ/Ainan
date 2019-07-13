#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class Environment;
	class ParticleSystem;
}


namespace ALZ {

	class NoiseCustomizer
	{
	public:
		void DisplayGUI();

	private:
		bool m_NoiseEnabled = false;
		float m_NoiseStrength = 1.0f;

		friend class ParticleSystem;
		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, int objectOrder);
		friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);
	};
}