#include <pch.h>
#include "GeneralSettingsGUI.h"

namespace ALZ {

	static const ImU32 ImageResolutionStep = 1;

	void GeneralSettingsGUI::DisplayGUI()
	{
		if (!GeneralSettingsWindowOpen)
			return;

		ImGui::Begin("Settings", &GeneralSettingsWindowOpen);

		if (ImGui::Checkbox("Blend", &m_GLBlendEnabled)) {
			if (m_GLBlendEnabled)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);
		}

		if (m_GLBlendEnabled) {

			if (ImGui::TreeNode("Blend Settings:"))
			{
				if (ImGui::BeginCombo("sFactor", GetBlendFuncText(m_Sfactor).c_str())) {

					DisplayAllBlendOptions(m_Sfactor);
					glBlendFunc(m_Sfactor, m_Dfactor);

					ImGui::EndCombo();
				}

				if (ImGui::BeginCombo("dFactor", GetBlendFuncText(m_Dfactor).c_str())) {

					DisplayAllBlendOptions(m_Dfactor);
					glBlendFunc(m_Sfactor, m_Dfactor);

					ImGui::EndCombo();
				}

				ImGui::TreePop();
			}
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

	void GeneralSettingsGUI::DisplayAllBlendOptions(GLenum& factor)
	{
		DisplayBlendOption(GL_SRC_ALPHA, factor);
		DisplayBlendOption(GL_ONE_MINUS_SRC_ALPHA, factor);
		DisplayBlendOption(GL_DST_ALPHA, factor);
		DisplayBlendOption(GL_ONE_MINUS_DST_ALPHA, factor);
		DisplayBlendOption(GL_SRC_COLOR, factor);
		DisplayBlendOption(GL_ONE_MINUS_SRC_COLOR, factor);
		DisplayBlendOption(GL_DST_COLOR, factor);
		DisplayBlendOption(GL_ONE_MINUS_DST_COLOR, factor);
		DisplayBlendOption(GL_CONSTANT_COLOR, factor);
		DisplayBlendOption(GL_ONE_MINUS_CONSTANT_COLOR, factor);
		DisplayBlendOption(GL_CONSTANT_ALPHA, factor);
		DisplayBlendOption(GL_ONE_MINUS_CONSTANT_ALPHA, factor);
		DisplayBlendOption(GL_SRC_ALPHA_SATURATE, factor);
		DisplayBlendOption(GL_SRC1_ALPHA, factor);
		DisplayBlendOption(GL_ONE_MINUS_SRC1_ALPHA, factor);
		DisplayBlendOption(GL_SRC1_COLOR, factor);
		DisplayBlendOption(GL_ONE_MINUS_SRC1_COLOR, factor);
		DisplayBlendOption(GL_ZERO, factor);
		DisplayBlendOption(GL_ONE, factor);
	}

	void GeneralSettingsGUI::DisplayBlendOption(const GLenum& option, GLenum& factor)
	{
		bool is_Active = m_Sfactor == option;
		if (ImGui::Selectable(GetBlendFuncText(option).c_str(), &is_Active)) {

			ImGui::SetItemDefaultFocus();
			factor = option;
		}
	}

	std::string GeneralSettingsGUI::GetBlendFuncText(const GLenum & func)
	{
		switch (func)
		{
		case GL_ZERO:
			return "Zero";

		case GL_ONE:
			return "One";

		case GL_SRC_COLOR:
			return "Source Color";

		case GL_ONE_MINUS_SRC_COLOR:
			return "1 - Source Color";

		case GL_DST_COLOR:
			return "Destination Color";

		case GL_ONE_MINUS_DST_COLOR:
			return "1 - Destination Color";

		case GL_SRC_ALPHA:
			return "Source Alpha";

		case GL_ONE_MINUS_SRC_ALPHA:
			return "1 - Source Alpha";

		case GL_DST_ALPHA:
			return "Destination Alpha";

		case GL_ONE_MINUS_DST_ALPHA:
			return "1 - Destination Alpha";

		case GL_CONSTANT_COLOR:
			return "Constant Color";

		case GL_ONE_MINUS_CONSTANT_COLOR:
			return "1 - Constant Color";

		case GL_CONSTANT_ALPHA:
			return "Constant Alpha";

		case GL_ONE_MINUS_CONSTANT_ALPHA:
			return "1 - Constant Alpha";

		case GL_SRC_ALPHA_SATURATE:
			return "Source Alpha Saturate";

		case GL_SRC1_COLOR:
			return "Source(1) Color";

		case GL_ONE_MINUS_SRC1_COLOR:
			return "1 - Source(1) Color";

		case GL_SRC1_ALPHA:
			return "Source(1) Alpha";

		case GL_ONE_MINUS_SRC1_ALPHA:
			return "1 - Source(1) Alpha";

		default:
			return std::string();
			break;
		}
	}
}