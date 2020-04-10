#pragma once

#include "renderer/Texture.h"
#include "renderer/FrameBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLFrameBuffer : public FrameBuffer
		{
		public:
			OpenGLFrameBuffer();
			~OpenGLFrameBuffer();

			// Inherited via FrameBuffer
			virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) override;

			virtual void Bind() const override;
			virtual void Unbind() const override;
			virtual unsigned int GetRendererID() const override { return m_RendererID; }

		private:
			unsigned int m_RendererID;
			glm::vec2 m_Size = { 0.0f, 0.0f };

			virtual void SetActiveTexture(const Texture& texture) override;

			// Inherited via FrameBuffer
			virtual glm::vec2 GetSize() const override { return m_Size; }
		};
	}
}