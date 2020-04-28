#pragma once

#include "EnvironmentObjectInterface.h"
#include "Particle.h"
#include "math/PerlinNoise2D.h"
#include "editor/Window.h"
#include "editor/Camera.h"
#include "editor/ParticleCustomizer.h"
#include "editor/Gizmo.h"
#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class ParticleSystem : public EnvironmentObjectInterface
	{
	public:
		ParticleSystem();

		void Update(const float deltaTime) override;
		void Draw() override;
		void SpawnAllParticlesOnQue(const float& deltaTime);
		void SpawnParticle(const Particle& particle);
		void ClearParticles();
		void DisplayGUI() override;

		glm::vec2& GetPositionRef() override;
		ParticleSystem(const ParticleSystem& Psystem);
		ParticleSystem operator=(const ParticleSystem& Psystem);

	public:
		ParticleCustomizer Customizer;
		//only for spawning on mouse press
		bool ShouldSpawnParticles;
		float TimeTillNextParticleSpawn = 0.0f;
		unsigned int ActiveParticleCount = 0;

	private:
		//these are the buffers for all the particles that will be drawn this frame
		std::vector<glm::vec2> m_ParticleDrawTranslationBuffer;
		std::vector<float> m_ParticleDrawScaleBuffer;
		std::vector<glm::vec4> m_ParticleDrawColorBuffer;
		size_t m_ParticleDrawCount = 0;

		std::vector<Particle> m_Particles;
		unsigned int m_ParticleCount;
		PerlinNoise2D m_Noise;
	};
}