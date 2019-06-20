#include <pch.h>

#include "Environment.h"
#include "json/json.hpp"

using json = nlohmann::json;

namespace ALZ {

	bool SaveEnvironment(const Environment& env, std::string path)
	{
		json data;

		data["backgroundColor"] = {env.m_Settings.BackgroundColor.r, env.m_Settings.BackgroundColor.g, env.m_Settings.BackgroundColor.b, env.m_Settings.BackgroundColor.a };

		std::cout << data;

		return true;
	}
}