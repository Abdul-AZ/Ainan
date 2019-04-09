#pragma once

#include <glm/glm.hpp>
#include "Window.h"

enum class CameraMode 
{
	Orthographic,
	Perspective //NOTE use this when we add 3D stuff
};

class Camera 
{
public:
	Camera(CameraMode mode = CameraMode::Orthographic);

	void Update(const float& deltaTime);

	void SetPosition(const glm::vec3& newPos);
	glm::mat4& GetViewMatrix() { return m_ViewMatrix; }
	glm::mat4& GetProjectionMatrix() { return m_ProjectionMatrix; }

public:
	glm::vec3 Position = { 0, 0, 0 };

private:
	CameraMode m_Mode;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ProjectionMatrix;
};