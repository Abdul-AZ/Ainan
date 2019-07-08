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
		float CustomCurveFunc(float t);
	public:
		std::vector<glm::vec2> ControlPoints;
		InterpolationType Type;
	private:
		int m_FollowingPoint = -1.0f;
	};

}