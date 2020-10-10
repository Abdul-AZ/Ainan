#pragma once

#include <glm/glm.hpp>
#include "editor/Window.h"
#include "renderer/Rectangle.h"

namespace Ainan {

	enum class CameraMode
	{
		CentreIsMidPoint,
		CentreIsBottomLeft
	};

	const float c_CameraZoomFactorDefault = 1000.0f;
	const float c_CameraZoomFactorMin = 100.0f;
	const float c_CameraZoomFactorMax = 2000.0f;

	class Camera
	{
	public:
		Camera(CameraMode mode = CameraMode::CentreIsMidPoint);

		//this only sets the camera size to the screen size
		void Update(float deltaTime, const Rectangle& viewport);
		void SetPosition(const glm::vec2& newPos);

		//returns the position in Normalized Dvice Coordinates relative to the viewport window and NOT the monitor
		glm::vec2 WorldSpaceToViewportNDC(glm::vec2 pos) const;
		glm::vec2 ViewportNDCToWorldSpace(glm::vec2 pos) const;

	public:
		glm::vec2 Position = glm::vec2(0.0f, 0.0f);
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
		float ZoomFactor = c_CameraZoomFactorDefault;

	private:
		CameraMode m_Mode;
	};
}