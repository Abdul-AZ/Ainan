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
		~Grid();
		void Draw(const Camera& camera);

	private:
		float m_UnitLength = 0.0f;
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
		UniformBuffer m_TransformUniformBuffer;
	};
}