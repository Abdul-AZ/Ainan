#pragma once

#include "ParticleSystem.h"
#include "imgui/imgui.h"
#include <random>

class ParticleCustomizer 
{
public:
	ParticleCustomizer();

	void DisplayImGuiCustomizer();
	void Update();
	Particle& GetParticle();

private:

	bool RandomVelocity;
	glm::vec2 minVelocity;
	glm::vec2 maxVelocity;
	glm::vec2 defiendVelocity;

	Particle m_Particle;

	std::mt19937 mt;
};
