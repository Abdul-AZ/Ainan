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
		void DisplayGUI();

	public:
		bool GeneralSettingsWindowOpen = true;

		bool ShowGrid = true;

		//blur
		bool  BlurEnabled       = false;
		float BlurScale         = 1.1f;
		float BlurStrength      = 4.0f;
		float BlurGaussianSigma = 4.5f;
		float BlurRadius = 1.0f;

	private:
		BlendMode m_ActiveBlendMode = BlendMode::Additive;
	};
}