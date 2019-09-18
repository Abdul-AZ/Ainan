#pragma once

#include "renderer/Renderer.h"
#include "file/FileBrowser.h"
#include "misc/ExposeToJson.h"
#include "misc/AssetManager.h"

namespace ALZ {

	class TextureCustomizer
	{
	public:
		TextureCustomizer();
		TextureCustomizer(const TextureCustomizer& customizer);
		TextureCustomizer operator=(const TextureCustomizer& customizer);

		void DisplayGUI();

	public:
		bool UseDefaultTexture = true;
		std::shared_ptr<Texture> ParticleTexture;
		std::string m_CurrentTexture = "";

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}