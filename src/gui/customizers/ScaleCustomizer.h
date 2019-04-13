#pragma once

#include "gui/InterpolationSelector.h"

namespace ALZ {

	class ScaleCustomizer
	{
	public:
		ScaleCustomizer();
		void DisplayGUI();

		InterpolationSelector<float>& GetScaleInterpolator();

	private:
		//starting scale
		bool RandomScale = true;
		float definedScale = 2.0f;
		float minScale = 1.0f;
		float maxScale = 3.0f;

		//scale over time
		InterpolationSelector<float> m_Interpolator;
		float endScale = definedScale;

		//random number generator
		std::mt19937 mt;
	};
}