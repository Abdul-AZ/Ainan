#include "LitSprite.h"

namespace Ainan {

	LitSprite::LitSprite()
	{
		Type = LitSpriteType;
		Space = OBJ_SPACE_2D;
		std::array<glm::vec2, 6> vertices =
		{
			 glm::vec2(-1.0f,  1.0f),  //top left
			 glm::vec2(1.0f,   1.0f),  //top right
			 glm::vec2(-1.0f, -1.0f),  //bottom left

			 glm::vec2(1.0f,   1.0f),  //top left
			 glm::vec2(1.0f,  -1.0f),  //bottom right
			 glm::vec2(-1.0f, -1.0f)   //bottom left
		};

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

	void LitSprite::DisplayGuiControls()
	{
		DisplayTransformationControls();

		ImGui::NextColumn();
		ImGui::Text("Color/Tint: ");
		ImGui::NextColumn();
		ImGui::ColorEdit4("##Color/Tint: ", &m_UniformBufferData.Tint.r);

		ImGui::NextColumn();
		ImGui::Text("Base Light: ");
		ImGui::NextColumn();
		ImGui::SliderFloat("##Base Light: ", &m_UniformBufferData.BaseLight, 0.0f, 1.0f);

		ImGui::NextColumn();
		if(ImGui::TreeNode("Material"))
		{
			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Constant Coefficient: ");
			
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			ImGui::Spacing();
			ImGui::DragFloat("##Constant Coefficient: ", &m_UniformBufferData.MaterialConstantCoefficient, 0.01f);

			ImGui::NextColumn();
			ImGui::Text("Linear Coefficient: ");
			ImGui::NextColumn();
			ImGui::DragFloat("##Linear Coefficient: ", &m_UniformBufferData.MaterialLinearCoefficient, 0.0001f);

			ImGui::NextColumn();
			ImGui::Text("Quadratic Coefficient: ");
			ImGui::NextColumn();
			ImGui::DragFloat("##Quadratic Coefficient: ", &m_UniformBufferData.MaterialQuadraticCoefficient, 0.00001f);

			ImGui::TreePop();
		}
	}

	void LitSprite::Draw()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(ModelMatrix, scale, rotation, translation, skew, perspective);

		//workaround for having one scale value
		ModelMatrix = glm::scale(ModelMatrix, (1.0f / scale));
		float scaleAverage = (scale.x + scale.y + scale.z) / 3.0f;
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scaleAverage));

		m_UniformBufferData.ModelMatrix = ModelMatrix;

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