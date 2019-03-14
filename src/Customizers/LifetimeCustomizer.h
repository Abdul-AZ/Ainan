#pragma once

#include <random>
#include <glm/glm.hpp>
#include <imgui.h>

class LifetimeCustomizer 
{
public:
	LifetimeCustomizer();
	void DisplayGUI();

	float GetLifetime();

private:
	bool RandomLifetime = true;
	float definedLifetime = 2.0f;
	float minLifetime = 1.0f;
	float maxLifetime = 3.0f;
	
	//random number generator
	std::mt19937 mt;
};