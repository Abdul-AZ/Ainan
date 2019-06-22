#include "Environment.h"

namespace ALZ {
	class StartMenu
	{
	public:
		StartMenu();

		void Update(Environment*& currentEnv);

		FileBrowser LoadEnvironmentPath;
	};

}
