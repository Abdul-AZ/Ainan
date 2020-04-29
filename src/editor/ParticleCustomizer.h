#pragma once

#include "Line.h"
#include "CircleOutline.h"
#include "editor/Window.h"
#include "customizers/TextureCustomizer.h"
#include "customizers/VelocityCustomizer.h"
#include "customizers/ColorCustomizer.h"
#include "customizers/ScaleCustomizer.h"
#include "customizers/LifetimeCustomizer.h"
#include "customizers/NoiseCustomizer.h"
#include "customizers/ForceCustomizer.h"

namespace Ainan {

	enum class SpawnMode 
	{
		SpawnOnPoint,
		SpawnOnLine,
		SpawnOnCircle,
		SpawnInsideCircle
	};

	struct ParticleDescription
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec2 Acceleration;
		float StartScale;
		float EndScale;
		float LifeTime;
	};

	std::string GetModeAsText(const SpawnMode& mode);
	SpawnMode GetTextAsMode(const std::string& mode);

	class ParticleCustomizer
	{
	public:
		ParticleCustomizer();

		ParticleCustomizer(const ParticleCustomizer& customizer);
		ParticleCustomizer operator=(const ParticleCustomizer& customizer);

		void DisplayGUI(const std::string& windowName, bool& windowOpen);
		ParticleDescription GetParticleDescription();

		float GetTimeBetweenParticles() { return 1 / m_ParticlesPerSecond; }

	public:
		SpawnMode Mode = SpawnMode::SpawnOnPoint;

	public:
		float m_ParticlesPerSecond = 100.0f;

		VelocityCustomizer m_VelocityCustomizer;
		NoiseCustomizer m_NoiseCustomizer;
		LifetimeCustomizer m_LifetimeCustomizer;
		ScaleCustomizer m_ScaleCustomizer;
		ColorCustomizer m_ColorCustomizer;
		TextureCustomizer m_TextureCustomizer;
		ForceCustomizer m_ForceCustomizer;

		//this is on a scale from 0 to 1
		glm::vec2 m_SpawnPosition = { 0.5f, 0.5f };

		//spawn particle on line option
		glm::vec2 m_LinePosition = { 0.5f, 0.5f };
		float m_LineLength = 0.2f;
		float m_LineAngle = 0.0f; //in degrees
		Line m_Line;

		//spawn particle on/in circle option
		CircleOutline m_CircleOutline;

		//random number generator
		std::mt19937 mt;

		friend class ParticleSystem;
	};
}