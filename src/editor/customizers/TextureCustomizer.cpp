#include "TextureCustomizer.h"

namespace Ainan {

	TextureCustomizer::TextureCustomizer() :
		UseDefaultTexture(true)
	{
	}

	TextureCustomizer::~TextureCustomizer()
	{
		if (ParticleTexture.IsValid())
			Renderer::DestroyTexture(ParticleTexture);
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
		ImGui::NextColumn();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Texture"))
		{
			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Texture: ");
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			if (ImGui::BeginCombo("##Texture: ", UseDefaultTexture ? "Default" : m_TexturePath.filename().u8string().c_str()))
			{
				bool selected = false;
				if (ImGui::Selectable("Default", &selected))
				{
					UseDefaultTexture = true;
					m_TexturePath = "";
				}
				for (auto& tex : AssetManager::Images) 
				{
					std::string textureFileName = std::filesystem::path(tex).filename().u8string();
					if (ImGui::Selectable(textureFileName.c_str(), &selected))
					{
						if (textureFileName != "Default") 
						{
							ParticleTexture = Renderer::CreateTexture(Image::LoadFromFile(tex.u8string()));
							Renderer::WaitUntilRendererIdle(); //TEMPORARY FIX 
							
							UseDefaultTexture = false;
							m_TexturePath = tex.lexically_relative(AssetManager::s_EnvironmentDirectory).u8string();
						}
					}
				}

				ImGui::EndCombo();
			}

			if (!UseDefaultTexture) 
			{
				ImGui::NextColumn();
				ImGui::Text("Texture Preview: ");
				ImGui::NextColumn();
				if (ParticleTexture.IsValid())
					ImGui::Image((void*)ParticleTexture.GetTextureID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			}

			ImGui::NextColumn();
			ImGui::TreePop();
		}
	}
}