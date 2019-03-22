#pragma once

#include "ShaderProgram.h"
#include "Window.h"
#include "Particle.h"

class ParticleSystem 
{
public:
	ParticleSystem();
	~ParticleSystem();

	void Update(const float& deltaTime);
	void Draw();
	void SpawnParticle(const Particle& particle);
	void ClearParticles();

	ParticleSystem(const ParticleSystem& Psystem);
	ParticleSystem operator=(const ParticleSystem& Psystem);

	unsigned int& GetActiveParticleCount() { return m_ActiveParticleCount; }

	void* m_ParticleInfoBuffer;
private:
	ShaderProgram m_Shader;
	std::vector<Particle> m_Particles;
	unsigned int m_ParticleCount;
	unsigned int m_ActiveParticleCount;
};