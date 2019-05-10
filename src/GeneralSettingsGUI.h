#pragma once

#include "Image.h"
#include "FolderBrowserGUI.h"

namespace ALZ {

	enum class BlendMode {
		Additive,
		Screen
	};

	class GeneralSettingsGUI {
	public:
		void DisplayGUI();

		void DisplayImageSaveLocationBrowser();
		std::string GetImageSaveLocation() { return ImageLocationBrowser.GetChosenFolderPath(); }

	public:
		bool GeneralSettingsWindowOpen = true;

		glm::vec3 BackgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float BaseBackgroundLight = 0.2f;

		//blur
		bool  BlurEnabled       = false;
		float BlurScale         = 1.5f;
		float BlurStrength      = 1.75f;
		float BlurGaussianSigma = 3.0f;

		//image saving
		ImageFormat   ImageFormat      = ImageFormat::png;
		glm::uvec2    ImageResolution  = { 1080, 720 };
		std::string   ImageFileName    = "test";
		FolderBrowser ImageLocationBrowser;

	private:
		BlendMode m_ActiveBlendMode = BlendMode::Additive;
	};
}