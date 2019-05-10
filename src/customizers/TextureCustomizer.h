#pragma once

#include "graphics/Texture.h"
#include "FileBrowserGUI.h"

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