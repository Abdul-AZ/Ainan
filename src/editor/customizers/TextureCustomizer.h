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
		std::string m_TexturePath = "";

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}