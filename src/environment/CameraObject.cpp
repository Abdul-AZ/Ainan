#include "CameraObject.h"

namespace Ainan {

	CameraObject::CameraObject()
	{
		Type = CameraType;
	}

	void CameraObject::DisplayGuiControls()
	{
		DisplayTransformationControls();

		ImGui::NextColumn();

		auto projMode = m_Camera.GetProjectionMode();
		IMGUI_DROPDOWN_START_USING_COLUMNS("Projection Mode: ", ProjectionModeToStr(projMode));
		IMGUI_DROPDOWN_SELECTABLE(projMode, ProjectionMode::Perspective, ProjectionModeToStr(ProjectionMode::Perspective));
		IMGUI_DROPDOWN_SELECTABLE(projMode, ProjectionMode::Orthographic, ProjectionModeToStr(ProjectionMode::Orthographic));
		IMGUI_DROPDOWN_END();
		if (projMode != m_Camera.GetProjectionMode())
		{
			if (projMode == ProjectionMode::Perspective)
				m_Camera.SetPersp();
			else
				m_Camera.SetOrtho();
		}
	}

	void CameraObject::OnTransform()
	{
		m_Camera.SetViewMatrix(glm::inverse(ModelMatrix));
	}
}