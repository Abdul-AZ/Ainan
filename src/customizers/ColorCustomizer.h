#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class ParticleSystem;
}

#include "InterpolationSelector.h"

namespace ALZ {

	class ColorCustomizer
	{
	public:
		ColorCustomizer();
		void DisplayGUI();

		InterpolationSelector<glm::vec4>& GetColorInterpolator();

	private:
		glm::vec4 m_DefinedColor = { 1.0f,1.0f,1.0f,1.0f };

		//scale over time
		InterpolationSelector<glm::vec4> m_Interpolator;
		glm::vec4 m_EndColor = m_DefinedColor;

		//random number generator
		std::mt19937 mt;

		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, const int& objectOrder);
	};
}