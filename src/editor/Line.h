#pragma once

#include "renderer/ShaderProgram.h"
#include "editor/Camera.h"
#include "editor/Window.h"
#include "renderer/Renderer.h"

namespace Ainan {

	class Line
	{
	public:
		Line();

		void Draw();
		void SetVertices(const std::vector<glm::vec2> vertices);

		float GetSlope(size_t startVertex = 0, size_t endVertex = 1);
		float GetYIntercept(size_t startVertex = 0, size_t endVertex = 1);
		glm::vec2 GetPointInLine(const float t, size_t startVertex = 0, size_t endVertex = 1);

	public:
		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	private:
		std::vector<glm::vec2> m_Vertices;

		std::shared_ptr<VertexBuffer> VBO = nullptr;

		friend class ParticleCustomizer;
	};
}