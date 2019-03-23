#pragma once

#include "file/Image.h"

class GeneralSettingsGUI {
public:
	
	void DisplayGUI(bool& windowOpen);

	bool GetBlurEnabled() { return m_BlurEnabled; }
	float GetBlurScale() { return m_BlurScale; }
	float GetBlurStrength() { return m_BlurStrength; }
	float GetBlurGaussianSigma() { return m_BlurGaussianSigma; }

	ImageFormat& GetImageFormat() { return m_ImageFormat; }

private:

	//blending
	void DisplayAllBlendOptions(GLenum& factor);
	void DisplayBlendOption(const GLenum& option, GLenum& factor);

	std::string GetBlendFuncText(const GLenum& func);
	bool m_GLBlendEnabled;
	GLenum m_Sfactor = GL_SRC_ALPHA;
	GLenum m_Dfactor = GL_ONE_MINUS_SRC_ALPHA;

	//background
	glm::vec4 m_BackgroundColor = { 0,0,0,0 };

	//blur
	bool m_BlurEnabled = false;
	float m_BlurScale = 1.5f;
	float m_BlurStrength = 1.75f;
	float m_BlurGaussianSigma = 3.0f;

	//image saving
	ImageFormat m_ImageFormat = ImageFormat::png;
};
