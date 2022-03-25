#include "CurveEditor.h"

#include <GLFW/glfw3.h>

namespace Ainan {

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
	
	CurveEditor::CurveEditor(const InterpolationType& type) :
		Type(type)
	{}

	//for using the imgui plotlines function
	static CurveEditor* s_CurrentCurve = nullptr;

	void CurveEditor::DisplayInCurrentWindow(const glm::vec2& size)
	{
		switch (Type)
		{
		case InterpolationType::Fixed:
			break;
		case InterpolationType::Linear:
			ImGui::PlotLines("End Point", linearWrapper, nullptr, 100, 0, nullptr, 3.402823466e+38F, 3.402823466e+38F, { size.x, size.y });
			break;
		case InterpolationType::Cubic:
			ImGui::PlotLines("End Point", cubicWrapper, nullptr, 100, 0, nullptr, 3.402823466e+38F, 3.402823466e+38F, { size.x, size.y });
			break;
		case InterpolationType::Smoothstep:
			ImGui::PlotLines("End Point", smoothStepWrapper, nullptr, 100, 0, nullptr, 3.402823466e+38F, 3.402823466e+38F, { size.x, size.y });
			break;
		case InterpolationType::Custom:
			DrawCustomCurve();
		default:
			break;
		}
	}

	float CurveEditor::Interpolate(float startPoint, float endPoint, float t)
	{
		return startPoint + (endPoint - startPoint) * CustomCurveFunc(t);
	}

	float CurveEditor::CustomCurveFunc(float t)
	{
		 float result = pow((1.0f - t), 3) * CustomCurve.StartPoint.y + 3.0f * pow((1.0f - t), 2) * t * CustomCurve.ControlPoint1.y +
						3.0f * pow((1.0f - t), 2) * pow(t, 2) * CustomCurve.ControlPoint2.y + pow(t, 3) * CustomCurve.EndPoint.y;


		return std::clamp(result, 0.0f, 1.0f);
	}

	void CurveEditor::DrawCustomCurve()
	{
		s_CurrentCurve = this;
		float cursorAtStartY = ImGui::GetCursorPosY();
		ImGui::PlotLines("Custom Curve", [](void* data, int idx) {return s_CurrentCurve->CustomCurveFunc(idx / 100.0f); }, nullptr, 100, 0, nullptr, 0.0f, 1.0f, ImVec2(400, 300));
		s_CurrentCurve = nullptr;

		m_SelectedPoint = DrawControls(cursorAtStartY);

		//move the point to the mouse position
		if (m_SelectedPoint != nullptr) {

			m_SelectedPoint->x = std::clamp((ImGui::GetMousePos().x - (ImGui::GetWindowPos().x + ImGui::GetCursorPosX())) / 400.0f, 0.0f, 1.0f);
			m_SelectedPoint->y = std::clamp(1.0f - ((ImGui::GetMousePos().y - (ImGui::GetWindowPos().y + cursorAtStartY - ImGui::GetScrollY())) / 300.0f), 0.0f, 1.0f);
		}
	}

	//returns a pointer to the selected point position
	glm::vec2* CurveEditor::DrawControls(float graphYstart)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 baseCirclePos = { ImGui::GetCursorPosX() + ImGui::GetWindowPos().x, graphYstart + 300 + ImGui::GetWindowPos().y - ImGui::GetScrollY() };

		ImVec2 startpointPos = { baseCirclePos.x + CustomCurve.StartPoint.x * 400.0f, baseCirclePos.y - CustomCurve.StartPoint.y * 300.0f };

		ImVec2 endpointPos = { baseCirclePos.x + CustomCurve.EndPoint.x * 400.0f, baseCirclePos.y - CustomCurve.EndPoint.y * 300.0f };

		ImVec2 controlpoint1Pos = { baseCirclePos.x + CustomCurve.ControlPoint1.x * 400.0f, baseCirclePos.y - CustomCurve.ControlPoint1.y * 300.0f };

		ImVec2 controlpoint2Pos = { baseCirclePos.x + CustomCurve.ControlPoint2.x * 400.0f, baseCirclePos.y - CustomCurve.ControlPoint2.y * 300.0f };

		//we draw them in this order so that the lines are behind the circles

		//Draw line from startpoint to controlpoint1
		drawList->AddLine(startpointPos, controlpoint1Pos, ControlPointColor);

		//Draw line from startpoint to controlpoint2
		drawList->AddLine(endpointPos, controlpoint2Pos, ControlPointColor);

