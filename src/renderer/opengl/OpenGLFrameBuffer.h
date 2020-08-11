#pragma once

#include "renderer/Texture.h"
#include "renderer/FrameBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLFrameBuffer : public FrameBuffer
		{
		public:
			OpenGLFrameBuffer(const glm::vec2& size);
			~OpenGLFrameBuffer();

			virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) override;
			virtual Image ReadPixels(glm::vec2 bottomLeftPixel = { 0,0 }, glm::vec2 topRightPixel = { 0,0 }) override;

			virtual glm::vec2 GetSize() const override { return m_Size; }
			virtual void Resize(const glm::vec2& newSize) override;
			virtual void ResizeUnsafe(const glm::vec2& newSize) override;
			virtual void* GetTextureID() override { return (void*)(uintptr_t)m_TextureID; };

			virtual void Bind() const override;
			virtual void BindUnsafe() const override;
		public:
			uint32_t m_RendererID = 0;
			uint32_t m_TextureID = 0;
			glm::vec2 m_Size = { 0.0f, 0.0f };
		};
	}
}