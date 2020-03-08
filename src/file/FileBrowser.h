#pragma once

#include "AssetManager.h"

namespace Ainan {

	class FileBrowser
	{
	public:
		FileBrowser(const std::string& startingFolder, const std::string& windowName = "File Browser");

		void OpenWindow();
		void CloseWindow();
		void DisplayGUI(const std::function<void(const std::filesystem::path)>& func = nullptr);

		std::vector<std::string> Filter;
		std::filesystem::path m_CurrentselectedFilePath;
		std::function<void()> OnCloseWindow = nullptr;

	private:
		std::filesystem::path m_CurrentFolderPath;
		std::string m_WindowName;
		std::string m_InputFolder;
		bool m_WindowOpen = false;
		//to check if window closed last frame
		bool m_LastWindowState = false;
	};
}