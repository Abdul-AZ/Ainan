#include "LitSprite.h"

namespace Ainan {

	LitSprite::LitSprite()
	{
		Type = LitSpriteType;
		auto vertices = Renderer::GetQuadVertices();

		VertexLayout vertexBufferlayout = 
		{
			VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2)
		};
		m_VertexBuffer = Renderer::CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(glm::vec2), vertexBufferlayout, Renderer::Rdata->ShaderLibrary["LitSpriteShader"]);

		VertexLayout uniformBufferLayout =
		{
			VertexLayoutElement( "u_Model"    ,0, ShaderVariableType::Mat4 ),
			VertexLayoutElement( "u_BaseColor",0, ShaderVariableType::Vec4 ),
			VertexLayoutElement( "u_BaseLight",0, ShaderVariableType::Float),
			VertexLayoutElement( "u_Constant" ,0, ShaderVariableType::Float),
			VertexLayoutElement( "u_Linear"   ,0, ShaderVariableType::Float),
			VertexLayoutElement( "u_Quadratic",0, ShaderVariableType::Float)
		};

		m_UniformBuffer = Renderer::CreateUniformBuffer("ObjectData", 1, uniformBufferLayout);

	}

	LitSprite::~LitSprite()
	{
		Renderer::DestroyVertexBuffer(m_VertexBuffer);
		Renderer::DestroyUniformBuffer(m_UniformBuffer);
	}

	void LitSprite::DisplayGUI()
	{
		if (!EditorOpen)
			return;

		ImGui::PushID(this);

		ImGui::Begin((m_Name + "##" + std::to_string(ImGui::GetID(this))).c_str(), &EditorOpen, ImGuiWindowFlags_NoSavedSettings);

		const float spacing = 175.0f;

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Scale: ", &m_Position.x, c_ObjectPositionDragControlSpeed);

		ImGui::Text("Scale: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Scale: ", &m_Scale, c_ObjectScaleDragControlSpeed);

		ImGui::Text("Rotation: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Rotation: ", &m_Rotation, c_ObjectRotationDragControlSpeed);
		m_Rotation = std::clamp(m_Rotation, 0.0f, 360.0f);

		ImGui::Text("Color/Tint: ");
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::ColorEdit4("##Color/Tint: ", &m_UniformBufferData.Tint.r);

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
		model = glm::translate(model, glm::vec3(m_Position.x, m_Position.y, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f - m_Rotation), glm::vec3(0, 0, 1.0f));
		model = glm::scale(model, glm::vec3(m_Scale, m_Scale, m_Scale));
		
		m_UniformBuffer.UpdateData(&m_UniformBufferData, sizeof(LitSpriteUniformBuffer));

		uint32_t identifier = Renderer::Rdata->UniformBuffers[m_UniformBuffer.Identifier].Identifier;
		uint32_t alignedSize = Renderer::Rdata->UniformBuffers[m_UniformBuffer.Identifier].AlignedSize;

		auto& shader = Renderer::Rdata->ShaderLibrary["LitSpriteShader"];

		shader.BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::VertexShader);
		shader.BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::FragmentShader);

		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}
}