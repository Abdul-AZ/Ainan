#include "Environment.h"
#include "file/FolderBrowser.h"

#define WINDOW_SIZE_FACTOR_ON_LAUNCH 500

namespace ALZ {
	class StartMenu
	{
		enum Status
		{
			DisplayingMainGUI,
			DisplayingCreateEnvironmentGUI
		};

	public:
		StartMenu();

		void Update(Environment*& currentEnv);

	private: //helper functions to display gui in each state
		inline void DisplayMainGUI(Environment*& currentEnv);
		inline void DisplayCreateEnvironmentGUI(Environment*& currentEnv);

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
