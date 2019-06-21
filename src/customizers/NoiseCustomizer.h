#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
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
		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, const int& objectOrder);
	};
}