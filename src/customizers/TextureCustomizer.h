#pragma once

#include "renderer/Renderer.h"
#include "file/FileBrowser.h"
#include "misc/ExposeToJson.h"

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
		std::unique_ptr<Texture> ParticleTexture;

	private:
		FileBrowser m_FileBrowser;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}