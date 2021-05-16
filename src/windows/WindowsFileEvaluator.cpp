#include "WindowsFileEvaluator.h"

#include <Windows.h>

namespace Ainan {

	bool WinEvaluateDirectory(std::filesystem::path path)
	{
		auto attrib = GetFileAttributes(path.u8string().c_str());

		if (attrib & FILE_ATTRIBUTE_HIDDEN)
			return false;

		if (attrib & FILE_ATTRIBUTE_SYSTEM)
			return false;

		return true;
	}
}