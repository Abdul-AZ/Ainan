#include "Camera.h"

namespace Ainan {

	Camera::Camera(CameraMode mode) :
		m_Mode(mode), 
		ViewMatrix(glm::mat4(1.0f))
	{
		SetPosition(Position);
	}

	void Camera::Update(float deltaTime, const Rectangle& viewport)
	{
		float aspectRatio = (float)viewport.Width / viewport.Height;

		switch (m_Mode)
		{
		case CameraMode::CentreIsMidPoint:
			ProjectionMatrix = glm::ortho(
				-ZoomFactor * aspectRatio / 2.0f,  //left
				 ZoomFactor * aspectRatio / 2.0f,  //right
				-ZoomFactor / 2.0f,                //bottom
				 ZoomFactor / 2.0f,                //top
				 c_CameraNearClip,
				 c_CameraFarClip
				 );
			break;

		case CameraMode::CentreIsBottomLeft:
			ProjectionMatrix = glm::ortho(
				0.0f,							   //left
				ZoomFactor * aspectRatio,		   //right
				0.0f,							   //bottom
				ZoomFactor,  					   //top
				c_CameraFarClip,
				c_CameraFarClip
				);
			break;
		}
	}

	void Camera::SetPosition(const glm::vec3& newPos)
	{
		Position = newPos;
		ViewMatrix = glm::translate(glm::mat4(1.0f), -Position);
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