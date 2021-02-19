#pragma once

#include "EnvironmentObjectInterface.h"
#include "editor/Window.h"
#include "editor/Camera.h"
#include "editor/ParticleCustomizer.h"
#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class ParticleSystem : public EnvironmentObjectInterface
	{
		const size_t c_ParticlePoolSize = 3000;

	public:
		ParticleSystem();
		~ParticleSystem();

		void Update(const float deltaTime) override;
		void Draw() override;
		void SpawnAllParticlesOnQue(const float& deltaTime);
		void SpawnParticle(const ParticleDescription& particle);
		void ClearParticles();
		void DisplayGUI() override;

		ParticleSystem(const ParticleSystem& Psystem);
		ParticleSystem operator=(const ParticleSystem& Psystem);

	public:
		ParticleCustomizer Customizer;
		//only for spawning on mouse press
		bool ShouldSpawnParticles;
		float TimeTillNextParticleSpawn = 0.0f;
		uint32_t ActiveParticleCount = 0;

	private:
		//data for each particles
		//NOTE the size of these vectors is c_ParticlePoolSize
		struct ParticlesData
		{
			std::vector<bool> IsActive;
			std::vector<glm::vec2> Position;
			std::vector<glm::vec2> Velocity;
			std::vector<glm::vec2> Acceleration;
			std::vector<float> StartScale;
			std::vector<float> EndScale;
			std::vector<float> LifeTime;
			std::vector<float> RemainingLifeTime;
		};

		//buffers for rendering data
		std::vector<glm::vec2> m_ParticleDrawTranslationBuffer;
		std::vector<float> m_ParticleDrawScaleBuffer;
		std::vector<glm::vec4> m_ParticleDrawColorBuffer;
		size_t m_ParticleDrawCount = 0;

		ParticlesData m_Particles;
	};
}