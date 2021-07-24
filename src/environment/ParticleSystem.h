#pragma once

#include "EnvironmentObjectInterface.h"
#include "editor/Window.h"
#include "editor/EditorCamera.h"
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
		void OnTransform() override;
		void SpawnAllParticlesOnQue(const float& deltaTime);
		void SpawnParticle(const ParticleDescription& particle);
		void ClearParticles();
		void DisplayGuiControls() override;
		int32_t GetAllowedGizmoOperation(ImGuizmo::OPERATION operation) override;

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
			std::vector<glm::vec3> Position;
			std::vector<glm::vec3> Velocity;
			std::vector<glm::vec3> Acceleration;
			std::vector<float> StartScale;
			std::vector<float> EndScale;
			std::vector<float> LifeTime;
			std::vector<float> RemainingLifeTime;
		};

		//buffers for rendering data
		std::vector<glm::vec3> m_ParticleDrawTranslationBuffer;
		std::vector<float> m_ParticleDrawScaleBuffer;
		std::vector<glm::vec4> m_ParticleDrawColorBuffer;
		size_t m_ParticleDrawCount = 0;

		ParticlesData m_Particles;
	};
}