#pragma once

#include <glm/glm.hpp>
#include "editor/Window.h"
#include "renderer/Rectangle.h"

namespace Ainan {

	enum class ProjectionMode : int32_t
	{
		Orthographic = 0,
		Perspective = 1
	};

	const float c_CameraZoomFactorDefault = 10.0f;
	const float c_CameraZoomFactorMin = 1.0f;
	const float c_CameraZoomFactorMax = 100.0f;
	const float c_CameraOrthoMoveSpeedFactor = 0.5f;
	const float c_CameraPerspMoveSpeedFactor = 10.0f;
	const float c_CameraNearClip = 0.1f;
	const float c_CameraFarClip = 1000.0f;
	const glm::vec3 c_CameraStartingPosition = glm::vec3(0.0f, 0.0f, -10.0f);
	const glm::vec3 c_CameraStartingForwardDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	class Camera
	{
	public:
		Camera(ProjectionMode mode = ProjectionMode::Orthographic);

		//this only sets the camera size to the screen size
		void Update(float deltaTime, const Rectangle& viewport);
		void SetPosition(const glm::vec3& newPos);
		void CalculateMatrices();

		//returns the position in Normalized Dvice Coordinates relative to the viewport window and NOT the monitor
		glm::vec2 WorldSpaceToViewportNDC(glm::vec2 pos) const;
		glm::vec2 ViewportNDCToWorldSpace(glm::vec2 pos) const;

	public:
		glm::vec3 Position = c_CameraStartingPosition;
		glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 CameraForward = c_CameraStartingForwardDirection;
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
		float ZoomFactor = c_CameraZoomFactorDefault;
		ProjectionMode Mode;
	};
}