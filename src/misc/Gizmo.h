#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"

namespace ALZ {

	class Gizmo
	{
	public:

		enum GizmoArrow {
			None,
			Horizontal,
			Vertical
		};

		Gizmo();
		void Draw(glm::vec2& objectPosition, const glm::vec2& mousePositionNDC);

	private:
		GizmoArrow m_ArrowPressed = None;
	};
}