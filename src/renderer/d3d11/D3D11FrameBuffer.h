#pragma once

#include "D3D11RendererContext.h"
#include "renderer/FrameBuffer.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11FrameBuffer : public FrameBuffer
		{
		public:
			D3D11FrameBuffer(RendererContext* context);

			// Inherited via FrameBuffer
			virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) override;

			virtual glm::vec2 GetSize() const override;

			virtual Image ReadPixels(glm::vec2 bottomLeftPixel = { 0,0 }, glm::vec2 topRightPixel = { 0,0 }) override;

			virtual void Bind() const override;

		public:
			D3D11RendererContext* Context;

			// Inherited via FrameBuffer
			virtual void Resize(const glm::vec2& newSize) override;
		};
	}
}