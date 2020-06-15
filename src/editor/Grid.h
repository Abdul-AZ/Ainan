#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"
#include "Camera.h"
#include "Window.h"

namespace Ainan {

	class Grid
	{
	public:
		Grid(float unitLength = 100.0f, int32_t numLinesPerAxis = 51);
		void Draw(const Camera& camera);

	private:
		float m_UnitLength = 0.0f;
		std::shared_ptr<VertexBuffer> m_VertexBuffer = nullptr;
		std::shared_ptr<IndexBuffer> m_IndexBuffer = nullptr;
		std::shared_ptr<UniformBuffer> m_TransformUniformBuffer = nullptr;
	};
}