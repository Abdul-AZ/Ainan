#pragma once

#define BROWSER_LIST_BOX_HEIGHT    15
#define BROWSER_WINDOW_WIDTH   400
#define BROWSER_MIN_WINDOW_HEIGHT  350

namespace Ainan {

	//TODO add refresh functionality
	class AssetManager
	{
	public:
		static void Init(const std::filesystem::path& environmentDirectory);
		static void Terminate();

		static void DisplayGUI();

		static std::string ReadEntireTextFile(const std::string& path);
		//check if the path is valid and OK to use
		static bool EvaluateDirectory(const std::filesystem::path& path);
	public:
		//this a path to the environment directory
		static std::filesystem::path s_EnvironmentDirectory;
		//this a path to the directory the asset browser is in (always inside the environment direcory)
		static std::filesystem::path s_CurrentDirectory;

		//for ease of access
		static std::vector<std::filesystem::path> Images;
		static std::vector<std::filesystem::path> Models;
	};

	void BrowserWindowSizeCallback(ImGuiSizeCallbackData* data);
}