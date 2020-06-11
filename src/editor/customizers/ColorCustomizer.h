#pragma once

#include "editor/InterpolationSelector.h"
#include "environment/ExposeToJson.h"

namespace Ainan {

	class ColorCustomizer
	{
	public:
		void DisplayGUI();

		glm::vec4 StartColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 EndColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

	private:
		//scale over time
		InterpolationType m_InterpolationType = InterpolationType::Fixed;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}