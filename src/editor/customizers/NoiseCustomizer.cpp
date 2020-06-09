#include <pch.h>

#include "NoiseCustomizer.h"

namespace Ainan {

#define NOISE_TEXTURE_SIZE 256

	NoiseCustomizer::NoiseCustomizer()
	{
		NoisePreviewTexture = Renderer::CreateTexture(glm::vec2(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE), TextureFormat::RGBA, nullptr);
		UpdateNoiseTex();
	}

	void NoiseCustomizer::DisplayGUI()
	{
		if (ImGui::TreeNode("Noise")) 
		{
			ImGui::Text("Enabled: ");
			ImGui::SameLine();
			ImGui::Checkbox("##Enabled:", &m_NoiseEnabled);

			if (m_NoiseEnabled) 
			{
				ImGui::Text("Strength: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Strength: ", &m_NoiseStrength, 0.5f, 0.0f);

				ImGui::Text("Frequency: ");
				ImGui::SameLine();
				if (ImGui::DragFloat("##Frequency: ", &m_NoiseFrequency, 0.001f, 0.0f, 1.0f))
				{
					NoiseLibrary.SetFrequency(m_NoiseFrequency);
					UpdateNoiseTex();
				}

				ImGui::Text("Noise Preview: ");
				ImGui::SameLine();
				ImGui::Image(NoisePreviewTexture->GetTextureID(), ImVec2(128, 128));
			}

			ImGui::TreePop();
		}
	}

	float NoiseCustomizer::GetNoise(const glm::vec2& pos)
	{
		return NoiseLibrary.GetNoise(pos.x, pos.y);
	}

	void NoiseCustomizer::UpdateNoiseTex()
	{
		uint32_t pixelCount = NOISE_TEXTURE_SIZE * NOISE_TEXTURE_SIZE * 4;
		Image img;
		img.m_Width = NOISE_TEXTURE_SIZE;
		img.m_Height= NOISE_TEXTURE_SIZE;
		img.Format= TextureFormat::RGBA;
		img.m_Data = new uint8_t[pixelCount];

		for (size_t x = 0; x < NOISE_TEXTURE_SIZE; x++)
		{
			for (size_t y = 0; y < NOISE_TEXTURE_SIZE; y++)
			{
				float value = NoiseLibrary.GetNoise(x, y);
				//change to uint8_t range
				uint8_t intValue = (value + 1) * 128;

				img.m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 0] = intValue;
				img.m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 1] = intValue;
				img.m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 2] = intValue;
				img.m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 3] = 255;
			}
		}

		//send image to the gpu
		NoisePreviewTexture->SetImage(img);

		delete[] img.m_Data;
		img.m_Data = nullptr;
	}
}

#include "../submodules/FastNoise/FastNoise.cpp"