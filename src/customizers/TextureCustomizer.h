#pragma once

//for declaring friend to json serializer
#include "json/json_fwd.hpp"
namespace ALZ {
	class Environment;
	class ParticleSystem;
}

#include "renderer/Renderer.h"
#include "file/FileBrowser.h"

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

		friend void toJson(nlohmann::json& j, const ParticleSystem& ps, const int& objectOrder);
		friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);
	};
}