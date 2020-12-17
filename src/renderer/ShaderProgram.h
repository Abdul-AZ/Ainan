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
		virtual void BindUniformBuffer(uint32_t identifier, uint32_t bufferAlignedSize, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) = 0;

	private:
		virtual void BindUniformBufferUnsafe(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTextureUnsafe(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) = 0;
		virtual void BindTextureUnsafe(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) = 0;

		friend class Renderer;
	};

	class UniformBufferNew;
	class ShaderProgramNew
	{
	public:
		//This is used by the renderer to interact with the abstracted uniform buffer
		uint32_t Identifier = 0;

		void BindUniformBuffer(UniformBufferNew buffer, uint32_t slot, RenderingStage stage);
		void BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage);
		void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage);
		void BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage);

		friend class Renderer;
	};

	struct ShaderProgramDataView
	{
		uint64_t Identifier = 0;
	};
}