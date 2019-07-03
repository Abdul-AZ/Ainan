#pragma once

#include "Image.h"
#include "file/SaveItemBrowser.h"

namespace ALZ {

	enum class BlendMode {
		Additive,
		Screen
	};

	class GeneralSettingsGUI {
	public:
		GeneralSettingsGUI();;
		void DisplayGUI();

		std::string GetImageSaveLocation() { return ImageLocationBrowser.m_CurrentselectedFolder; }

	public:
		bool GeneralSettingsWindowOpen = true;

		glm::vec4 BackgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);

		bool ShowGrid = true;

		//blur
		bool  BlurEnabled       = false;
		float BlurScale         = 1.1f;
		float BlurStrength      = 4.0f;
		float BlurGaussianSigma = 4.5f;

		//image saving
		ImageFormat   ImageFormat      = ImageFormat::png;
		glm::uvec2    ImageResolution  = { 1080, 720 };
		//FolderBrowser ImageLocationBrowser;
		SaveItemBrowser ImageLocationBrowser;

	private:
		BlendMode m_ActiveBlendMode = BlendMode::Additive;
	};
}