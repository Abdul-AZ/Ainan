#include <pch.h>

#include "TextureCustomizer.h"

namespace ALZ {

	TextureCustomizer::TextureCustomizer() :
		m_FileBrowser("res/"),
		UseDefaultTexture(true)
	{
		m_FileBrowser.Filter.reserve(4);
		m_FileBrowser.Filter.push_back(".png");
		m_FileBrowser.Filter.push_back(".bmp");
		m_FileBrowser.Filter.push_back(".jpg");
		m_FileBrowser.Filter.push_back(".jpeg");
	}

	TextureCustomizer::TextureCustomizer(const TextureCustomizer& customizer) :
		m_FileBrowser(customizer.m_FileBrowser)
	{
		UseDefaultTexture = customizer.UseDefaultTexture;

		if (!UseDefaultTexture)
		{
			ParticleTexture = Renderer::CreateTexture();
			ParticleTexture->SetImage(Image::LoadFromFile(customizer.m_FileBrowser.m_CurrentselectedFilePath));
		}
	}

	TextureCustomizer TextureCustomizer::operator=(const TextureCustomizer& customizer)
	{
		return TextureCustomizer(customizer);
	}

	void TextureCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Texture")) {

			ImGui::Text("Use Default Texture");
			ImGui::SameLine();
			ImGui::Checkbox("##Use Default Texture", &UseDefaultTexture);

			if (!UseDefaultTexture) {
				if (ImGui::Button("Select Texture")) {
					m_FileBrowser.OpenWindow();
				}
			}

			m_FileBrowser.DisplayGUI([this](const std::string& filePath) {
				ParticleTexture = Renderer::CreateTexture();
				ParticleTexture->SetImage(Image::LoadFromFile(filePath));
				UseDefaultTexture = false;
			});

			if (!UseDefaultTexture) {
				ImGui::Text("Current Selected Texture");
				if(ParticleTexture)
					ImGui::Image((void*)(uintptr_t)ParticleTexture->GetRendererID(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			}

			ImGui::TreePop();
		}
	}
}