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
		bool CheckIfInsideArrow(const GizmoArrow& arrow, const glm::vec2& arrowCentre, const glm::vec2& point);

	private:
		GizmoArrow m_ArrowPressed = None;
		float m_ArrowPressLocationDistanceFromArrowOrigin = 0.0f;
	};
}