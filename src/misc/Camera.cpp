#include <pch.h>
#include "Camera.h"

namespace ALZ {

	Camera::Camera(CameraMode mode)
	{
		m_Mode = mode;

		switch (m_Mode)
		{
		case CameraMode::Orthographic:
			glm::vec2& size = Window::WindowSize;
			ProjectionMatrix = glm::ortho(0.0f, size.x, 0.0f, size.y);
			ViewMatrix = glm::mat4(1.0f);
			ViewMatrix = glm::translate(ViewMatrix, glm::vec3(Position.x, Position.y, 0.0f));
			break;
		}
	}

	void Camera::Update(const float & deltaTime)
	{
		if (Window::WindowSizeChangedSinceLastFrame())
		{
			float aspectRatio = Window::WindowSize.x / Window::WindowSize.y;
			ProjectionMatrix = glm::ortho(0.0f, GlobalScaleFactor * aspectRatio, 0.0f, GlobalScaleFactor);
		}
	}

	void Camera::SetPosition(const glm::vec2& newPos)
	{
		ViewMatrix = glm::mat4(1.0f);
		Position = newPos;
		ViewMatrix = glm::translate(ViewMatrix, glm::vec3(Position.x, Position.y, 0.0f));
	}
}