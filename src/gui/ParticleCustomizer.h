#pragma once

#include "environment/Window.h"
#include "environment/Particle.h"
#include "gui/Customizers/VelocityCustomizer.h"
#include "gui/Customizers/ColorCustomizer.h"
#include "gui/Customizers/ScaleCustomizer.h"
#include "gui/Customizers/LifetimeCustomizer.h"
#include "renderer/Line.h"

enum class SpawnMode {
	SpawnOnMousePosition,
	SpawnOnPoint,
	SpawnOnLine
};

class ParticleCustomizer 
{
public:
	ParticleCustomizer();

	void DisplayGUI(const std::string& windowName, bool& windowOpen);
	void Update();
	Particle& GetParticle();

	SpawnMode m_Mode = SpawnMode::SpawnOnMousePosition;

	float GetTimeBetweenParticles() { return 1 / m_ParticlesPerSecond; }

private:
	float m_ParticlesPerSecond = 100.0f;

	VelocityCustomizer m_VelocityCustomizer;
	LifetimeCustomizer m_LifetimeCustomizer;
	ScaleCustomizer m_ScaleCustomizer;
	ColorCustomizer m_ColorCustomizer;

	//this is on a scale from 0 to 1
	glm::vec2 m_SpawnPosition = { 0.5f, 0.5f };

	//the particle that is going to be spawned next
	Particle m_Particle;

	//spawn particle on line option
	glm::vec2 m_LinePosition = { 0.5f, 0.5f };
	float m_LineLength = 0.01f;
	float m_LineAngle = 0.0f; //in degrees
	Line m_Line;

	//random number generator
	std::mt19937 mt;

	friend class ParticleSystem;
};
