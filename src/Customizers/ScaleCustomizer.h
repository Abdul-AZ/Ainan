#pragma once

#include <random>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

class ScaleCustomizer 
{
public:
	ScaleCustomizer();
	void DisplayGUI();

	float GetScale();

private:
	//starting scale
	bool RandomScale = true;
	float definedScale = 2.0f;
	float minScale = 1.0f;
	float maxScale = 3.0f;

	//random number generator
	std::mt19937 mt;
};