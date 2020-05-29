#include <pch.h>

#include "D3D11FrameBuffer.h"

namespace Ainan {
	namespace D3D11 {

		void D3D11FrameBuffer::Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize)
		{
		}

		void D3D11FrameBuffer::SetActiveTexture(const Texture& texture)
		{
		}

		glm::vec2 D3D11FrameBuffer::GetSize() const
		{
			return glm::vec2();
		}

		Image Ainan::D3D11::D3D11FrameBuffer::ReadPixels(glm::vec2 bottomLeftPixel, glm::vec2 topRightPixel)
		{
			return Image();
		}

		void D3D11FrameBuffer::Bind() const
		{
		}

		void D3D11FrameBuffer::Unbind() const
		{
		}

		unsigned int D3D11FrameBuffer::GetRendererID() const
		{
			return 0;
		}
	}
}
