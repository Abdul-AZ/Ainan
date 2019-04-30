#pragma once

#include "renderer/ShaderProgram.h"
#include "Window.h"
#include "Particle.h"
#include "Camera.h"
#include "../gui/ParticleCustomizer.h"
#include "noise/PerlinNoise2D.h"
#include "InspectorInterface.h"
#include "renderer/Texture.h"

namespace ALZ {

	class ParticleSystem : public InspectorInterface
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		void Update(const float& deltaTime, Camera& camera) override;
		void Render(Camera& camera) override;
		void SpawnAllParticlesOnQue(const float& deltaTime, Camera& camera);
		void SpawnParticle(const Particle& particle);
		void ClearParticles();
		void DisplayGUI(Camera& camera) override;

		ParticleSystem(const ParticleSystem& Psystem);
		ParticleSystem operator=(const ParticleSystem& Psystem);

	public:
		void* m_ParticleInfoBuffer;
		float m_TimeTillNextParticleSpawn = 0.0f;
		ParticleCustomizer m_Customizer;
		unsigned int m_ActiveParticleCount = 0;

		//only for spawning on mouse press
		bool m_ShouldSpawnParticles;
	private:
		std::vector<Particle> m_Particles;
		unsigned int m_ParticleCount;
		PerlinNoise2D m_Noise;
	};
}