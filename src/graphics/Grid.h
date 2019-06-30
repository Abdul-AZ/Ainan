#pragma once

#include "ShaderProgram.h"
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"

namespace ALZ {

	class Grid
	{
	public:
		Grid();
		void Render(Camera& camera);
	};
}