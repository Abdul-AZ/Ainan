#pragma once

#include <random>
#include <glm/glm.hpp>
#include <imgui.h>

class VelocityCustomizer 
{
public:
	VelocityCustomizer();
	void DisplayGUI();

	glm::vec2 GetVelocity();

private:
	bool RandomVelocity = true;
	glm::vec2 minVelocity = { -100, -100 };
	glm::vec2 maxVelocity = { 100, 100 };
	glm::vec2 definedVelocity = { 100, -100 };

	//random number generator
	std::mt19937 mt;
};