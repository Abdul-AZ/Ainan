#pragma once

#include "renderer/ShaderProgram.h"
#include "misc/Window.h"
#include "misc/Particle.h"
#include "misc/Camera.h"
#include "misc/ParticleCustomizer.h"
#include "misc/PerlinNoise2D.h"
#include "misc/InspectorInterface.h"
#include "renderer/Renderer.h"

namespace ALZ {

	class ParticleSystem : public InspectorInterface
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		void Update(const float& deltaTime) override;
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
		void* m_ParticleInfoBuffer;

		std::vector<Particle> m_Particles;
		unsigned int m_ParticleCount;
		PerlinNoise2D m_Noise;
	};
}