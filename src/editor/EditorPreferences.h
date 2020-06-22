#pragma once

#include "EditorStyles.h"
#include "renderer/Renderer.h"
#include "file/AssetManager.h"

namespace Ainan {

	const char* const c_DefaultPreferencesPath = "user_settings.info";

	struct EditorPreferences
	{
		static EditorPreferences Default();
		//returns Default() if there is no file in path
		static EditorPreferences LoadFromDefaultPath();

		std::string SerializeIntoJson();
		void SaveToDefaultPath();

		//these are not the default values, the values are only here to avoid undefined behaviour
		EditorStyle Style = EditorStyle::Dark_Gray;
		bool WindowMaximized = false;
		glm::ivec2 WindowSize = { 0, 0 };
		RendererType RenderingBackend = RendererType::OpenGL;
	};
}