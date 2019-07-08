#include <pch.h>

#include "CurveEditor.h"

#include <GLFW/glfw3.h>

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
	{
		ControlPoints.push_back({ 0.0f, 0.0f});
		ControlPoints.push_back({ 0.5f, 0.95f});
		ControlPoints.push_back({ 1.0f, 1.0f });
	}

	CurveEditor::CurveEditor(const InterpolationType& type) :
		Type(type)
	{}


	static CurveEditor* s_CurrentCurve = nullptr;

	static int offset = 0;
	static int count = 100;
	static float t_max = 0.0f;

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
		
		s_CurrentCurve = this;
		int cursorAtStartY = ImGui::GetCursorPosY();
		ImGui::PlotLines("Custom Curve", [](void* data, int idx) {return s_CurrentCurve->CustomCurveFunc(idx / 100.0f); }, nullptr, count, offset, nullptr, 3.402823466e+38F, 3.402823466e+38F, ImVec2(400, 300));
		s_CurrentCurve = nullptr;

		bool mouseButtonDown = glfwGetMouseButton(&Window::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		float radius = 10.0f;

		for (size_t i = 0; i < ControlPoints.size(); i++)
		{
			ImVec2 circlePos = { ImGui::GetCursorPosX() + (float)ControlPoints[i].x * 400 + ImGui::GetWindowPos().x, cursorAtStartY + 300 - ControlPoints[i].y * 300 + ImGui::GetWindowPos().y - ImGui::GetScrollY() };
			ImVec2 mousePos = ImGui::GetMousePos();
			ImGui::GetWindowDrawList()->AddCircle(circlePos, radius, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), 12, 1.0f);

			bool mouseInCircle = sqrt(pow(mousePos.x - circlePos.x, 2) + pow(mousePos.y - circlePos.y, 2)) < radius * radius;

			if (mouseInCircle && mouseButtonDown)
					m_FollowingPoint = i;
		}

		if (m_FollowingPoint != -1) {
			ControlPoints[m_FollowingPoint].x = std::clamp((ImGui::GetMousePos().x - (ImGui::GetWindowPos().x + ImGui::GetCursorPosX())) / 400.0f, 0.0f, 1.0f);
			ControlPoints[m_FollowingPoint].y = std::clamp(1.0f - ((ImGui::GetMousePos().y  - (ImGui::GetWindowPos().y + cursorAtStartY - ImGui::GetScrollY())) / 300.0f), 0.0f, 1.0f);
		}

		if (!mouseButtonDown)
			m_FollowingPoint = -1;
	}


	//these two functions will be changed
	static int Factorial(int x) {
		if (x > 1)
			return x * Factorial(x - 1);
		else
			return 1;
	}

	static int BinomialCoefficiant(int n, int k)
	{
		return Factorial(n) / (Factorial(k) * Factorial(n - k));
	}

	float CurveEditor::CustomCurveFunc(float t)
	{
		glm::vec2 result = { 0.0f, 0.0f };

		int n = ControlPoints.size() - 1;

		for (size_t i = 0; i <= n; i++)
		{
			result += (float)BinomialCoefficiant(n, i) * (float)pow(1.0f - t, n - i) * (float) pow(t, i) * ControlPoints[i];
		}

		return std::clamp(result.y, 0.0f, 1.0f);
	}
}