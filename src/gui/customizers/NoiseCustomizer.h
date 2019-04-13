#pragma once

namespace ALZ {

	class NoiseCustomizer
	{
	public:
		void DisplayGUI();

	private:
		bool NoiseEnabled = false;
		float NoiseStrength = 1.0f;

		friend class ParticleSystem;
	};
}