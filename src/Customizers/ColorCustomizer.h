#pragma once

#include <random>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

class ColorCustomizer 
{
public:
	ColorCustomizer();
	void DisplayGUI();

	glm::vec4 GetColor();

private:

	glm::vec4 definedColor = { 1.0f,1.0f,1.0f,1.0f };

	//random number generator
	std::mt19937 mt;
};