		bool mouseButtonDown = glfwGetMouseButton(Window::Ptr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		//if the mouse is not pressed returns because no point is selected
		if (!mouseButtonDown)
			m_SelectedPoint = nullptr;

		//check if mouse is over any point and returns a pointer to it's position

		//so it is easier to check distance
		auto distance = [](const ImVec2& vec1, const ImVec2& vec2) {
			return sqrt(pow(vec1.x - vec2.x, 2) + pow(vec1.y - vec2.y, 2));
		};

		if (distance(startpointPos, mousePos) < StartAndEndPointRadius) { // if the mouse is hovering over the start point

			//draw the other points
			drawList->AddCircleFilled(controlpoint1Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(controlpoint2Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(endpointPos, StartAndEndPointRadius, StartAndEndPointColor);

			if (mouseButtonDown) 
			{
				m_SelectedPoint = &CustomCurve.StartPoint;

				//if the point is selected draw it bigger
				drawList->AddCircleFilled(startpointPos, StartAndEndPointRadius * 1.5f, StartAndEndPointSelectedColor); //diffrent color for selected point
			}
			else 
			{
				//if the point is not selected but only hovered, just draw it with a diffrent color
				drawList->AddCircleFilled(startpointPos, StartAndEndPointRadius, StartAndEndPointSelectedColor); //diffrent color for selected point
			}
		}
		else if (distance(controlpoint1Pos, mousePos) < ControlPointRadius) { // if the mouse is hovering over the first control point

			//draw the other points
			drawList->AddCircleFilled(startpointPos, StartAndEndPointRadius, StartAndEndPointColor);
			drawList->AddCircleFilled(controlpoint2Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(endpointPos, StartAndEndPointRadius, StartAndEndPointColor);

			if (mouseButtonDown) 
			{
				m_SelectedPoint = &CustomCurve.ControlPoint1;

				//if the point is selected draw it bigger
				drawList->AddCircleFilled(controlpoint1Pos, ControlPointRadius * 2.0f, ControlPointSelectedColor); //diffrent color for selected point
			}
			else
			{
				//if the point is not selected but only hovered, just draw it with a diffrent color
				drawList->AddCircleFilled(controlpoint1Pos, ControlPointRadius, ControlPointSelectedColor); //diffrent color for selected point
			}
		}
		else if (distance(controlpoint2Pos, mousePos) < ControlPointRadius) { // if the mouse is hovering over the second control point

			//draw the other points
			drawList->AddCircleFilled(startpointPos, StartAndEndPointRadius, StartAndEndPointColor);
			drawList->AddCircleFilled(controlpoint1Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(endpointPos, StartAndEndPointRadius, StartAndEndPointColor);

			if (mouseButtonDown) 
			{
				m_SelectedPoint = &CustomCurve.ControlPoint2;

				//if the point is selected draw it bigger
				drawList->AddCircleFilled(controlpoint2Pos, ControlPointRadius * 2.0f, ControlPointSelectedColor); //diffrent color for selected point
			}
			else 
			{
				//if the point is not selected but only hovered, just draw it with a diffrent color
				drawList->AddCircleFilled(controlpoint2Pos, ControlPointRadius, ControlPointSelectedColor); //diffrent color for selected point
			}
		}
		else if (distance(endpointPos, mousePos) < StartAndEndPointRadius) { // if the mouse is hovering over the end point

			//draw the other points
			drawList->AddCircleFilled(startpointPos, StartAndEndPointRadius, StartAndEndPointColor);
			drawList->AddCircleFilled(controlpoint1Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(controlpoint2Pos, ControlPointRadius, ControlPointColor);



			if (mouseButtonDown) {
				m_SelectedPoint = &CustomCurve.EndPoint;

				//if the point is selected draw it bigger
				drawList->AddCircleFilled(endpointPos, StartAndEndPointRadius * 1.5f, StartAndEndPointSelectedColor); //diffrent color for selected point
			}
			else 
			{
				//if the point is not selected but only hovered, just draw it with a diffrent color
				drawList->AddCircleFilled(endpointPos, StartAndEndPointRadius, StartAndEndPointSelectedColor); //diffrent color for selected point
			}
		}
		else { // draw them all with the normal color and size
			drawList->AddCircleFilled(startpointPos, StartAndEndPointRadius, StartAndEndPointColor);
			drawList->AddCircleFilled(controlpoint1Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(controlpoint2Pos, ControlPointRadius, ControlPointColor);
			drawList->AddCircleFilled(endpointPos, StartAndEndPointRadius, StartAndEndPointColor);
		}

		return m_SelectedPoint;
	}
}
