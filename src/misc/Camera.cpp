#include <pch.h>
#include "Camera.h"

namespace Ainan {

	Camera::Camera(CameraMode mode)
	{
		m_Mode = mode;

		switch (m_Mode)
		{
		case CameraMode::Orthographic:
			glm::vec2& size = Window::FramebufferSize;
			ProjectionMatrix = glm::ortho(0.0f, size.x, 0.0f, size.y);
			ViewMatrix = glm::mat4(1.0f);
			ViewMatrix = glm::translate(ViewMatrix, glm::vec3(Position.x, Position.y, 0.0f));
			break;
		}
	}

	void Camera::Update(float deltaTime, const Rectangle& viewport)
	{
		float aspectRatio = (float)viewport.Width / viewport.Height;
		ProjectionMatrix = glm::ortho(-ZoomFactor * aspectRatio / 2.0f,  //left
									   ZoomFactor * aspectRatio / 2.0f,  //right
									  -ZoomFactor / 2.0f,                //bottom
									   ZoomFactor / 2.0f                 //top
									   );
	}

	void Camera::SetPosition(const glm::vec2& newPos)
	{
		ViewMatrix = glm::mat4(1.0f);
		Position = newPos;
		ViewMatrix = glm::translate(ViewMatrix, glm::vec3(Position.x, Position.y, 0.0f));
	}
}