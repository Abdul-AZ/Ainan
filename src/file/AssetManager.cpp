#include "AssetManager.h"

namespace Ainan 
{
	std::filesystem::path AssetManager::s_EnvironmentDirectory = "";
	std::filesystem::path AssetManager::s_CurrentDirectory = "";

	void AssetManager::Init(const std::string& environmentDirectory)
	{
		//check if asset manager is initilized more than once
		assert(s_EnvironmentDirectory == "");

		s_EnvironmentDirectory = environmentDirectory;
		s_CurrentDirectory = environmentDirectory;
	}

	void AssetManager::Terminate()
	{
		//check if asset manager is terminated without being initilized
		assert(s_EnvironmentDirectory != "");

		s_EnvironmentDirectory = "";
	}

	void AssetManager::DisplayGUI()
	{
		ImGui::Begin("Asset Explorer");

		if (ImGui::ListBoxHeader("##Files", ImVec2(ImGui::GetWindowWidth() - 20, 370)))
		{
			//if we are in the top level directory(environment directory)
			if (s_CurrentDirectory != "" && s_CurrentDirectory.compare(s_EnvironmentDirectory) > 0)
				if (ImGui::Button(".."))
				{
					s_CurrentDirectory = s_CurrentDirectory.parent_path();
				}

			for (const auto& entry : std::filesystem::directory_iterator(s_CurrentDirectory))
			{
				if (entry.status().type() == std::filesystem::file_type::directory)
				{
					if (ImGui::Button(entry.path().filename().u8string().c_str()))
					{
						s_CurrentDirectory += "\\";
						s_CurrentDirectory += entry.path().filename();
					}
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
		for (const auto& entry : std::filesystem::recursive_directory_iterator(s_EnvironmentDirectory)) 
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

	std::string AssetManager::ReadEntireTextFile(const std::string& path)
	{
		std::string file_content;

		FILE* file = fopen(path.c_str(), "r");

		fseek(file, 0, SEEK_END);
		int content_length = ftell(file);
		file_content.resize(content_length + 1);
		fseek(file, 0, SEEK_SET);

		fread((void*)file_content.data(), sizeof(char), content_length, file);

		fclose(file);

		return file_content;
	}

	void BrowserWindowSizeCallback(ImGuiSizeCallbackData* data)
	{
		data->DesiredSize.x = BROWSER_WINDOW_WIDTH;
	}
}