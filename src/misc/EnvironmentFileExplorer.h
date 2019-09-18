#pragma once

namespace ALZ {

	class EnvironmentFileExplorer
	{
	public:
		EnvironmentFileExplorer(const std::string& environmentDirPath);

		void DisplayGUI();
		void Refresh();

		std::vector<std::string> GetAllTextures();

	private:
		std::string m_EnvironmentPath;
	};

}