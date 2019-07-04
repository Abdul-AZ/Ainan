#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"

namespace ALZ {

	class Gizmo
	{
	public:
		Gizmo();
		void Draw(glm::vec2& objectPosition, const glm::vec2& mousePositionNDC);
	};
}