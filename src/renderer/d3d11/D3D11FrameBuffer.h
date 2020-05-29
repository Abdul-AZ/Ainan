#pragma once

#include "D3D11RendererContext.h"
#include "renderer/FrameBuffer.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11FrameBuffer : public FrameBuffer
		{
		public:


			// Inherited via FrameBuffer
			virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) override;

			virtual void SetActiveTexture(const Texture& texture) override;

			virtual glm::vec2 GetSize() const override;

			virtual Image ReadPixels(glm::vec2 bottomLeftPixel = { 0,0 }, glm::vec2 topRightPixel = { 0,0 }) override;

			virtual void Bind() const override;

			virtual void Unbind() const override;

			virtual unsigned int GetRendererID() const override;

		};
	}
}