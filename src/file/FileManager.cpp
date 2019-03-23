#include <pch.h>
#include "FileManager.h"

std::string FileManager::ApplicationFolder = "";

void FileManager::Init(const std::string & executableLocation)
{
	ApplicationFolder = executableLocation;

	auto lastBackslashLoc = ApplicationFolder.find_last_of('\\');
	ApplicationFolder.erase(lastBackslashLoc, ApplicationFolder.size() - lastBackslashLoc);
	std::cout << ApplicationFolder;
}
