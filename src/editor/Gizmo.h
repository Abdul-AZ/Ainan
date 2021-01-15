#pragma once

#include "ViewportWindow.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class Gizmo
	{
	public:
		enum GizmoArrow 
		{
			None,
			Horizontal,
			Vertical
		};

		struct TransformationData
		{
			glm::vec2 Position = glm::vec2(0.0f);
			float AspectRatio = 1.0f;
			float OpacityR = 1.0f;
			float OpacityG = 1.0f;
		};

		Gizmo();
		~Gizmo();
		void Draw(glm::vec2* objectPosition,
			const ViewportWindow& window,
			const Camera& camera);

	private:
		void HandleInteration(float xposNDC, float yposNDC, glm::vec2* objectPosition, const Camera& camera, TransformationData& data);

	private:
		GizmoArrow m_CurrentlyPressedArrow = None;
		float m_DistanceBetweenMouseAndArrowNDC = 0.0f;

		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
		UniformBuffer m_UniformBuffer;
	};
}