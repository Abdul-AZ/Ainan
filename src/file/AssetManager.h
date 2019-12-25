#pragma once

#define BROWSER_LIST_BOX_HEIGHT    15
#define BROWSER_WINDOW_WIDTH   400
#define BROWSER_MIN_WINDOW_HEIGHT  350

namespace Ainan {

	class AssetManager
	{
	public:
		static void Init(const std::string& environmentDirectory);
		static void Terminate();

		static void DisplayGUI();

		static std::string GetAbsolutePath() { return s_EnvironmentPath; };
		static std::vector<std::string> GetAll2DTextures();
		static std::string ReadEntireTextFile(const std::string& path);
	private:
		//this is an absolute path to the environment working directory
		static std::string s_EnvironmentPath;
		//this is NOT an absolute path. it is a pth relative to the environment path
		static std::string s_CurrentDirectory;
		//if you want an absolute path to the current directory do s_EnvironmentPath + "\\" + s_CurrentDirectory
	};

	void BrowserWindowSizeCallback(ImGuiSizeCallbackData* data);
}