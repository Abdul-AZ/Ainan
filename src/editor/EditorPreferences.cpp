#include "EditorPreferences.h"
#include <json/json.hpp>

#define JSON_ARRAY_TO_IVEC2(arr) glm::ivec2(arr[0], arr[1])

namespace Ainan {

	EditorPreferences EditorPreferences::Default()
	{
		EditorPreferences defaultPreferences;

		defaultPreferences.Style = EditorStyle::Dark_Gray;
		defaultPreferences.WindowMaximized = true;
		defaultPreferences.WindowSize = { 1280, 720 };

		//we default to Direct X in windows because we want nativity
#ifdef PLATFORM_WINDOWS
		defaultPreferences.RenderingBackend = RendererType::D3D11;
#else
		defaultPreferences.RenderingBackend = RendererType::OpenGL;
#endif // PLATFORM_WINDOWS

		return defaultPreferences;
	}

	EditorPreferences EditorPreferences::LoadFromDefaultPath()
	{
		FILE* file;
		fopen_s(&file, c_DefaultPreferencesPath, "r");

		EditorPreferences preferences = Default();

		if (file)
		{
			nlohmann::json j;
			std::string errorStr = "";
			try
			{
				j = nlohmann::json::parse(AssetManager::ReadEntireTextFile(c_DefaultPreferencesPath));
			}
			catch (const std::exception& e)
			{
				errorStr = e.what();
			}

			if (errorStr == "")
			{
				preferences.WindowMaximized =  j["EditorWindowMaximized"].get<bool>();
				preferences.WindowSize = JSON_ARRAY_TO_IVEC2(j["EditorWindowSize"].get<std::vector<float>>());
				preferences.Style = EditorStyleVal(j["EditorStyle"].get<std::string>());
				preferences.RenderingBackend = RendererTypeVal(j["EditorBackend"].get<std::string>());
			}

			fclose(file);
		}

		return preferences;
	}

	std::string EditorPreferences::SerializeIntoJson()
	{
		nlohmann::json j;

		j["EditorWindowMaximized"] = WindowMaximized;
		j["EditorWindowSize"] = { WindowSize.x, WindowSize.y };
		j["EditorWindowSize"] = { WindowSize.x, WindowSize.y };
		j["EditorStyle"] = EditorStyleStr(Style);
		j["EditorBackend"] = RendererTypeStr(RenderingBackend);

		return j.dump(4);
	}

	void EditorPreferences::SaveToDefaultPath()
	{
		std::string jsonStr = SerializeIntoJson();

		FILE* file;
		fopen_s(&file, c_DefaultPreferencesPath, "w");

		if (file)
		{
			fwrite(jsonStr.data(), 1, jsonStr.size(), file);

			fclose(file);
		}
		else
			assert(false);

	}
}

#undef JSON_ARRAY_TO_IVEC2