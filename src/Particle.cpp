#include "Particle.h"

Particle::Particle() :
	m_Position(0.0f, 0.0f),
	m_Color(0.0f, 0.0f, 0.0f, 0.0f),
	m_Velocity(0.0f, 0.0f),
	isActive(false),
	m_LifeTime(0.0f),
	m_RemainingLifeTime(0.0f),
	m_ScaleInterpolator(InterpolatorMode::Fixed, 0.0f, 0.0f)
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
