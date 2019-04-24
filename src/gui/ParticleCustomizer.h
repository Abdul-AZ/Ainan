#pragma once

#include "environment/Window.h"
#include "environment/Particle.h"
#include "Customizers/VelocityCustomizer.h"
#include "Customizers/ColorCustomizer.h"
#include "Customizers/ScaleCustomizer.h"
#include "Customizers/LifetimeCustomizer.h"
#include "Customizers/NoiseCustomizer.h"
#include "renderer/Line.h"
#include "renderer/CircleOutline.h"

namespace ALZ {

	enum class SpawnMode {
		SpawnOnMousePosition,
		SpawnOnPoint,
		SpawnOnLine,
		SpawnOnCircle
	};

	class ParticleCustomizer
	{
	public:
		ParticleCustomizer();

		void DisplayGUI(const std::string& windowName, bool& windowOpen);
		void Update();
		Particle& GetParticle();


		float GetTimeBetweenParticles() { return 1 / m_ParticlesPerSecond; }

	public:
		SpawnMode Mode = SpawnMode::SpawnOnMousePosition;

	private:
		float m_ParticlesPerSecond = 100.0f;

		VelocityCustomizer m_VelocityCustomizer;
		NoiseCustomizer m_NoiseCustomizer;
		LifetimeCustomizer m_LifetimeCustomizer;
		ScaleCustomizer m_ScaleCustomizer;
		ColorCustomizer m_ColorCustomizer;

		//this is on a scale from 0 to 1
		glm::vec2 m_SpawnPosition = { 0.5f, 0.5f };

		//the particle that is going to be spawned next
		Particle m_Particle;

		//spawn particle on line option
		glm::vec2 m_LinePosition = { 0.5f, 0.5f };
		float m_LineLength = 0.2f;
		float m_LineAngle = 0.0f; //in degrees
		Line m_Line;

		//spawn particle on circle option
		CircleOutline m_CircleOutline;

		//random number generator
		std::mt19937 mt;

		friend class ParticleSystem;
	};
}