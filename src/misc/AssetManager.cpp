#include <pch.h>

#include "AssetManager.h"

namespace ALZ 
{
	std::string AssetManager::s_EnvironmentPath = "";
	std::string AssetManager::s_CurrentDirectory = "";

	void AssetManager::Init(const std::string& environmentDirectory)
	{
		//check if asset manager is initilized more than once
		assert(s_EnvironmentPath == "");

		s_EnvironmentPath = environmentDirectory;
	}

	void AssetManager::Terminate()
	{
		//check if asset manager is terminated without being initilized
		assert(s_EnvironmentPath != "");

		s_EnvironmentPath = "";
	}

	void AssetManager::DisplayGUI()
	{
		ImGui::Begin("Asset Explorer");

		if (ImGui::ListBoxHeader("##Files", ImVec2(ImGui::GetWindowWidth() - 20, 370)))
		{
			//if we are in the top level directory(environment directory)
			if (s_CurrentDirectory != "")
				if (ImGui::Button(".."))
				{
					size_t backslashPos = s_CurrentDirectory.find_last_of("\\");
					if (backslashPos != std::string::npos && backslashPos != 0)
						s_CurrentDirectory = s_CurrentDirectory.substr(0, backslashPos - 1);
					else
						s_CurrentDirectory = "";
				}

			for (const auto& entry : std::filesystem::directory_iterator(s_EnvironmentPath + "\\" + s_CurrentDirectory))
			{
				if (entry.status().type() == std::filesystem::file_type::directory)
				{
					if (ImGui::Button(entry.path().filename().u8string().c_str()))
						s_CurrentDirectory = s_CurrentDirectory + "\\" + entry.path().filename().u8string();
				}
				else
					ImGui::Text(entry.path().filename().u8string().c_str());
			}

			ImGui::ListBoxFooter();
		}

		ImGui::End();
	}

	std::vector<std::string> AssetManager::GetAll2DTextures()
	{
		std::vector<std::string> result;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(s_EnvironmentPath)) 
		{
			//check if the files format contains any of the supported image formats(jpg, bmp and png)
			if(entry.path().filename().u8string().find(".jpg") != std::string::npos ||
				entry.path().filename().u8string().find(".jpeg") != std::string::npos ||
				entry.path().filename().u8string().find(".bmp") != std::string::npos ||
				entry.path().filename().u8string().find(".png") != std::string::npos)
				result.push_back(entry.path().u8string().c_str());
		}

		return result;
	}
}