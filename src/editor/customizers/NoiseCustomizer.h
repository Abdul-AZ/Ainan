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

		void ApplyNoise(glm::vec2& pos, glm::vec2& velocity, glm::vec2& acceleration, uint32_t index = 0);
		float GetNoise(const glm::vec2& pos);

		enum NoiseApplyTarget
		{
			Add_To_Velocity,
			Add_To_Acceleration,
			Set_Velocity_As_Noise,
			Set_Acceleration_As_Noise
		};


		static const char* NoiseApplyTargetStr(NoiseApplyTarget target)
		{
			switch (target)
			{
			case Add_To_Velocity:
				return "Noise Added To Veclocity";
			case Add_To_Acceleration:
				return "Noise Added To Acceleration";
			case Set_Velocity_As_Noise:
				return "Velocity Is Noise";
			case Set_Acceleration_As_Noise:
				return "Acceleration Is Noise";
			default:
				return "";
			}
		}

		static const char* NoiseInterpolationModeStr(FastNoise::Interp target)
		{
			switch (target)
			{
			case FastNoise::Interp::Quintic:
				return "Quintic";
			case FastNoise::Interp::Hermite:
				return "Hermite";
			case FastNoise::Interp::Linear:
				return "Linear";
			default:
				return "";
			}
		}

		static NoiseApplyTarget NoiseApplyTargetVal(std::string str)
		{
			if (str == "Noise Added To Veclocity")
				return NoiseApplyTarget::Add_To_Velocity;
			else if (str == "Noise Added To Acceleration")
				return NoiseApplyTarget::Add_To_Acceleration;
			else if (str == "Velocity Is Noise")
				return NoiseApplyTarget::Set_Velocity_As_Noise;
			if (str == "Acceleration Is Noise")
				return NoiseApplyTarget::Set_Acceleration_As_Noise;

			return NoiseApplyTarget::Add_To_Velocity;
		}

		static FastNoise::Interp NoiseInterpolationModeVal(std::string str)
		{
			if (str == "Quintic")
				return FastNoise::Interp::Quintic;
			else if (str == "Hermite")
				return FastNoise::Interp::Hermite;
			else if (str == "Linear")
				return FastNoise::Interp::Linear;

			return FastNoise::Interp::Quintic;
		}

	public:
		TextureNew NoisePreviewTexture;

	private:
		void UpdateNoiseTex();

	private:
		bool m_NoiseEnabled = false;
		float m_NoiseStrength = 1.0f;
		float m_NoiseFrequency = 0.01f;
		NoiseApplyTarget NoiseTarget = Add_To_Velocity;
		FastNoise::Interp NoiseInterpolationMode = FastNoise::Interp::Quintic;
		FastNoise NoiseLibrary;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}