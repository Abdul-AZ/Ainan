#pragma once

#include "gui/Interpolator.h"

class ColorCustomizer 
{
public:
	ColorCustomizer();
	void DisplayGUI();

	Interpolator<glm::vec4>& GetColorInterpolator();

private:
	glm::vec4 definedColor = { 1.0f,1.0f,1.0f,1.0f };

	//scale over time
	Interpolator<glm::vec4> m_Interpolator;
	glm::vec4 endColor = definedColor;

	//random number generator
	std::mt19937 mt;
};