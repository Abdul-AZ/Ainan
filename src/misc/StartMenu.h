#pragma once

#include "environment/EnvironmentData.h"
#include "Environment.h"
#include "file/FolderBrowser.h"
#include "file/FileBrowser.h"

#define WINDOW_SIZE_FACTOR_ON_LAUNCH 500

namespace Ainan {
	class StartMenu
	{
		enum Status
		{
			DisplayingMainGUI,
			DisplayingCreateEnvironmentGUI
		};

	public:
		StartMenu();

		void Draw(EnvironmentData*& currentEnv);

	private: //helper functions to display gui in each state
		inline void DisplayMainGUI(EnvironmentData*& currentEnv);
		inline void DisplayCreateEnvironmentGUI(EnvironmentData*& currentEnv);

	private:
		std::string m_EnvironmentCreateFolderPath;
		std::string m_EnvironmentCreateName;
		Status m_CurrentStatus = DisplayingMainGUI;
		std::string m_EnvironmentLoadError;
		bool m_CreateEvironmentDirectory = false;
		FolderBrowser m_FolderBrowser;
		FileBrowser m_LoadEnvironmentBrowser;
		bool m_IncludeStarterAssets = false;
	};

}
