#pragma once

#include "gui/InterpolationSelector.h"

namespace ALZ {

	class ColorCustomizer
	{
	public:
		ColorCustomizer();
		void DisplayGUI();

		InterpolationSelector<glm::vec4>& GetColorInterpolator();

	private:
		glm::vec4 definedColor = { 1.0f,1.0f,1.0f,1.0f };

		//scale over time
		InterpolationSelector<glm::vec4> m_Interpolator;
		glm::vec4 endColor = definedColor;

		//random number generator
		std::mt19937 mt;
	};
}