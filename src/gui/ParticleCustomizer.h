#pragma once

#include "environment/ParticleSystem.h"
#include "gui/Customizers/VelocityCustomizer.h"
#include "gui/Customizers/ColorCustomizer.h"
#include "gui/Customizers/ScaleCustomizer.h"
#include "gui/Customizers/LifetimeCustomizer.h"

enum class SpawnMode {
	SpawnOnMousePosition,
	SpawnOnPosition
};

class ParticleCustomizer 
{
public:
	ParticleCustomizer();

	void DisplayGUI(const std::string& windowName, bool& windowOpen);
	void Update();
	Particle& GetParticle();

	SpawnMode m_Mode = SpawnMode::SpawnOnMousePosition;
private:

	VelocityCustomizer m_VelocityCustomizer;
	LifetimeCustomizer m_LifetimeCustomizer;
	ScaleCustomizer m_ScaleCustomizer;
	ColorCustomizer m_ColorCustomizer;

	//this is on a scale from 0 to 1000
	glm::vec2 m_SpawnPosition = { 500.0f, 500.0f };

	//the particle that is going to be spawned next
	Particle m_Particle;

	//random number generator
	std::mt19937 mt;
};
