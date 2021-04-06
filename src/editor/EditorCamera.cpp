#include "EditorCamera.h"

namespace Ainan {

	EditorCamera::EditorCamera(ProjectionMode mode) :
		m_Camera(mode, glm::mat4(1.0f), 16.0f / 9.0f, 0.01f, 1000.0f, c_CameraZoomFactorDefault)
	{
	}

	void EditorCamera::Update(float deltaTime, const Rectangle& viewport)
	{
		m_Camera.SetAspectRatio((float)viewport.Width / viewport.Height);
	}

	void EditorCamera::CalculateViewMatrix()
	{
		m_Camera.SetViewMatrix(glm::lookAt(Position, Position + CameraForward, CameraUp));
	}

	glm::vec2 EditorCamera::WorldSpaceToViewportNDC(glm::vec2 pos) const
	{
		glm::vec4 result = m_Camera.GetProjectionMatrix() * m_Camera.GetViewMatrix() * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);

		return glm::vec2(result.x, result.y);
	}

	glm::vec2 EditorCamera::ViewportNDCToWorldSpace(glm::vec2 pos) const
	{
		glm::mat4 invView = glm::inverse(m_Camera.GetViewMatrix());
		glm::mat4 invProj = glm::inverse(m_Camera.GetProjectionMatrix());

		glm::vec4 result = invView * invProj * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);


		return glm::vec2(result.x, result.y);
	}
}