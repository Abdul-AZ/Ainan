#include "Sprite.h"

namespace Ainan {

	Sprite::Sprite()
	{
		Type = SpriteType;
		Space = OBJ_SPACE_2D;
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
		DisplayTransformationControls();

		ImGui::NextColumn();
		ImGui::Text("Space: ");
		ImGui::NextColumn();
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

		ImGui::NextColumn();
		ImGui::Text("Texture: ");
		ImGui::NextColumn();

		if (ImGui::BeginCombo("##Texture: ", m_TexturePath == "" ? "None" : m_TexturePath.filename().u8string().c_str()))
		{
			bool selected = false;
			if (ImGui::Selectable("None", &selected))
			{
				LoadTextureFromFile("res/CheckerBoard.png");
				m_TexturePath = "";
			}
			for (auto& tex : AssetManager::Images)
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

		ImGui::NextColumn();
		ImGui::Text("Texture Preview: ");
		ImGui::NextColumn();
		ImGui::Image((void*)m_Texture.GetTextureID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));

		ImGui::NextColumn();
		ImGui::Text("Tint: ");
		ImGui::NextColumn();
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

		Image img = Image::LoadFromFile(path, TextureFormat::RGBA);

		m_Texture = Renderer::CreateTexture(img);
	}
}