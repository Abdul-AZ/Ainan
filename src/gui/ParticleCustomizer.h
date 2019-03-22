#pragma once

#include "environment/ParticleSystem.h"
#include "gui/Customizers/VelocityCustomizer.h"
#include "gui/Customizers/ColorCustomizer.h"
#include "gui/Customizers/ScaleCustomizer.h"
#include "gui/Customizers/LifetimeCustomizer.h"

class ParticleCustomizer 
{
public:
	ParticleCustomizer();

	void DisplayGUI(const std::string& windowName);
	void Update();
	Particle& GetParticle();

private:

	VelocityCustomizer m_VelocityCustomizer;
	LifetimeCustomizer m_LifetimeCustomizer;
	ScaleCustomizer m_ScaleCustomizer;
	ColorCustomizer m_ColorCustomizer;

	//the particle that is going to be spawned next
	Particle m_Particle;

	//random number generator
	std::mt19937 mt;
};
