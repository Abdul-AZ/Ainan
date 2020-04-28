#pragma once

#include "editor/InterpolationSelector.h"
#include "editor/CurveEditor.h"

namespace Ainan {

	class Particle
	{
	public:
		Particle();

		void Update(const float& deltaTime);

		void SetLifeTime(const float& lifeTime);

		InterpolationSelector<float> m_ScaleInterpolator;
		InterpolationSelector<glm::vec4> m_ColorInterpolator;
		CurveEditor CustomScaleCurve;

		bool isActive;

	private:
		glm::vec2 m_Position;
		glm::vec2 m_Velocity;
		glm::vec2 m_Acceleration = { 0.0f,0.0f };
		float m_LifeTime;
		float m_RemainingLifeTime;

		friend class ParticleSystem;
		friend class ParticleCustomizer;
	};
}