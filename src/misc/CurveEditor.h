#pragma once


#include "ImGuiWrapper.h"
#include "Interpolation.h"

namespace ALZ {

	class CurveEditor
	{
	public:
		CurveEditor();
		CurveEditor(const InterpolationType& Type);

		void DisplayInCurrentWindow(const glm::vec2& size = { 300, 200 });

	public:
		InterpolationType Type;
	};

}