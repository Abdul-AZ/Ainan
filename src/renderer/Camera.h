#pragma once

namespace Ainan {

	enum class ProjectionMode : int32_t
	{
		Orthographic = 0,
		Perspective = 1
	};

	const char* ProjectionModeToStr(ProjectionMode mode);
	ProjectionMode StrToProjectionMode(std::string str);

#define USE_CURRENT_VALUE -1.0f

	class Camera
	{
	public:
		//orthoZoom is only used in Orthographic Projection
		//fov(in degrees) is only used in Perspective Projection
		Camera() {};
		Camera(ProjectionMode mode, glm::mat4 viewMatrix, float aspectRatio, float nearClip = 0.01f, 
			   float farClip = 1000.0f, float orthoZoom = 1.0f, float fov = 60.0f);

		//passing USE_CURRENT_VALUE means to not change the current value
		void SetOrtho(float aspectRatio = USE_CURRENT_VALUE, float orthoZoom = USE_CURRENT_VALUE,
					  float nearClip = USE_CURRENT_VALUE, float farClip = USE_CURRENT_VALUE);
		void SetPersp(float aspectRatio = USE_CURRENT_VALUE, float fov = USE_CURRENT_VALUE,
					  float nearClip = USE_CURRENT_VALUE, float farClip = USE_CURRENT_VALUE);
		void SetAspectRatio(float aspectRatio);
		void SetFOV(float fov);
		void SetViewMatrix(const glm::mat4& view);
		
		float GetOrthoZoomFactor() const { return m_OrthographicZoom; };
		float GetFOV() const { return m_FOV; };
		float GetAspectRatio() const { return m_AspectRatio; };
		glm::mat4 GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; };
		glm::mat4 GetViewMatrix() const { return m_ViewMatrix; };
		glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; };
		ProjectionMode GetProjectionMode() const { return m_Mode; };
	private:
		void CalculateOrthoProjection();
		void CalculatePerspProjection();
		void CalculateViewProjection();

	private:
		ProjectionMode m_Mode = ProjectionMode::Perspective;
		float m_AspectRatio = 16.0f / 9.0f;
		float m_NearClip = 0.01f;
		float m_FarClip = 1000.0f;
		float m_OrthographicZoom = 1.0f;
		float m_FOV = 60.0f;
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);
	};
}