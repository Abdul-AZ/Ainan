#pragma once

#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

private:
	ShaderProgram m_Shader;
	std::vector<Particle> m_Particles;
	void* m_ParticleInfoBuffer;
	unsigned int m_ParticleCount;
};