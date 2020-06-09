#pragma once

#include "environment/ExposeToJson.h"

#include "renderer/Renderer.h"
#include "../submodules/FastNoise/FastNoise.h"

namespace Ainan {

	class NoiseCustomizer
	{
	public:
		NoiseCustomizer();
		void DisplayGUI();

		float GetNoise(const glm::vec2& pos);

		std::shared_ptr<Texture> NoisePreviewTexture;

	private:
		void UpdateNoiseTex();

	private:
		bool m_NoiseEnabled = false;
		float m_NoiseStrength = 1.0f;
		float m_NoiseFrequency = 0.01f;

		FastNoise NoiseLibrary;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}