#include "Environment.h"

namespace ALZ {
	class StartMenu
	{
	public:
		StartMenu();

		void Update(Environment*& currentEnv);

		FileBrowser LoadEnvironmentPath;

	private:
		std::string m_EnvironmentLoadError;
	};

}
