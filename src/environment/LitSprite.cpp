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

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(Model, scale, rotation, translation, skew, perspective);

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::DragFloat("##Scale: ", &m_Position.x, c_ObjectPositionDragControlSpeed);

		ImGui::Text("Scale: ");
		ImGui::SameLine();
		float scaleAverage = (scale.x + scale.y + scale.z) / 3.0f;
		if (ImGui::DragFloat("##Scale: ", &scaleAverage, c_ObjectScaleDragControlSpeed))
		{
			Model = glm::scale(Model, (1.0f / scale));
			Model = glm::scale(Model, glm::vec3(scaleAverage));
		}

		ImGui::Text("Rotation: ");
		ImGui::SameLine();
		float rotEular = glm::eulerAngles(rotation).z * 180.0f / PI;
		if (ImGui::DragFloat("##Rotation: ", &rotEular, c_ObjectRotationDragControlSpeed))
		{
			//reconstruct model with new rotation
			Model = glm::mat4(1.0f);
			Model = glm::translate(Model, translation);
			Model = glm::rotate(Model, rotEular * PI / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
			Model = glm::scale(Model, scale);
		}

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

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();

		ImGui::PopID();
	}

	void LitSprite::Draw()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(Model, scale, rotation, translation, skew, perspective);

		//workaround for having one scale value
		Model = glm::scale(Model, (1.0f / scale));
		float scaleAverage = (scale.x + scale.y + scale.z) / 3.0f;
		Model = glm::scale(Model, glm::vec3(scaleAverage));

		m_UniformBufferData.ModelMatrix = Model;

		m_UniformBuffer.UpdateData(&m_UniformBufferData, sizeof(LitSpriteUniformBuffer));

		uint32_t identifier = Renderer::Rdata->UniformBuffers[m_UniformBuffer.Identifier].Identifier;
		uint32_t alignedSize = Renderer::Rdata->UniformBuffers[m_UniformBuffer.Identifier].AlignedSize;

		auto& shader = Renderer::Rdata->ShaderLibrary["LitSpriteShader"];

		shader.BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::VertexShader);
		shader.BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::FragmentShader);

		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, 6);
	}

	int32_t LitSprite::GetAllowedGizmoOperation(ImGuizmo::OPERATION operation)
	{
		if (Space == OBJ_SPACE_2D)
		{
			if (operation == ImGuizmo::OPERATION::TRANSLATE)
				return ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
			else if (operation == ImGuizmo::OPERATION::ROTATE)
				return ImGuizmo::OPERATION::ROTATE_Z;
			else if (operation == ImGuizmo::OPERATION::SCALE)
				return ImGuizmo::OPERATION::SCALE_X | ImGuizmo::OPERATION::SCALE_Y;
		}
		else if (Space == OBJ_SPACE_3D)
		{
			return operation;
		}

		AINAN_LOG_ERROR("Invalid Gizmo Operation Given");
		return -1;
	}
}