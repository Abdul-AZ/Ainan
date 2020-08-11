#pragma once

#include "D3D11RendererContext.h"
#include "renderer/FrameBuffer.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11FrameBuffer : public FrameBuffer
		{
		public:
			D3D11FrameBuffer(const glm::vec2& size, RendererContext* context);
			~D3D11FrameBuffer();

			// Inherited via FrameBuffer
			virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) override;
			virtual Image ReadPixels(glm::vec2 bottomLeftPixel = { 0,0 }, glm::vec2 topRightPixel = { 0,0 }) override;
			virtual void Bind() const override;
			virtual void BindUnsafe() const override;
			virtual void Resize(const glm::vec2& newSize) override;
			virtual void ResizeUnsafe(const glm::vec2& newSize) override;

			virtual glm::vec2 GetSize() const override { return Size; };
			virtual void* GetTextureID() override { return RenderTargetTextureView; };

		public:
			D3D11RendererContext* Context;
			ID3D11Texture2D* RenderTargetTexture;
			ID3D11ShaderResourceView* RenderTargetTextureView;
			ID3D11SamplerState* RenderTargetTextureSampler;
			ID3D11RenderTargetView* RenderTargetView;
			glm::vec2 Size;
		};
	}
}