#include <pch.h>

#include "LitSprite.h"

namespace Ainan {

	LitSprite::LitSprite()
	{
		Type = LitSpriteType;
		auto vertices = Renderer::GetQuadVertices();

		VertexLayout vertexBufferlayout = 
		{
			{ "aPos", ShaderVariableType::Vec2 } 
		};
		m_VertexBuffer = Renderer::CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(glm::vec2), vertexBufferlayout, Renderer::ShaderLibrary()["LitSpriteShader"]);

		VertexLayout uniformBufferLayout =
		{
			{ "u_Model"    , ShaderVariableType::Mat4  },
			{ "u_BaseColor", ShaderVariableType::Vec4  },
			{ "u_BaseLight", ShaderVariableType::Float },
			{ "u_Constant" , ShaderVariableType::Float },
			{ "u_Linear"   , ShaderVariableType::Float },
			{ "u_Quadratic", ShaderVariableType::Float }
		};

		m_UniformBuffer = Renderer::CreateUniformBuffer("ObjectData", 1, uniformBufferLayout, nullptr);
	}

	void LitSprite::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen, ImGuiWindowFlags_NoSavedSettings);

		const float spacing = 175.0f;

		ImGui::Text("Color/Tint: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::ColorEdit4("##Color/Tint: ", &m_UniformBufferData.Tint.r);

		ImGui::Text("Scale: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Scale: ", &m_Scale, 0.01f);

		ImGui::Text("Rotate: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Rotate: ", &m_Rotation);
		m_Rotation = std::clamp(m_Rotation, 0.0f, 360.0f);

		ImGui::Text("Base Light: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::SliderFloat("##Base Light: ", &m_UniformBufferData.BaseLight, 0.0f, 1.0f);

		ImGui::Text("Material: ");

		ImGui::Text("Constant Coefficient: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::DragFloat("##Constant Coefficient: ", &m_UniformBufferData.MaterialConstantCoefficient, 0.01f);

		ImGui::Text("Linear Coefficient: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::DragFloat("##Linear Coefficient: ", &m_UniformBufferData.MaterialLinearCoefficient, 0.0001f);

		ImGui::Text("Quadratic Coefficient: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::DragFloat("##Quadratic Coefficient: ", &m_UniformBufferData.MaterialQuadraticCoefficient, 0.00001f);

		ImGui::End();

		ImGui::PopID();
	}

	void LitSprite::Draw()
	{
		auto& model = m_UniformBufferData.ModelMatrix;
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(m_Position.x, m_Position.y, 0.0f) * c_GlobalScaleFactor);
		model = glm::rotate(model, glm::radians(m_Rotation), glm::vec3(0, 0, 1.0f));
		model = glm::scale(model, glm::vec3(m_Scale, m_Scale, m_Scale) * c_GlobalScaleFactor);

		m_UniformBuffer->UpdateData(&m_UniformBufferData);

		auto& shader = Renderer::ShaderLibrary()["LitSpriteShader"];

		shader->BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::VertexShader);
		shader->BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::FragmentShader);

		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}
}