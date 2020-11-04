#pragma once

#include "AssetManager.h"

namespace Ainan {
	class SaveItemBrowser
	{
	public:
		SaveItemBrowser();
		SaveItemBrowser(const std::string& startingFolder, const std::string& windowName = "File Browser");

		void OpenWindow();
		void CloseWindow();
		void DisplayGUI(const std::function<void(const std::string&)>& func = nullptr);
		std::string GetSelectedSavePath();

		std::string m_FileName;
		std::string FileExtension = "";
		std::function<void()> OnCloseWindow = nullptr;

		std::string m_InputFolder;
	private:
		std::string m_WindowName;
		bool m_FileNameChosen;

		bool m_WindowOpen = false;
		//to check if window closed last frame
		bool m_LastWindowState = false;

		std::filesystem::path m_CurrentselectedFilePath;
		std::filesystem::path m_CurrentFolderPath;
	};
}