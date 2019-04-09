#include <pch.h>
#include "Camera.h"

Camera::Camera(CameraMode mode)
{
	m_Mode = mode;

	switch (m_Mode)
	{
	case CameraMode::Orthographic:
		glm::vec2& size = Window::GetSize();
		m_ProjectionMatrix = glm::ortho(0.0f, size.x, size.y, 0.0f);
		m_ViewMatrix = glm::mat4(1.0f);
		m_ViewMatrix = glm::translate(m_ViewMatrix, Position);
		break;
	}
}

void Camera::Update(const float & deltaTime)
{
	if(Window::WindowSizeChangedSinceLastFrame())
	{
		glm::vec2& size = Window::GetSize();
		m_ProjectionMatrix = glm::ortho(0.0f, size.x, size.y, 0.0f);
	}
}

void Camera::SetPosition(const glm::vec3 & newPos)
{
	m_ViewMatrix = glm::mat4(1.0f);
	Position = newPos;
	m_ViewMatrix = glm::translate(m_ViewMatrix, Position);
}
