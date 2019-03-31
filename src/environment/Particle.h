#pragma once

#include "gui/Interpolator.h"

class Particle
{
public:
	Particle();

	void Update(const float& deltaTime);

	void SetLifeTime(const float& lifeTime);

	Interpolator<float> m_ScaleInterpolator;
	Interpolator<glm::vec4> m_Color;

	glm::vec2 m_Position;
	glm::vec2 m_Velocity;
	bool isActive;

private:
	float m_LifeTime;
	float m_RemainingLifeTime;

	friend class ParticleSystem;
	friend class ParticleCustomizer;
};