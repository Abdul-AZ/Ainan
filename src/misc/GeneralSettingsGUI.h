#pragma once

#include "Image.h"
#include "file/FolderBrowser.h"

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

		glm::vec4 BackgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.2f);

		bool ShowGrid = true;

		//blur
		bool  BlurEnabled       = false;
		float BlurScale         = 1.1f;
		float BlurStrength      = 3.6f;
		float BlurGaussianSigma = 1.3f;

		//image saving
		ImageFormat   ImageFormat      = ImageFormat::png;
		glm::uvec2    ImageResolution  = { 1080, 720 };
		std::string   ImageFileName    = "test";
		FolderBrowser ImageLocationBrowser;

	private:
		BlendMode m_ActiveBlendMode = BlendMode::Additive;
	};
}