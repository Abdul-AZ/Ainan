#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/UniformBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLShaderProgram : public ShaderProgram
		{
		public:
			OpenGLShaderProgram(const std::string& vertPath, const std::string& fragPath);
			OpenGLShaderProgram() { m_RendererID = 0; }
			static std::shared_ptr<OpenGLShaderProgram> CreateRaw(const std::string& vertSrc, const std::string& fragSrc);
			~OpenGLShaderProgram();

			virtual void BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) override;
			virtual void BindUniformBufferUnsafe(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) override;

			virtual void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) override;
			virtual void BindTextureUnsafe(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) override;

			virtual void BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) override;
			virtual void BindTextureUnsafe(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) override;
		public:
			uint32_t m_RendererID;

			// Inherited via ShaderProgram
			virtual void BindUniformBuffer(uint32_t identifier, uint32_t bufferAlignedSize, uint32_t slot, RenderingStage stage) override;
		};
	}
}