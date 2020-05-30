#pragma once

#include "AssetManager.h"

namespace Ainan {

	class FolderBrowser
	{
	public:
		FolderBrowser(const std::string& startingFolder, const std::string& windowName = "Folder Browser");
		FolderBrowser();

		void DisplayGUI(std::function<void(const std::filesystem::path&)> onSelect = nullptr);

	public:
		bool WindowOpen = false;

	private:
		std::filesystem::path SelectedFolder;
		std::string m_WindowName;
		std::string m_InputFolder;
	};
}