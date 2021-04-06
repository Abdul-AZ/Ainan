#pragma once

#include "editor/Window.h"
#include "renderer/Rectangle.h"
#include "renderer/Camera.h"

namespace Ainan {

	const float c_CameraZoomFactorDefault = 10.0f;
	const float c_CameraZoomFactorMin = 1.0f;
	const float c_CameraZoomFactorMax = 100.0f;
	const float c_CameraOrthoMoveSpeedFactor = 0.5f;
	const float c_CameraPerspMoveSpeedFactor = 10.0f;
	const float c_CameraNearClip = 0.1f;
	const float c_CameraFarClip = 1000.0f;
	const glm::vec3 c_CameraStartingPosition = glm::vec3(0.0f, 0.0f, -10.0f);
	const glm::vec3 c_CameraStartingForwardDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	class EditorCamera
	{
	public:
		EditorCamera(ProjectionMode mode = ProjectionMode::Orthographic);

		//this only sets the camera size to the screen size
		void Update(float deltaTime, const Rectangle& viewport);

		void CalculateViewMatrix();

		//returns the position in Normalized Dvice Coordinates relative to the viewport window and NOT the monitor
		glm::vec2 WorldSpaceToViewportNDC(glm::vec2 pos) const;
		glm::vec2 ViewportNDCToWorldSpace(glm::vec2 pos) const;

	public:
		Camera m_Camera;
		glm::vec3 Position = c_CameraStartingPosition;
		glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 CameraForward = c_CameraStartingForwardDirection;
		float CameraPitch = 0;
		float CameraYaw = 90.0f;
	};
}