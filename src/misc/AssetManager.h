#pragma once

namespace Ainan {

	class AssetManager
	{
	public:
		static void Init(const std::string& environmentDirectory);
		static void Terminate();

		static void DisplayGUI();

		static std::string GetAbsolutePath() { return s_EnvironmentPath; };
		static std::vector<std::string> GetAll2DTextures();
	private:
		//this is an absolute path to the environment working directory
		static std::string s_EnvironmentPath;
		//this is NOT an absolute path. it is a pth relative to the environment path
		static std::string s_CurrentDirectory;
		//if you want an absolute path to the current directory do s_EnvironmentPath + "\\" + s_CurrentDirectory
	};

}