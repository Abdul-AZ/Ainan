#include "NoiseCustomizer.h"

namespace Ainan {

#define NOISE_TEXTURE_SIZE 256
#define NOISE_WINDOW_INPUT_GUI_START_X 175.0f

	NoiseCustomizer::NoiseCustomizer()
	{
		NoisePreviewTexture = Renderer::CreateTexture(glm::vec2(NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE), TextureFormat::RGBA, TextureType::Texture2D, nullptr);
		NoiseLibrary.SetNoiseType(FastNoise::NoiseType::Perlin);
		UpdateNoiseTex();
	}

	NoiseCustomizer::~NoiseCustomizer()
	{
		Renderer::DestroyTexture(NoisePreviewTexture);
	}

	void NoiseCustomizer::DisplayGUI()
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_SpanAllColumns);
		if (ImGui::TreeNode("Noise"))
		{
			auto spacing = ImGui::GetCursorPosY();
			ImGui::Text("Enabled: ");
			ImGui::NextColumn();
			ImGui::SetCursorPosY(spacing);
			ImGui::Checkbox("##Enabled:", &m_NoiseEnabled);

			if (m_NoiseEnabled) 
			{
				ImGui::NextColumn();
				ImGui::Text("Strength: ");
				ImGui::NextColumn();
				ImGui::DragFloat("##Strength: ", &m_NoiseStrength, 0.5f, 0.0f);

				ImGui::NextColumn();
				ImGui::Text("Frequency: ");
				ImGui::NextColumn();
				if (ImGui::DragFloat("##Frequency: ", &m_NoiseFrequency, 0.001f, 0.0f, 1.0f))
				{
					NoiseLibrary.SetFrequency(m_NoiseFrequency);
					UpdateNoiseTex();
				}

				ImGui::Spacing();

				ImGui::NextColumn();
				IMGUI_DROPDOWN_START_USING_COLUMNS("Noise\nApply Target: ", NoiseApplyTargetStr(NoiseTarget));
				IMGUI_DROPDOWN_SELECTABLE(NoiseTarget, Add_To_Velocity, NoiseApplyTargetStr(Add_To_Velocity));
				IMGUI_DROPDOWN_SELECTABLE(NoiseTarget, Add_To_Acceleration, NoiseApplyTargetStr(Add_To_Acceleration));
				IMGUI_DROPDOWN_SELECTABLE(NoiseTarget, Set_Velocity_As_Noise, NoiseApplyTargetStr(Set_Velocity_As_Noise));
				IMGUI_DROPDOWN_SELECTABLE(NoiseTarget, Set_Acceleration_As_Noise, NoiseApplyTargetStr(Set_Acceleration_As_Noise));
				IMGUI_DROPDOWN_END();

				ImGui::NextColumn();
				auto mode = NoiseInterpolationMode;
				IMGUI_DROPDOWN_START_USING_COLUMNS("Noise\nInterpolation Mode: ", NoiseInterpolationModeStr(NoiseInterpolationMode));
				IMGUI_DROPDOWN_SELECTABLE(NoiseInterpolationMode, FastNoise::Interp::Quintic, NoiseInterpolationModeStr(FastNoise::Interp::Quintic));
				IMGUI_DROPDOWN_SELECTABLE(NoiseInterpolationMode, FastNoise::Interp::Hermite, NoiseInterpolationModeStr(FastNoise::Interp::Hermite));
				IMGUI_DROPDOWN_SELECTABLE(NoiseInterpolationMode, FastNoise::Interp::Linear, NoiseInterpolationModeStr(FastNoise::Interp::Linear));
				IMGUI_DROPDOWN_END();
				//if mode changed
				if (mode != NoiseInterpolationMode)
				{
					ImGui::SetItemDefaultFocus();
					NoiseLibrary.SetInterp(NoiseInterpolationMode);
					UpdateNoiseTex();
				}
				ImGui::Spacing();

				ImGui::NextColumn();
				ImGui::Text("Noise Preview: ");
				ImGui::NextColumn();
				ImGui::Image((void*)NoisePreviewTexture.GetTextureID(), ImVec2(128, 128),
					ImVec2(0,0), ImVec2(1,1),ImVec4(1, 1, 1, 1), ImVec4(0.9, 0.9, 0.9, 1));
			}

			ImGui::NextColumn();
			ImGui::TreePop();
		}
	}

	void NoiseCustomizer::ApplyNoise(glm::vec3& pos, glm::vec3& velocity, glm::vec3& acceleration, uint32_t index)
	{
		if (m_NoiseEnabled == false)
			return;

		//this is so that the noise is different in every particle
		glm::vec3 noiseInput = pos + (float)index * glm::vec3(100, 100, 100);

		switch (NoiseTarget)
		{
		case Add_To_Velocity:
			velocity.x += GetNoise(noiseInput) * m_NoiseStrength;
			velocity.y += GetNoise(noiseInput + glm::vec3(1000)) * m_NoiseStrength;
			velocity.z += GetNoise(noiseInput + glm::vec3(2000)) * m_NoiseStrength;
			return;

		case Add_To_Acceleration:
			acceleration.x += GetNoise(noiseInput) * m_NoiseStrength;
			acceleration.y += GetNoise(noiseInput + glm::vec3(1000)) * m_NoiseStrength;
			acceleration.z += GetNoise(noiseInput + glm::vec3(2000)) * m_NoiseStrength;
			return;

		case NoiseCustomizer::Set_Velocity_As_Noise:
			velocity.x = GetNoise(noiseInput) * m_NoiseStrength;
			velocity.y = GetNoise(noiseInput + glm::vec3(1000)) * m_NoiseStrength;
			velocity.z = GetNoise(noiseInput + glm::vec3(2000)) * m_NoiseStrength;
			return;

		case NoiseCustomizer::Set_Acceleration_As_Noise:
			acceleration.x = GetNoise(noiseInput) * m_NoiseStrength;
			acceleration.y = GetNoise(noiseInput + glm::vec3(1000)) * m_NoiseStrength;
			acceleration.z = GetNoise(noiseInput + glm::vec3(2000)) * m_NoiseStrength;
			return;
		}
	}

	float NoiseCustomizer::GetNoise(const glm::vec3& pos)
	{
		return NoiseLibrary.GetNoise(pos.x, pos.y, pos.z);
	}

	void NoiseCustomizer::UpdateNoiseTex()
	{
		uint32_t pixelCount = NOISE_TEXTURE_SIZE * NOISE_TEXTURE_SIZE * 4;
		auto img = std::make_shared<Image>();
		img->m_Width = NOISE_TEXTURE_SIZE;
		img->m_Height= NOISE_TEXTURE_SIZE;
		img->Format= TextureFormat::RGBA;
		img->m_Data = new uint8_t[pixelCount];

		for (size_t x = 0; x < NOISE_TEXTURE_SIZE; x++)
		{
			for (size_t y = 0; y < NOISE_TEXTURE_SIZE; y++)
			{
				float value = NoiseLibrary.GetNoise(x, y);
				//change to uint8_t range
				uint8_t intValue = (value + 1) * 128;

				img->m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 0] = intValue;
				img->m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 1] = intValue;
				img->m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 2] = intValue;
				img->m_Data[(x * NOISE_TEXTURE_SIZE + y) * 4+ 3] = 255;
			}
		}

		//send image to the gpu
		NoisePreviewTexture.UpdateData(img);
	}
}

#include "../submodules/FastNoise/FastNoise.cpp"