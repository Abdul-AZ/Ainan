#pragma once
#include <iostream>

#include <glad/glad.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>

class GeneralSettingsGUI {
public:
	
	void DisplayGUI();

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
};
