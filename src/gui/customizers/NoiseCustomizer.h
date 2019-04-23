#pragma once

namespace ALZ {

	class NoiseCustomizer
	{
	public:
		void DisplayGUI();

	private:
		bool m_NoiseEnabled = false;
		float m_NoiseStrength = 1.0f;

		friend class ParticleSystem;
	};
}