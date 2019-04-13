#pragma once

namespace ALZ {

	static class FileManager
	{
	public:
		static void Init(const std::string& executableLocation);
		static std::string FileManager::ReadEntireTextFile(const std::string& path);

		static std::string ApplicationFolder;
	};
}