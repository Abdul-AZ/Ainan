#pragma once

#define BROWSER_LIST_BOX_HEIGHT    15
#define BROWSER_WINDOW_WIDTH   400
#define BROWSER_MIN_WINDOW_HEIGHT  350

namespace ALZ {

	class FileManager
	{
	public:
		static void Init(const std::string& executableLocation);
		static std::string FileManager::ReadEntireTextFile(const std::string& path);

		static std::string ApplicationFolder;
	};

	void BrowserWindowSizeCallback(ImGuiSizeCallbackData* data);
}