#include <pch.h>

#include "EditorPreferences.h"

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

	//TODO
	std::string EditorPreferences::SerializeIntoJson()
	{
		return "";
	}
}