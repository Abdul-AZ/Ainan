#pragma once

#include "misc/InterpolationSelector.h"
#include "misc/ExposeToJson.h"

namespace Ainan {

	class ColorCustomizer
	{
	public:
		ColorCustomizer();
		void DisplayGUI();

		InterpolationSelector<glm::vec4> GetColorInterpolator();

	private:
		glm::vec4 m_DefinedColor = { 1.0f,1.0f,1.0f,1.0f };

		//scale over time
		InterpolationSelector<glm::vec4> m_Interpolator;
		glm::vec4 m_EndColor = m_DefinedColor;

		//random number generator
		std::mt19937 mt;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}