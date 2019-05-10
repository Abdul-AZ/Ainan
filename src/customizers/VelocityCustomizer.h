#pragma once

namespace ALZ {

	class VelocityCustomizer
	{
	public:
		VelocityCustomizer();
		void DisplayGUI();

		glm::vec2 GetVelocity();

	private:
		bool m_RandomVelocity = true;
		glm::vec2 m_MinVelocity = { -100, -100 };
		glm::vec2 m_MaxVelocity = { 100, 100 };
		glm::vec2 m_DefinedVelocity = { 100, -100 };

		//random number generator
		std::mt19937 mt;
	};
}