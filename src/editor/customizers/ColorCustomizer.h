#pragma once

#include "editor/InterpolationSelector.h"
#include "environment/ExposeToJson.h"

namespace Ainan {

	class ColorCustomizer
	{
	public:
		ColorCustomizer();
		void DisplayGUI();

		InterpolationSelector<glm::vec4> GetColorInterpolator();

	private:
		//scale over time
		InterpolationSelector<glm::vec4> m_Interpolator;

		//random number generator
		std::mt19937 mt;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}