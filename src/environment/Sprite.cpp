#include "Sprite.h"

namespace Ainan {

	Sprite::Sprite()
	{
		Type = SpriteType;
		Space = OBJ_SPACE_3D;
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
		Renderer::DrawQuad(ModelMatrix, Tint, m_Texture);
	}

	void Sprite::DisplayGuiControls()
	{
		DisplayTransformationControls();

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

	void Sprite::LoadTextureFromFile(const std::string& path)
	{
		Renderer::DestroyTexture(m_Texture);

		Image img = Image::LoadFromFile(path, TextureFormat::RGBA);

		m_Texture = Renderer::CreateTexture(img);
	}
}