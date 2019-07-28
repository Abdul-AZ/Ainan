#pragma once

#include "RenderSurface.h"

namespace ALZ {

	class GaussianBlur {
	public:
		static void Init();
		static void Blur(RenderSurface& surface, const float& scale, const float& blurScale, const float& sigma);
		static void Blur(RenderSurface& surface, float radius);
		
		static float GaussianDistribution(float x, float sigma);

	private:
		static ShaderProgram* Hblur5x5;
		static ShaderProgram* Vblur5x5;
	};
}