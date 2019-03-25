#include <pch.h>
#include "FileManager.h"

std::string FileManager::ApplicationFolder = "";


std::string FileManager::ReadEntireTextFile(const std::string& path)
{
	std::string file_content;

	FILE* file = fopen(path.c_str(), "r");

	fseek(file, 0, SEEK_END);
	int content_length = ftell(file);
	file_content.resize(content_length + 1);
	fseek(file, 0, SEEK_SET);

	fread((void*)file_content.data(), sizeof(char), content_length, file);

	fclose(file);

	return file_content;
}

void FileManager::Init(const std::string & executableLocation)
{
	ApplicationFolder = executableLocation;
	
	auto lastBackslashLoc = ApplicationFolder.find_last_of('\\');
	ApplicationFolder.erase(lastBackslashLoc, ApplicationFolder.size() - lastBackslashLoc);
}
