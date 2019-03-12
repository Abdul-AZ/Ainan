#pragma once

#include <random>
#include "ParticleSystem.h"
#include "imgui/imgui.h"
#include <Customizers/VelocityCustomizer.h>
#include <Customizers/ColorCustomizer.h>
#include <Customizers/ScaleCustomizer.h>
#include <Customizers/LifetimeCustomizer.h>

class ParticleCustomizer 
{
public:
	ParticleCustomizer();

	void DisplayGUI();
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
