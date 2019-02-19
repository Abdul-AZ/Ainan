#include <glm/glm.hpp>
#include <Interpolator.h>

class Particle
{
public:
	Particle();

	void Update(const float& deltaTime);

	void SetLifeTime(const float& lifeTime);

	Interpolator<float> m_ScaleInterpolator;

	glm::vec2 m_Position;
	glm::vec4 m_Color;
	glm::vec2 m_Velocity;
	bool isActive;

private:
	float m_LifeTime;
	float m_RemainingLifeTime;

	friend class ParticleSystem;
	friend class ParticleCustomizer;
};