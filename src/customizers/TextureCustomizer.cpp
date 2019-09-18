#include <pch.h>

#include "TextureCustomizer.h"

namespace ALZ {

	TextureCustomizer::TextureCustomizer() :
		UseDefaultTexture(true)
	{
	}

	TextureCustomizer::TextureCustomizer(const TextureCustomizer& customizer)
	{
		UseDefaultTexture = customizer.UseDefaultTexture;

		if (!UseDefaultTexture)
		{
			ParticleTexture = Renderer::CreateTexture();
			ParticleTexture->SetImage(Image::LoadFromFile(customizer.m_CurrentTexture));
		}
	}

	TextureCustomizer TextureCustomizer::operator=(const TextureCustomizer& customizer)
	{
		return TextureCustomizer(customizer);
	}

	void TextureCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Texture")) 
		{
			if (ImGui::BeginCombo("Texture", UseDefaultTexture ? "Default" : std::filesystem::path(m_CurrentTexture).filename().u8string().c_str()))
			{
				auto textures = AssetManager::GetAll2DTextures();
				bool selected = false;
				if (ImGui::Selectable("Default", &selected))
				{
					UseDefaultTexture = true;
					m_CurrentTexture = "";
				}
				for (auto& tex : textures) 
				{
					std::string textureFileName = std::filesystem::path(tex).filename().u8string();
					if (ImGui::Selectable(textureFileName.c_str(), &selected))
					{
						if (textureFileName != "Default") 
						{
							ParticleTexture = Renderer::CreateTexture();
							ParticleTexture->SetImage(Image::LoadFromFile(tex));
							UseDefaultTexture = false;
							m_CurrentTexture = tex;
						}
					}
				}

				ImGui::EndCombo();
			}

			if (!UseDefaultTexture) {
				ImGui::Text("Current Selected Texture");
				if(ParticleTexture)
					ImGui::Image((void*)(uintptr_t)ParticleTexture->GetRendererID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			}

			ImGui::TreePop();
		}
	}
}