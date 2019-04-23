#pragma once

namespace ALZ {

	class LifetimeCustomizer
	{
	public:
		LifetimeCustomizer();
		void DisplayGUI();

		float GetLifetime();

	private:
		bool m_RandomLifetime = true;
		float m_DefinedLifetime = 2.0f;
		float m_MinLifetime = 1.0f;
		float m_MaxLifetime = 3.0f;

		//random number generator
		std::mt19937 mt;
	};
}