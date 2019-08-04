#pragma once

#include "misc/ExposeToJson.h"

namespace ALZ {

	struct Force 
	{
		glm::vec2 Value = { 0.0f,0.0f };
		bool Enabled = false;
	};

	class ForceCustomizer
	{
	public:
		ForceCustomizer();
		void DisplayGUI();

	private:
		std::map<std::string, Force> m_Forces;

		bool m_DisplayAddForceSettings = false;
		std::string m_AddForceInputString = "";
		std::string m_CurrentSelectedForceName = "";

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}