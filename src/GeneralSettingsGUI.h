#pragma once
#include <iostream>

#include <glad/glad.h>
#include <imgui/imgui.h>

class GeneralSettingsGUI {
public:
	
	void DisplayGUI();

private:

	void DisplayAllBlendOptions(GLenum& factor);
	void DisplayBlendOption(const GLenum& option, GLenum& factor);

	std::string GetBlendFuncText(const GLenum& func);
	bool m_GLBlendEnabled;
	GLenum m_Sfactor = GL_SRC_ALPHA;
	GLenum m_Dfactor = GL_ONE_MINUS_SRC_ALPHA;
};
