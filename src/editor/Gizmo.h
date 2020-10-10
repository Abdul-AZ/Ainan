#pragma once

#include "renderer/ShaderProgram.h"
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
		void Draw(glm::vec2* objectPosition,
			const glm::vec2& viewportWindowPos,
			const glm::vec2& viewportWindowSize,
			const glm::vec2& viewportWindowContentRegionSize,
			const Camera& camera);

	private:
		void HandleInteration(float xposNDC, float yposNDC, glm::vec2* objectPosition, const Camera& camera, TransformationData& data);

	private:
		GizmoArrow m_CurrentlyPressedArrow = None;
		float m_DistanceBetweenMouseAndArrowNDC = 0.0f;

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		std::shared_ptr<UniformBuffer> m_UniformBuffer;
	};
}