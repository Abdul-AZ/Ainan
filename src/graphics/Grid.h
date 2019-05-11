#pragma once

#include "ShaderProgram.h"
#include "Camera.h"
#include "Window.h"

namespace ALZ {

	class Grid
	{
	public:
		Grid();
		void Render(Camera& camera);
	};
}