#include <pch.h>

#include "D3D11FrameBuffer.h"

namespace Ainan {
	namespace D3D11 {

		D3D11FrameBuffer::D3D11FrameBuffer(RendererContext* context)
		{
			Context = (D3D11RendererContext*)context;
		}

		void D3D11FrameBuffer::Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize)
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
		void D3D11FrameBuffer::Resize(const glm::vec2& newSize)
		{
		}
	}
}
