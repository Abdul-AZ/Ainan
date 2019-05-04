#pragma once

#include <glm/glm.hpp>
#include "Window.h"

namespace ALZ {

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
		void SetPosition(const glm::vec2& newPos);

	public:
		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;

	private:
		CameraMode m_Mode;
	};
}