#pragma once

#include "editor/InterpolationSelector.h"
#include "editor/CurveEditor.h"
#include "environment/ExposeToJson.h"

namespace Ainan {

	class ScaleCustomizer
	{
	public:
		void DisplayGUI();

		//InterpolationSelector<float> GetScaleInterpolator();
		CurveEditor m_Curve;

		//starting scale
		bool m_RandomScale = true;
		float m_DefinedScale = 2.0f;
		float m_MinScale = 20.0f;
		float m_MaxScale = 25.0f;

		InterpolationType m_InterpolationType = InterpolationType::Linear;
		float m_EndScale = m_DefinedScale;

	private:

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}