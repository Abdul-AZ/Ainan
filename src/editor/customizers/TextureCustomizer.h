#pragma once

#include "renderer/Renderer.h"
#include "file/FileBrowser.h"
#include "environment/ExposeToJson.h"
#include "file/AssetManager.h"
#include "renderer/Image.h"

namespace Ainan {

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
		std::filesystem::path m_TexturePath = ""; //relative to the environment folder

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}