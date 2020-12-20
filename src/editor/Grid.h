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
		VertexBufferNew m_VertexBuffer;
		IndexBufferNew m_IndexBuffer;
		UniformBufferNew m_TransformUniformBuffer;
	};
}