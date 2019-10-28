#pragma once

#include "FileManager.h"

namespace Ainan {

	class FileBrowser
	{
	public:
		FileBrowser(const std::string& startingFolder, const std::string& windowName = "File Browser");

		void OpenWindow();
		void CloseWindow();
		void DisplayGUI(const std::function<void(const std::string&)>& func = nullptr);

		std::vector<std::string> Filter;
		std::string m_CurrentselectedFilePath;
		std::function<void()> OnCloseWindow = nullptr;

	private:
		std::string m_CurrentFolder;
		std::string m_WindowName;
		std::string m_InputFolder;
		bool m_WindowOpen = false;
		//to check if window closed last frame
		bool m_LastWindowState = false;
	};
}