#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class ParticleSystem;
}

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

		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, const int& objectOrder);
	};
}