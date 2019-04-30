#include <pch.h>

#include "TextureCustomizer.h"

namespace ALZ {

	TextureCustomizer::TextureCustomizer() :
		m_FileBrowser("")
	{
		m_FileBrowser.Filter.reserve(4);
		m_FileBrowser.Filter.push_back(".png");
		m_FileBrowser.Filter.push_back(".bmp");
		m_FileBrowser.Filter.push_back(".jpg");
		m_FileBrowser.Filter.push_back(".jpeg");
	}

	void TextureCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Texture")) {

			ImGui::Checkbox("Use Default Texture", &UseDefaultTexture);

			if (!UseDefaultTexture) {
				if (ImGui::Button("Select Texture")) {
					m_FileBrowser.OpenWindow();
				}
			}

			m_FileBrowser.DisplayGUI([this](const std::string& filePath) {
				ParticleTexture.Init(filePath, 4);
				UseDefaultTexture = false;
			});

			if (!UseDefaultTexture) {
				ImGui::Text("Current Selected Texture");
				ImGui::Image((ImTextureID)ParticleTexture.TextureID, ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			}

			ImGui::TreePop();
		}
	}
}