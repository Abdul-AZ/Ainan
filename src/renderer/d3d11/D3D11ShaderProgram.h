#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/RendererAPI.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;

		class D3D11ShaderProgram : public ShaderProgram
		{
		public:
			D3D11ShaderProgram(const std::string& vertPath, const std::string& fragPath, RendererContext* context);
			virtual ~D3D11ShaderProgram();

			virtual void BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) override;
			virtual void BindUniformBufferUnsafe(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) override;

			virtual void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) override;
			virtual void BindTextureUnsafe(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) override;

			virtual void BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) override;
			virtual void BindTextureUnsafe(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) override;

		public:
			D3D11RendererContext* Context;
			ID3D11VertexShader* VertexShader;
			ID3D11PixelShader* FragmentShader;

			//this is needed for creating vertex buffers
			uint8_t* VertexByteCode = nullptr;
			uint32_t VertexByteCodeSize = 0;

			// Inherited via ShaderProgram
			virtual void BindUniformBuffer(uint32_t identifier, uint32_t bufferAlignedSize, uint32_t slot, RenderingStage stage) override;
		};
	}
}