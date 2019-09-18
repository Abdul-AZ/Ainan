#include <pch.h>

#include "EnvironmentFileExplorer.h"

namespace ALZ 
{
	EnvironmentFileExplorer::EnvironmentFileExplorer(const std::string& environmentDirPath) :
		m_EnvironmentPath(environmentDirPath)
	{}

	void EnvironmentFileExplorer::DisplayGUI()
	{
		ImGui::Begin("File Explorer");

		if (ImGui::ListBoxHeader("##Files", ImVec2(0, ImGui::GetWindowHeight() - 30)))
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(m_EnvironmentPath)) {
				ImGui::Text(entry.path().filename().u8string().c_str());
			}

			ImGui::ListBoxFooter();
		}

		ImGui::End();
	}
	std::vector<std::string> EnvironmentFileExplorer::GetAllTextures()
	{
		std::vector<std::string> result;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(m_EnvironmentPath)) 
		{
			if(entry.path().filename().u8string().find(".jpg") != std::string::npos ||
				entry.path().filename().u8string().find(".jpeg") != std::string::npos ||
				entry.path().filename().u8string().find(".bmp") != std::string::npos ||
				entry.path().filename().u8string().find(".png") != std::string::npos)
				result.push_back(entry.path().u8string().c_str());
		}

		return result;
	}
}