#pragma once

#include "EnvironmentObjectInterface.h"

#include "renderer/Renderer.h"

namespace Ainan {

	class CameraObject : public EnvironmentObjectInterface
	{
	public:
		CameraObject();
		virtual ~CameraObject();
		Camera GetCamera() const { return m_Camera; };
		virtual void DisplayGuiControls() override;
		virtual void OnTransform() override;
		void Init(const std::string& name, const glm::mat4& model, const glm::ivec2& aspectRatio, ProjectionMode projMode);
		void DrawFrustum();
	public:
		VertexBuffer m_FrustumOutlineVertexBuffer;
		UniformBuffer m_FrustumOutlineUniformBuffer;
		Camera m_Camera;
		glm::ivec2 m_AspectRatio = glm::ivec2(16, 9);
	};
}
