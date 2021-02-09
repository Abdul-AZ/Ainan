#include "Camera.h"

namespace Ainan {

	Camera::Camera(ProjectionMode mode) :
		Mode(mode), 
		ViewMatrix(glm::mat4(1.0f))
	{
		SetPosition(Position);
	}

	void Camera::Update(float deltaTime, const Rectangle& viewport)
	{
		float aspectRatio = (float)viewport.Width / viewport.Height;

		switch (Mode)
		{
		case ProjectionMode::Orthographic:
			ProjectionMatrix = glm::ortho(
				-ZoomFactor * aspectRatio / 2.0f,  //left
				 ZoomFactor * aspectRatio / 2.0f,  //right
				-ZoomFactor / 2.0f,                //bottom
				 ZoomFactor / 2.0f,                //top
				 c_CameraNearClip,
				 c_CameraFarClip
				 );
			break;

		case ProjectionMode::Perspective:
			ProjectionMatrix = glm::perspective(glm::radians((float)60), aspectRatio, c_CameraNearClip, c_CameraFarClip);
			break;
		}
	}

	void Camera::SetPosition(const glm::vec3& newPos)
	{
		Position = newPos;
		CalculateMatrices();
	}

	void Camera::CalculateMatrices()
	{
		ViewMatrix = glm::lookAt(Position, Position + CameraForward, CameraUp);
	}

	glm::vec2 Camera::WorldSpaceToViewportNDC(glm::vec2 pos) const
	{
		glm::vec4 result = ProjectionMatrix * ViewMatrix * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);

		return glm::vec2(result.x, result.y);
	}

	glm::vec2 Camera::ViewportNDCToWorldSpace(glm::vec2 pos) const
	{
		glm::mat4 invView = glm::inverse(ViewMatrix);
		glm::mat4 invProj = glm::inverse(ProjectionMatrix);

		glm::vec4 result = invView * invProj * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);


		return glm::vec2(result.x, result.y);
	}
}