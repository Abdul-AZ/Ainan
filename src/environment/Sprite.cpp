#include "Sprite.h"

namespace Ainan {

	Sprite::Sprite()
	{
		Type = SpriteType;
		m_Name = "Sprite";

		Image img = Image::LoadFromFile("res/CheckerBoard.png");

		Image::GrayScaleToRGBA(img);

		m_Texture = Renderer::CreateTexture(img);
	}

	Sprite::~Sprite()
	{
		Renderer::DestroyTexture(m_Texture);
	}

	void Sprite::Update(const float deltaTime)
	{
	}

	void Sprite::Draw()
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

		if (Space == OBJ_SPACE_2D)
		{
			Renderer::DrawQuad(translation, Tint, scaleAverage, glm::eulerAngles(rotation).z, m_Texture);
		}
	}

	void Sprite::DisplayGuiControls()
	{
		if (ImGui::BeginCombo("##Space: ", ObjSpaceToStr(Space)))
		{
			{
				bool selected = Space == OBJ_SPACE_2D;
				if (ImGui::Selectable(ObjSpaceToStr(OBJ_SPACE_2D), &selected))
					Space = OBJ_SPACE_2D;
			}

			{
				bool selected = Space == OBJ_SPACE_3D;
				if (ImGui::Selectable(ObjSpaceToStr(OBJ_SPACE_3D), &selected))
					Space = OBJ_SPACE_3D;
			}

			ImGui::EndCombo();
		}

		ImGui::Text("Texture: ");
		ImGui::SameLine();
		ImGui::Image((void*)m_Texture.GetTextureID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));

		if (ImGui::BeginCombo("##Texture: ", m_TexturePath == "" ? "None" : m_TexturePath.filename().u8string().c_str()))
		{
			auto textures = AssetManager::GetAll2DTextures();
			bool selected = false;
			if (ImGui::Selectable("None", &selected))
			{
				LoadTextureFromFile("res/CheckerBoard.png");
				m_TexturePath = "";
			}
			for (auto& tex : textures)
			{
				std::string textureFileName = std::filesystem::path(tex).filename().u8string();
				if (ImGui::Selectable(textureFileName.c_str(), &selected))
				{
					if (textureFileName != "Default")
					{
						LoadTextureFromFile(tex.u8string());
						m_TexturePath = tex.lexically_relative(AssetManager::s_EnvironmentDirectory).u8string();
					}
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(ModelMatrix, scale, rotation, translation, skew, perspective);

		ImGui::Text("Position: ");
		ImGui::SameLine();
		ImGui::DragFloat2("##Position: ", &ModelMatrix[3][0], c_ObjectPositionDragControlSpeed);

		ImGui::Text("Scale: ");
		ImGui::SameLine();
		float scaleAverage = (scale.x + scale.y + scale.z) / 3.0f;
		if (ImGui::DragFloat("##Scale: ", &scaleAverage, c_ObjectScaleDragControlSpeed))
		{
			ModelMatrix = glm::scale(ModelMatrix, (1.0f / scale));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scaleAverage));
		}

		ImGui::Text("Rotation: ");
		ImGui::SameLine();
		float rotEular = glm::eulerAngles(rotation).z * 180.0f / glm::pi<float>();
		if (ImGui::DragFloat("##Rotation: ", &rotEular, c_ObjectRotationDragControlSpeed))
		{
			//reconstruct model with new rotation
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, translation);
			ModelMatrix = glm::rotate(ModelMatrix, rotEular * glm::pi<float>() / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::scale(ModelMatrix, scale);
		}

		ImGui::Text("Tint: ");
		ImGui::SameLine();
		ImGui::ColorEdit4("##Tint: ", &Tint.r);
	}

	int32_t Sprite::GetAllowedGizmoOperation(ImGuizmo::OPERATION operation)
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

	void Sprite::LoadTextureFromFile(const std::string& path)
	{
		Renderer::DestroyTexture(m_Texture);

		Image img = Image::LoadFromFile(path);

		if (img.Format == TextureFormat::R)
			Image::GrayScaleToRGB(img);

		m_Texture = Renderer::CreateTexture(img);
	}
}