#include <pch.h>
#include "Particle.h"

namespace ALZ {

	Particle::Particle() :
		m_Position(0.0f, 0.0f),
		m_ColorInterpolator(InterpolationType::Fixed, glm::vec4(0, 0, 0, 0), glm::vec4(0, 0, 0, 0)),
		m_Velocity(0.0f, 0.0f),
		isActive(false),
		m_LifeTime(0.0f),
		m_RemainingLifeTime(0.0f),
		m_ScaleInterpolator(InterpolationType::Fixed, 0.0f, 0.0f)
	{}

	void Particle::Update(const float & deltaTime)
	{
		m_Position += m_Velocity * deltaTime;

		m_RemainingLifeTime -= deltaTime;
		if (m_RemainingLifeTime < 0.0f)
			isActive = false;

	}

	void Particle::SetLifeTime(const float & lifeTime)
	{
		m_LifeTime = lifeTime;
		m_RemainingLifeTime = lifeTime;
	}
}