#include <pch.h>

#include "NoiseCustomizer.h"

namespace Ainan {

#define NOISE_TEXTURE_SIZE 256
#define NOISE_WINDOW_INPUT_GUI_START_X 175.0f

	NoiseCustomizer::NoiseCustomizer()
	{
		NoisePreviewTexture = Renderer::CreateTexture(glm::vec2(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE), TextureFormat::RGBA, nullptr);
		NoiseLibrary.SetNoiseType(FastNoise::NoiseType::Perlin);
		UpdateNoiseTex();
	}

#define SET_GUI_POS_INPUT() ImGui::SameLine();\
		ImGui::SetCursorPosX(NOISE_WINDOW_INPUT_GUI_START_X)

	void NoiseCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		if (ImGui::TreeNode("Noise"))
		{
			ImGui::Text("Enabled: ");
			SET_GUI_POS_INPUT();
			ImGui::Checkbox("##Enabled:", &m_NoiseEnabled);

			if (m_NoiseEnabled) 
			{

				ImGui::Text("Strength: ");
				SET_GUI_POS_INPUT();
				ImGui::DragFloat("##Strength: ", &m_NoiseStrength, 0.5f, 0.0f);

				ImGui::Text("Frequency: ");
				SET_GUI_POS_INPUT();
				if (ImGui::DragFloat("##Frequency: ", &m_NoiseFrequency, 0.001f, 0.0f, 1.0f))
				{
					NoiseLibrary.SetFrequency(m_NoiseFrequency);
					UpdateNoiseTex();
				}

				ImGui::Spacing();

				ImGui::Text("Noise\nApply Target: ");
				SET_GUI_POS_INPUT();

				if (ImGui::BeginCombo("##Noise nApply Target: ", NoiseApplyTargetStr(NoiseTarget)))
				{
					{
						bool is_active = NoiseTarget == Add_To_Velocity;
						if (ImGui::Selectable(NoiseApplyTargetStr(Add_To_Velocity), &is_active)) {
							ImGui::SetItemDefaultFocus();
							NoiseTarget = Add_To_Velocity;
						}
					}

					{
						bool is_active = NoiseTarget == Add_To_Acceleration;
						if (ImGui::Selectable(NoiseApplyTargetStr(Add_To_Acceleration), &is_active)) {
							ImGui::SetItemDefaultFocus();
							NoiseTarget = Add_To_Acceleration;
						}
					}

					{
						bool is_active = NoiseTarget == Set_Velocity_As_Noise;
						if (ImGui::Selectable(NoiseApplyTargetStr(Set_Velocity_As_Noise), &is_active)) {
							ImGui::SetItemDefaultFocus();
							NoiseTarget = Set_Velocity_As_Noise;
						}
					}

					{
						bool is_active = NoiseTarget == Set_Acceleration_As_Noise;
						if (ImGui::Selectable(NoiseApplyTargetStr(Set_Acceleration_As_Noise), &is_active)) {
							ImGui::SetItemDefaultFocus();
							NoiseTarget = Set_Acceleration_As_Noise;
						}
					}

					ImGui::EndCombo();
				}


				ImGui::Text("Noise\nInterpolation Mode: ");
				SET_GUI_POS_INPUT();

				if (ImGui::BeginCombo("##Noise Interpolation Mode: ", NoiseInterpolationModeStr(NoiseInterpolationMode)))
				{
					{
						auto mode = FastNoise::Interp::Quintic;
						bool is_active = NoiseInterpolationMode == mode;
						if (ImGui::Selectable(NoiseInterpolationModeStr(mode), &is_active))
						{
							ImGui::SetItemDefaultFocus();
							NoiseInterpolationMode = mode;
							NoiseLibrary.SetInterp(mode);
							UpdateNoiseTex();
						}
					}

					{
						auto mode = FastNoise::Interp::Hermite;
						bool is_active = NoiseInterpolationMode == mode;
						if (ImGui::Selectable(NoiseInterpolationModeStr(mode), &is_active))
						{
							ImGui::SetItemDefaultFocus();
							NoiseInterpolationMode = mode;
							NoiseLibrary.SetInterp(mode);
							UpdateNoiseTex();
						}
					}

					{
						auto mode = FastNoise::Interp::Linear;
						bool is_active = NoiseInterpolationMode == mode;
						if (ImGui::Selectable(NoiseInterpolationModeStr(mode), &is_active))
						{
							ImGui::SetItemDefaultFocus();
							NoiseInterpolationMode = mode;
							NoiseLibrary.SetInterp(mode);
							UpdateNoiseTex();
						}
					}

					ImGui::EndCombo();
				}

				ImGui::Spacing();

				ImGui::Text("Noise Preview: ");
				SET_GUI_POS_INPUT();
				ImGui::Image(NoisePreviewTexture->GetTextureID(), ImVec2(128, 128),
					ImVec2(0,0), ImVec2(1,1),ImVec4(1, 1, 1, 1), ImVec4(0.9, 0.9, 0.9, 1));
			}

			ImGui::TreePop();
		}
	}

	void NoiseCustomizer::ApplyNoise(glm::vec2& pos, glm::vec2& velocity, glm::vec2& acceleration, uint32_t index)
	{
		if (m_NoiseEnabled == false)
			return;

		//this is so that the noise is different in every particle
		glm::vec2 noiseInput = pos + (float)index * glm::vec2(100, 100);

		switch (NoiseTarget)
		{
		case Add_To_Velocity:
			velocity.x += GetNoise(noiseInput) * m_NoiseStrength;
			velocity.y += GetNoise(-noiseInput) * m_NoiseStrength;
			return;

		case Add_To_Acceleration:
			acceleration.x += GetNoise(noiseInput) * m_NoiseStrength;
			acceleration.y += GetNoise(-noiseInput) * m_NoiseStrength;
			return;

		case Ainan::NoiseCustomizer::Set_Velocity_As_Noise:
			velocity.x = GetNoise(noiseInput) * m_NoiseStrength;
			velocity.y = GetNoise(-noiseInput) * m_NoiseStrength;
			return;

		case Ainan::NoiseCustomizer::Set_Acceleration_As_Noise:
			acceleration.x = GetNoise(noiseInput) * m_NoiseStrength;
			acceleration.y = GetNoise(-noiseInput) * m_NoiseStrength;
			return;
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