#pragma once

#include "file/FileManager.h"

namespace ALZ {

	class FileBrowser
	{
	public:
		FileBrowser(const std::string& startingFolder, const std::string& windowName = "File Browser");

		void OpenWindow();
		void CloseWindow();
		void DisplayGUI(const std::function<void(const std::string&)>& func = nullptr);

		std::vector<std::string> Filter;

	private:
		std::string m_CurrentFolder;
		std::string m_WindowName;
		std::string m_CurrentselectedFilePath;
		std::string m_InputFolder;
		bool m_WindowOpen = false;
	};
}