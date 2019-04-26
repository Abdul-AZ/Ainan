#pragma once

#include "file/Image.h"
#include "gui/FolderBrowserGUI.h"

namespace ALZ {

	class GeneralSettingsGUI {
	public:
		void DisplayGUI();

		void DisplayImageSaveLocationBrowser();
		std::string GetImageSaveLocation() { return ImageLocationBrowser.GetChosenFolderPath(); }

	public:
		bool GeneralSettingsWindowOpen = true;

		glm::vec4 BackgroundColor = { 1.0f, 1.0f, 1.0f, 1.0f };
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
		//blending
		void DisplayAllBlendOptions(GLenum& factor);
		void DisplayBlendOption(const GLenum& option, GLenum& factor);
		std::string GetBlendFuncText(const GLenum& func);

	private:
		bool m_GLBlendEnabled;
		GLenum m_Sfactor = GL_SRC_ALPHA;
		GLenum m_Dfactor = GL_ONE_MINUS_SRC_ALPHA;
	};
}