#pragma once

#include "editor/Window.h"

namespace Ainan {

	class AppStatusWindow
	{
	public:
		void DisplayGUI();
		void Update(float deltaTime);

		void SetText(const std::string& text, float time = 1.0f);

	private:
		std::string m_Text = "";
		float m_DisplayTime = 0.0f;
	};
}