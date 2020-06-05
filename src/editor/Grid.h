#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"
#include "Camera.h"
#include "Window.h"

namespace Ainan {

	class Grid
	{
	public:
		Grid();
		void Draw();

	private:
		std::shared_ptr<VertexBuffer> VBO = nullptr;
		std::shared_ptr<IndexBuffer> EBO = nullptr;
		std::shared_ptr<UniformBuffer> ColorUniformBuffer = nullptr;
	};
}