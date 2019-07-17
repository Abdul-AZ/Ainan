#pragma once

#include "misc/ExposeToJson.h"

namespace ALZ {

	class ForceCustomizer
	{
	public:
		void DisplayGUI();

	private:
		std::map<std::string, glm::vec2> m_Forces;

		bool m_EnableGravity = false;
		float m_GravityStrength = -9.81f;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}