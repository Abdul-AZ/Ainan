#pragma once


#include "ImGuiWrapper.h"
#include "Interpolation.h"

namespace ALZ {
	struct BezierCurve
	{
		glm::vec2 StartPoint;
		glm::vec2 EndPoint;
		glm::vec2 ControlPoint1;
		glm::vec2 ControlPoint2;
	};

	class CurveEditor
	{
	public:
		CurveEditor() {};
		CurveEditor(const InterpolationType& Type);

		void DisplayInCurrentWindow(const glm::vec2& size = { 300, 200 });
		float CustomCurveFunc(float t);

	private:
		void DrawCustomCurve();
		//returns a pointer to the selected point position
		glm::vec2* DrawControls(float graphYstart);

	public:
		BezierCurve CustomCurve =
		{
			glm::vec2{0.0f ,0.0f} , //startpoint
			glm::vec2{1.0f ,1.0f} , //endpoint
			glm::vec2{0.25f,0.25f}, //controlpoint1
			glm::vec2{0.75f,0.75f}  //controlpoint2
		};
		InterpolationType Type = InterpolationType::Linear;
	private:
		glm::vec2* m_SelectedPoint = nullptr;
		float StartAndEndPointRadius = 7.0f;
		ImU32 StartAndEndPointColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.9f, 0.0f, 0.0f, 1.0f));
		float ControlPointRadius = 4.0f;
		ImU32 ControlPointColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.9f, 1.0f));
	};
}