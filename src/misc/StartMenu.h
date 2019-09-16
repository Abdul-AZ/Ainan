#include "Environment.h"

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

		FileBrowser LoadEnvironmentPath;

	private: //helper functions to display gui in each state
		inline void DisplayMainGUI(Environment*& currentEnv);
		inline void DisplayCreateEnvironmentGUI(Environment*& currentEnv);

	private:
		std::string m_EnvironmentCreateFolderPath;
		std::string m_EnvironmentCreateName;
		Status m_CurrentStatus = DisplayingMainGUI;
		std::string m_EnvironmentLoadError;
		bool m_CreateEvironmentDirectory = false;
	};

}
