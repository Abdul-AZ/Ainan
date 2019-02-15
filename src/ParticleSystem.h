#pragma once

#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderProgram.h"

struct Particle 
{
	Particle() {}

	Particle(const glm::vec2& position, const glm::vec4& color) :
		m_Position(position),
		m_Color(color),
		m_Velocity(0.0f,0.0f),
		isActive(true),
		m_Scale(0)
	{}

	void Update(const float& deltaTime) {
		m_Position += m_Velocity * deltaTime;

		m_RemainingLifeTime -= deltaTime;
		if (m_RemainingLifeTime < 0.0f)
			isActive = false;

	}

	void SetLifeTime(const float& lifeTime) {
		m_LifeTime = lifeTime;
		m_RemainingLifeTime = lifeTime;
	}

	float m_Scale;
	glm::vec2 m_Position;
	glm::vec4 m_Color;

	glm::vec2 m_Velocity;

	bool isActive;


private:
	float m_LifeTime;
	float m_RemainingLifeTime;

	friend class ParticleSystem;
};

class ParticleSystem 
{
public:
	ParticleSystem();

	void Update(const float& deltaTime);
	void Draw();
	void SpawnParticle(const Particle& particle);
	void ClearParticles();

public:
	float Scale;
private:
	ShaderProgram m_Shader;
	std::vector<Particle> m_Particles;
};