#pragma once

#include "InterpolationSelector.h"
#include "CurveEditor.h"

namespace ALZ {

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
		float m_LifeTime;
		float m_RemainingLifeTime;

		friend class ParticleSystem;
		friend class ParticleCustomizer;
	};
}