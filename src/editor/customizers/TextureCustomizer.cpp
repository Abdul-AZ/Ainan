#include "TextureCustomizer.h"

namespace Ainan {

	TextureCustomizer::TextureCustomizer() :
		UseDefaultTexture(true)
	{
	}

	TextureCustomizer::TextureCustomizer(const TextureCustomizer& customizer)
	{
		UseDefaultTexture = customizer.UseDefaultTexture;

		if (!UseDefaultTexture)
		{
			ParticleTexture = Renderer::CreateTexture(Image::LoadFromFile(AssetManager::s_EnvironmentDirectory.u8string() + "\\" + customizer.m_TexturePath.u8string()));
		}
	}

	TextureCustomizer TextureCustomizer::operator=(const TextureCustomizer& customizer)
	{
		return TextureCustomizer(customizer);
	}

	void TextureCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		if (ImGui::TreeNode("Texture"))
		{
			ImGui::Text("Texture: ");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Texture: ", UseDefaultTexture ? "Default" : m_TexturePath.filename().u8string().c_str()))
			{
				auto textures = AssetManager::GetAll2DTextures();
				bool selected = false;
				if (ImGui::Selectable("Default", &selected))
				{
					UseDefaultTexture = true;
					m_TexturePath = "";
				}
				for (auto& tex : textures) 
				{
					std::string textureFileName = std::filesystem::path(tex).filename().u8string();
					if (ImGui::Selectable(textureFileName.c_str(), &selected))
					{
						if (textureFileName != "Default") 
						{
							ParticleTexture = Renderer::CreateTexture(Image::LoadFromFile(tex.u8string()));
							
							UseDefaultTexture = false;
							m_TexturePath = tex.lexically_relative(AssetManager::s_EnvironmentDirectory).u8string();
						}
					}
				}

				ImGui::EndCombo();
			}

			if (!UseDefaultTexture) {
				ImGui::Text("Current Selected Texture");
				if(ParticleTexture)
					ImGui::Image(ParticleTexture->GetTextureID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			}

			ImGui::TreePop();
		}
	}
}