#pragma once

#include "RendererAPI.h"

namespace Ainan {

	class Texture;
	class UniformBuffer;
	class FrameBuffer;

	//ONLY create this using Renderer::CreateShaderProgram
	class ShaderProgram
	{
	public:
		virtual void BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) = 0;

	private:
		virtual void BindUniformBufferUnsafe(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTextureUnsafe(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTextureUnsafe(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) = 0;

		friend class Renderer;
	};
}