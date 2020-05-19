#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class Gizmo
	{
	public:

		enum GizmoArrow {
			None,
			Horizontal,
			Vertical
		};

		Gizmo();
		void Draw(glm::vec2& objectPosition, const Rectangle& viewport);

	private:
		bool CheckIfInsideArrow(const GizmoArrow& arrow, const glm::vec2& arrowCentre, const glm::vec2& point);

	private:
		std::unordered_map<void*, GizmoArrow> m_ArrowPressedMap;
		std::unordered_map<void*, float> m_ArrowPressLocationDistanceFromArrowOriginMap;

		std::shared_ptr<IndexBuffer> EBO;
		std::shared_ptr<VertexBuffer> VBO;
	};
}