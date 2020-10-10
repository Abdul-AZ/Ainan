#include "Camera.h"

namespace Ainan {

	Camera::Camera(CameraMode mode) :
		m_Mode(mode), 
		ViewMatrix(glm::mat4(1.0f))
	{}

	void Camera::Update(float deltaTime, const Rectangle& viewport)
	{
		float aspectRatio = (float)viewport.Width / viewport.Height;

		switch (m_Mode)
		{
		case Ainan::CameraMode::CentreIsMidPoint:
			ProjectionMatrix = glm::ortho(
				-ZoomFactor * aspectRatio / 2.0f,  //left
				 ZoomFactor * aspectRatio / 2.0f,  //right
				-ZoomFactor / 2.0f,                //bottom
				 ZoomFactor / 2.0f                 //top
				);
			break;

		case Ainan::CameraMode::CentreIsBottomLeft:
			ProjectionMatrix = glm::ortho(
				0.0f,							   //left
				ZoomFactor * aspectRatio,		   //right
				0.0f,							   //bottom
				ZoomFactor);					   //top
			break;
		}
	}

	void Camera::SetPosition(const glm::vec2& newPos)
	{
		Position = newPos;
		ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, 0.0f));
	}

	glm::vec2 Camera::WorldSpaceToViewportNDC(glm::vec2 pos) const
	{
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, 0.0f) / c_GlobalScaleFactor);
		glm::vec4 result = ProjectionMatrix * view * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);

		return glm::vec2(result.x, result.y) * c_GlobalScaleFactor;
	}

	glm::vec2 Camera::ViewportNDCToWorldSpace(glm::vec2 pos) const
	{
		glm::mat4 invView = glm::inverse(ViewMatrix);
		glm::mat4 invProj = glm::inverse(ProjectionMatrix);

		glm::vec4 result = invView * invProj * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);


		return glm::vec2(result.x, result.y) / c_GlobalScaleFactor;
	}
}