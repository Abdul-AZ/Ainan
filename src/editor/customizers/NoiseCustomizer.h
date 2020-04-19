#pragma once

#include "environment/ExposeToJson.h"

namespace Ainan {

	class NoiseCustomizer
	{
	public:
		void DisplayGUI();

	private:
		bool m_NoiseEnabled = false;
		float m_NoiseStrength = 1.0f;

		EXPOSE_CUSTOMIZER_TO_JSON
	};
}