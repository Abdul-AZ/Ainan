#pragma once

#include "FileManager.h"

namespace ALZ {
	class SaveItemBrowser
	{
	public:
		SaveItemBrowser(const std::string& startingFolder, const std::string& windowName = "File Browser");

		void OpenWindow();
		void CloseWindow();
		void DisplayGUI(const std::function<void(const std::string&)>& func = nullptr);
		std::string GetSelectedSavePath();

		std::string m_CurrentselectedFolder;
		std::string m_FileName;
		std::function<void()> OnCloseWindow = nullptr;

	private:
		std::string m_CurrentFolder;
		std::string m_WindowName;
		std::string m_InputFolder;
		bool m_FileNameChosen;

		bool m_WindowOpen = false;
		//to check if window closed last frame
		bool m_LastWindowState = false;
	};
}