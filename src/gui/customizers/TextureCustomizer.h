#pragma once

#include "renderer/Texture.h"
#include "gui/FileBrowserGUI.h"

namespace ALZ {

	class TextureCustomizer
	{
	public:
		TextureCustomizer();

		void DisplayGUI();

	public:
		bool UseDefaultTexture = true;
		Texture ParticleTexture;

	private:
		FileBrowser m_FileBrowser;
	};
}