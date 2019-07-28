#pragma once

#include "RenderSurface.h"

namespace ALZ {

	class GaussianBlur {
	public:
		static void Init();
		static void Blur(RenderSurface& surface, float radius);

	private:
		static ShaderProgram* Hblur5x5;
		static ShaderProgram* Vblur5x5;
	};
}