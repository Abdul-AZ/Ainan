#pragma once

#include "editor/InterpolationSelector.h"
#include "environment/ExposeToJson.h"

namespace Ainan {

	class ColorCustomizer
	{
	public:
		ColorCustomizer();
		void DisplayGUI();

		//InterpolationSelector<glm::vec4> GetColorInterpolator();

		glm::vec4 StartColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 EndColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	private:
		//scale over time

		//InterpolationSelector<glm::vec4> m_Interpolator;
		InterpolationType m_InterpolationType = InterpolationType::Fixed;

		//random number generator
		std::mt19937 mt;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}