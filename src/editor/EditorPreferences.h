#pragma once

#include "EditorStyles.h"
#include "renderer/Renderer.h"

namespace Ainan {

	struct EditorPreferences
	{
		static EditorPreferences Default();
		std::string SerializeIntoJson();

		//these are not the default values, the values are only here to avoid undefined behaviour
		EditorStyle Style = EditorStyle::Dark_Gray;
		bool WindowMaximized = false;
		glm::ivec2 WindowSize = { 0,0 };
		RendererType RenderingBackend = RendererType::OpenGL;
	};
}