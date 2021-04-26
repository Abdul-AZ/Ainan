#include "Camera.h"
#include "Camera.h"

namespace Ainan {
	
	Camera::Camera(ProjectionMode mode, glm::mat4 viewMatrix, float aspectRatio, float nearClip, float farClip, float orthoZoom, float fov) :
		m_Mode(mode), m_ViewMatrix(viewMatrix), m_AspectRatio(aspectRatio), m_NearClip(nearClip), 
		m_FarClip(farClip), m_OrthographicZoom(orthoZoom), m_FOV(fov)
	{
		switch (mode)
		{
		case ProjectionMode::Orthographic:
			CalculateOrthoProjection();
			break;

		case ProjectionMode::Perspective:
			CalculatePerspProjection();
			break;

		default:
			AINAN_LOG_FATAL("Invalid projection made passed");
			break;
		}

		CalculateViewProjection();
	}

	void Camera::SetOrtho(float aspectRatio, float orthoZoom, float nearClip, float farClip)
	{
		m_Mode = ProjectionMode::Orthographic;

		if (aspectRatio != USE_CURRENT_VALUE)
			m_AspectRatio = aspectRatio;
		if (orthoZoom != USE_CURRENT_VALUE)
			m_OrthographicZoom = orthoZoom;
		if (nearClip != USE_CURRENT_VALUE)
			m_NearClip = nearClip;
		if (farClip != USE_CURRENT_VALUE)
			m_FarClip = farClip;

		CalculateOrthoProjection();
		CalculateViewProjection();
	}

	void Camera::SetPersp(float aspectRatio, float fov, float nearClip, float farClip)
	{
		m_Mode = ProjectionMode::Perspective;

		if (aspectRatio != USE_CURRENT_VALUE)
			m_AspectRatio = aspectRatio;
		if (fov != USE_CURRENT_VALUE)
			m_FOV = fov;
		if (nearClip != USE_CURRENT_VALUE)
			m_NearClip = nearClip;
		if (farClip != USE_CURRENT_VALUE)
			m_FarClip = farClip;

		CalculatePerspProjection();
		CalculateViewProjection();
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		if (m_Mode == ProjectionMode::Orthographic)
			CalculateOrthoProjection();
		else if (m_Mode == ProjectionMode::Perspective)
			CalculatePerspProjection();
		else
			AINAN_LOG_FATAL("Unknown projection mode is set");

		CalculateViewProjection();
	}

	void Camera::SetFOV(float fov)
	{
		m_FOV = fov;
		CalculatePerspProjection();
		CalculateViewProjection();
	}

	void Camera::SetViewMatrix(const glm::mat4& view)
	{
		m_ViewMatrix = view;
		CalculateViewProjection();
	}

	void Camera::CalculateOrthoProjection()
	{
		m_ProjectionMatrix = glm::ortho(
			-m_OrthographicZoom * m_AspectRatio / 2.0f,  //left
			 m_OrthographicZoom * m_AspectRatio / 2.0f,  //right
			-m_OrthographicZoom / 2.0f,                  //bottom
			 m_OrthographicZoom / 2.0f,                  //top
			 m_NearClip,
			 m_FarClip
		);
	}

	void Camera::CalculatePerspProjection()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians((float)m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void Camera::CalculateViewProjection()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	const char* ProjectionModeToStr(ProjectionMode mode)
	{
		switch (mode)
		{
		case ProjectionMode::Orthographic:
			return "Orthographic";
		case ProjectionMode::Perspective:
			return "Perspective";
		}

		AINAN_LOG_ERROR("Unknown projection mode passed");
		return "";
	}

	ProjectionMode StrToProjectionMode(std::string str)
	{
		if (str == "Orthographic")
			return ProjectionMode::Orthographic;
		else if (str == "Perspective")
			return ProjectionMode::Perspective;

		AINAN_LOG_ERROR("Unknown projection mode passed");
		return ProjectionMode::Perspective;
	}
}