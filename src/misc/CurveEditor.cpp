#include <pch.h>

#include "CurveEditor.h"

namespace ALZ {

	static float smoothStepWrapper(void* data, int idx)
	{
		float t = idx / 100.0f;

		return Interpolation::Smoothstep<float>(0.0f, 1.0f, t);
	}

	static float linearWrapper(void* data, int idx)
	{
		float t = idx / 100.0f;

		return Interpolation::Linear<float>(0.0f, 1.0f, t);
	}

	static float cubicWrapper(void* data, int idx)
	{
		float t = idx / 100.0f;

		return Interpolation::Cubic<float>(0.0f, 1.0f, t);
	}
	
	CurveEditor::CurveEditor() :
		Type(InterpolationType::Linear)
	{}

	CurveEditor::CurveEditor(const InterpolationType& type) :
		Type(type)
	{}

	void CurveEditor::DisplayInCurrentWindow(const glm::vec2& size)
	{
		switch (Type)
		{
		case ALZ::Fixed:
			break;
		case ALZ::Linear:
			ImGui::PlotLines("End Point", linearWrapper, nullptr, 100, 0, nullptr, 3.402823466e+38F, 3.402823466e+38F, { size.x, size.y });
			break;
		case ALZ::Cubic:
			ImGui::PlotLines("End Point", cubicWrapper, nullptr, 100, 0, nullptr, 3.402823466e+38F, 3.402823466e+38F, { size.x, size.y });
			break;
		case ALZ::Smoothstep:
			ImGui::PlotLines("End Point", smoothStepWrapper, nullptr, 100, 0, nullptr, 3.402823466e+38F, 3.402823466e+38F, { size.x, size.y });
			break;
		default:
			break;
		}
	}
}