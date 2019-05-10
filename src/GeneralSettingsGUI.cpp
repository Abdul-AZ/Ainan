#include <pch.h>
#include "GeneralSettingsGUI.h"

namespace ALZ {

	static const ImU32 ImageResolutionStep = 1;

	void GeneralSettingsGUI::DisplayGUI()
	{
		if (!GeneralSettingsWindowOpen)
			return;

		ImGui::Begin("Settings", &GeneralSettingsWindowOpen);

		if (ImGui::TreeNode("Blend Settings:"))
		{
			if (ImGui::BeginCombo("Mode", (m_ActiveBlendMode == BlendMode::Additive)? "Additive" : "Screen"))
			{
				{
					bool is_Active = m_ActiveBlendMode == BlendMode::Additive;
					if (ImGui::Selectable("Additive", &is_Active)) {
					
						ImGui::SetItemDefaultFocus();
						m_ActiveBlendMode = BlendMode::Additive;
					}
				}

				{
					bool is_Active = m_ActiveBlendMode == BlendMode::Screen;
					if (ImGui::Selectable("Screen", &is_Active)) {
					
						ImGui::SetItemDefaultFocus();
						m_ActiveBlendMode = BlendMode::Screen;
					}
				}
				glEnable(GL_BLEND);

				if (m_ActiveBlendMode == BlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				else if (m_ActiveBlendMode == BlendMode::Screen)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

				ImGui::EndCombo();

			}

			ImGui::TreePop();
		}

		ImGui::Checkbox("Blur", &BlurEnabled);

		if (BlurEnabled) {
			if (ImGui::TreeNode("Blur Settings:")) {

				ImGui::SliderFloat("Scale: ", &BlurScale, 1.0f, 3.0f);
				ImGui::SliderFloat("Strength: ", &BlurStrength, 1.0f, 5.0f);
				ImGui::SliderFloat("Gaussian Sigma: ", &BlurGaussianSigma, 1.0f, 5.0f);

				ImGui::TreePop();
			}
		}

		if (ImGui::TreeNode("Background:"))
		{
			ImGui::DragFloat("Base Background Light", &BaseBackgroundLight, 0.01f);
			BaseBackgroundLight = std::clamp(BaseBackgroundLight, 0.0f, 1.0f);
			ImGui::ColorPicker4("Background Color", &BackgroundColor.r);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Image Saving:"))
		{
			if (ImGui::BeginCombo("Image Format", Image::GetFormatString(ImageFormat).c_str()))
			{
				bool is_png = ImageFormat == ImageFormat::png ? true : false;
				if (ImGui::Selectable(Image::GetFormatString(ImageFormat::png).c_str(), &is_png)) {

					ImGui::SetItemDefaultFocus();
					ImageFormat = ImageFormat::png;
				}

				bool is_jpeg = ImageFormat == ImageFormat::jpeg ? true : false;
				if (ImGui::Selectable(Image::GetFormatString(ImageFormat::jpeg).c_str(), &is_jpeg)) {

					ImGui::SetItemDefaultFocus();
					ImageFormat = ImageFormat::jpeg;
				}

				bool is_bmp = ImageFormat == ImageFormat::bmp ? true : false;
				if (ImGui::Selectable(Image::GetFormatString(ImageFormat::bmp).c_str(), &is_bmp)) {

					ImGui::SetItemDefaultFocus();
					ImageFormat = ImageFormat::bmp;
				}

				ImGui::EndCombo();
			}

			ImGui::Text("Image Resolution");
			ImGui::InputScalar("width", ImGuiDataType_::ImGuiDataType_U32, &ImageResolution.x, &ImageResolutionStep, &ImageResolutionStep);
			ImGui::InputScalar("height", ImGuiDataType_::ImGuiDataType_U32, &ImageResolution.y, &ImageResolutionStep, &ImageResolutionStep);

			if (ImGui::Button("Change Save Location"))
				DisplayImageSaveLocationBrowser();

			ImGui::InputText("Image Name", &ImageFileName);

			ImGui::TreePop();
		}

		ImGui::End();

		ImageLocationBrowser.DisplayGUI();
	}

	void GeneralSettingsGUI::DisplayImageSaveLocationBrowser()
	{
		ImageLocationBrowser.WindowOpen = true;
	}
}