#pragma once

#include "RendererAPI.h"

namespace Ainan {

	class Texture;
	class UniformBuffer;

	//ONLY create this using Renderer::CreateShaderProgram
	class ShaderProgram
	{
	public:
		virtual void BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) = 0;
		
		virtual int GetUniformLocation(const char* name) = 0;
		virtual int GetRendererID() const = 0;
	};
}