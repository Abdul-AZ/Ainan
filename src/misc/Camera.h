#pragma once

#include <glm/glm.hpp>
#include "Window.h"
#include "renderer/Rectangle.h"

namespace Ainan {

	enum class CameraMode
	{
		Orthographic,
		Perspective //NOTE use this when we add 3D stuff
	};

	class Camera
	{
	public:
		Camera(CameraMode mode = CameraMode::Orthographic);

		//this only sets the camera size to the screen size
		void Update(float deltaTime, const Rectangle& viewport);
		void SetPosition(const glm::vec2& newPos);

	public:
		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;

	private:
		CameraMode m_Mode;
	};
}