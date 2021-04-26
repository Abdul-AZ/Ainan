#include "CameraObject.h"

namespace Ainan {

	CameraObject::CameraObject()
	{
		Type = CameraType;
		m_FrustumOutlineVertexBuffer = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec3) * 8,
			VertexLayout({ VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec3) }), Renderer::ShaderLibrary()["LineShader"], true);

		m_FrustumOutlineUniformBuffer = Renderer::CreateUniformBuffer("ObjectColor", 1, VertexLayout({ VertexLayoutElement("u_Color", 0, ShaderVariableType::Vec4) }));
		glm::vec4 color(0.8f, 0.0f, 0.0f, 1.0f);
		m_FrustumOutlineUniformBuffer.UpdateData((void*)&color, sizeof(glm::vec4));
	}

	CameraObject::~CameraObject()
	{
		Renderer::DestroyVertexBuffer(m_FrustumOutlineVertexBuffer);
		Renderer::DestroyUniformBuffer(m_FrustumOutlineUniformBuffer);
	}

	void CameraObject::DisplayGuiControls()
	{
		DisplayTransformationControls();

		ImGui::NextColumn();

		auto projMode = m_Camera.GetProjectionMode();
		IMGUI_DROPDOWN_START_USING_COLUMNS("Projection Mode: ", ProjectionModeToStr(projMode));
		IMGUI_DROPDOWN_SELECTABLE(projMode, ProjectionMode::Perspective, ProjectionModeToStr(ProjectionMode::Perspective));
		IMGUI_DROPDOWN_SELECTABLE(projMode, ProjectionMode::Orthographic, ProjectionModeToStr(ProjectionMode::Orthographic));
		IMGUI_DROPDOWN_END();
		if (projMode != m_Camera.GetProjectionMode())
		{
			if (projMode == ProjectionMode::Perspective)
				m_Camera.SetPersp();
			else
				m_Camera.SetOrtho();
		}

		ImGui::NextColumn();
		ImGui::Text("Aspect Ratio");
		ImGui::NextColumn();
		if (ImGui::DragInt2("##Aspect Ratio", &m_AspectRatio.x))
			m_Camera.SetAspectRatio((float)m_AspectRatio.x / (float)m_AspectRatio.y);

		switch (projMode)
		{
		case ProjectionMode::Orthographic:
			break;
		case ProjectionMode::Perspective:
		{
			float fov = m_Camera.GetFOV();
			ImGui::NextColumn();
			ImGui::Text("Field Of View");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##Field Of View", &fov))
				m_Camera.SetFOV(fov);
		}
			break;
		default:
			AINAN_LOG_FATAL("Invalid projection mode set");
			break;
		}
	}

	void CameraObject::OnTransform()
	{
		m_Camera.SetViewMatrix(glm::inverse(ModelMatrix));
	}

	void CameraObject::Init(const std::string& name, const glm::mat4& model, const glm::ivec2& aspectRatio, ProjectionMode projMode)
	{
		m_Name = name;
		ModelMatrix = model;
		m_AspectRatio = aspectRatio;
		
		if (projMode == ProjectionMode::Orthographic)
			m_Camera.SetOrtho();
		else
			m_Camera.SetPersp();
		m_Camera.SetViewMatrix(glm::inverse(ModelMatrix));
	}

	void CameraObject::DrawFrustum()
	{
		glm::vec3 v0(0.0f), v1(0.0f), v2(0.0f), v3(0.0f);
		glm::vec3 v0f(0.0f), v1f(0.0f), v2f(0.0f), v3f(0.0f);
		
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(ModelMatrix, scale, rotation, translation, skew, perspective);

		switch (m_Camera.GetProjectionMode())
		{
		case ProjectionMode::Orthographic:
		{
			float zoom = m_Camera.GetOrthoZoomFactor();
			float aspectRatio = m_Camera.GetAspectRatio();
			float frustumWidth = zoom * aspectRatio;
			float frustumHeight = zoom;
			glm::vec3 forward = rotation * c_CameraStartingForwardDirection;

			v0 = translation + rotation * (glm::vec3(-frustumWidth, -frustumHeight, 0.0f) / 2.0f);
			v0f = v0 + forward * 1000.0f;
			v1 = translation + rotation * (glm::vec3(-frustumWidth, frustumHeight, 0.0f) / 2.0f);
			v1f = v1 + forward * 1000.0f;
			v2 = translation + rotation * (glm::vec3(frustumWidth, -frustumHeight, 0.0f) / 2.0f);
			v2f = v2 + forward * 1000.0f;
			v3 = translation + rotation * (glm::vec3(frustumWidth, frustumHeight, 0.0f) / 2.0f);
			v3f = v3 + forward * 1000.0f;
		}
			break;

		case ProjectionMode::Perspective:
		{
			float fov = m_Camera.GetFOV();
			float aspectRatio = m_Camera.GetAspectRatio();
			glm::vec3 forward = rotation * c_CameraStartingForwardDirection;

			float frustumHeight = 2.0 * 1000.0f * tan(glm::radians(fov * 0.5));
			float frustumWidth = frustumHeight * aspectRatio;
			v0 = translation;
			v0f = v0 + forward * 1000.0f + rotation * (glm::vec3(-frustumWidth, -frustumHeight, 0.0f) / 2.0f);
			v1 = translation;
			v1f = v0 + forward * 1000.0f + rotation * (glm::vec3(-frustumWidth, frustumHeight, 0.0f) / 2.0f);
			v2 = translation;
			v2f = v0 + forward * 1000.0f + rotation * (glm::vec3(frustumWidth, -frustumHeight, 0.0f) / 2.0f);
			v3 = translation;
			v3f = v0 + forward * 1000.0f + rotation * (glm::vec3(frustumWidth, frustumHeight, 0.0f) / 2.0f);
			break;
		}
		default:
			AINAN_LOG_FATAL("Invalid projection mode set");
			break;
		}

		glm::vec3 data[] =
		{
			v0, v0f,
			v1, v1f,
			v2, v2f,
			v3, v3f
		};
		m_FrustumOutlineVertexBuffer.UpdateData(0, sizeof(data), data);

		auto& shader = Renderer::ShaderLibrary()["LineShader"];
		shader.BindUniformBuffer(m_FrustumOutlineUniformBuffer, 1, RenderingStage::FragmentShader);
		Renderer::Draw(m_FrustumOutlineVertexBuffer, shader, Primitive::Lines, 8);
	}
}