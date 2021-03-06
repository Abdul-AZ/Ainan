#pragma once

#include "editor/Window.h"
#include "customizers/TextureCustomizer.h"
#include "customizers/VelocityCustomizer.h"
#include "customizers/ColorCustomizer.h"
#include "customizers/ScaleCustomizer.h"
#include "customizers/LifetimeCustomizer.h"
#include "customizers/NoiseCustomizer.h"
#include "customizers/ForceCustomizer.h"

namespace Ainan {

	const glm::vec4 c_ParticleSpawnAreaColor = glm::vec4(0.0f, 0.7f, 0.0f, 0.85f);
	const int32_t   c_CircleVertexCount = 60;

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
		~ParticleCustomizer();

		void DisplayGUI(const std::string& windowName);
		ParticleDescription GetParticleDescription();

		void DrawWorldSpaceUI();

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

		glm::vec2 m_SpawnPosition = { 0.5f, 0.5f };

		//spawn particle on line option
		float m_LineLength = 0.2f;
		float m_LineAngle = 0.0f; //in degrees
		float m_CircleRadius = 0.25f;
		VertexBuffer m_LineVertexBuffer;
		VertexBuffer m_CircleVertexBuffer;
		IndexBuffer m_CircleIndexBuffer;
		UniformBuffer m_SpawnAreaColorUniformBuffer;
		UniformBuffer m_CircleTransformUniformBuffer;

		//random number generator
		std::mt19937 mt;

		friend class ParticleSystem;
	};
}