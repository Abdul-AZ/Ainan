#include <pch.h>
#include <glad/glad.h>

#include "OpenGLFrameBuffer.h"

#include "renderer/Renderer.h"

namespace Ainan {
	namespace OpenGL {

		OpenGLFrameBuffer::OpenGLFrameBuffer(const glm::vec2& size) :
			m_Size(size)
		{
			glGenFramebuffers(1, &m_RendererID);
			glGenTextures(1, &m_TextureID);

			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
			glBindTexture(GL_TEXTURE_2D, m_TextureID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		OpenGLFrameBuffer::~OpenGLFrameBuffer()
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_TextureID);
		}

		void OpenGLFrameBuffer::Bind() const
		{
			auto func = [this]()
			{
				BindUnsafe();
			};
			Renderer::PushCommand(func);
		}

		void OpenGLFrameBuffer::BindUnsafe() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		}

		void OpenGLFrameBuffer::Resize(const glm::vec2& newSize)
		{
			auto func = [this, newSize]()
			{
				ResizeUnsafe(newSize);
			};
			Renderer::PushCommand(func);
		}

		void OpenGLFrameBuffer::ResizeUnsafe(const glm::vec2& newSize)
		{
			m_Size = newSize;
			glBindTexture(GL_TEXTURE_2D, m_TextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, newSize.x, newSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		Image OpenGLFrameBuffer::ReadPixels(glm::vec2 bottomLeftPixel, glm::vec2 topRightPixel)
		{
			Image image;
			image.m_Width = (uint32_t)m_Size.x;
			image.m_Height = (uint32_t)m_Size.y;
			image.m_Data = new uint8_t[image.m_Width * image.m_Height * 4];
			image.Format = TextureFormat::RGBA;

			BindUnsafe();
			glReadPixels(bottomLeftPixel.x,
				bottomLeftPixel.y,
				topRightPixel.x == 0 ? m_Size.x : topRightPixel.x,
				topRightPixel.y == 0 ? m_Size.y : topRightPixel.y,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				image.m_Data);

			return image;
		}

		void OpenGLFrameBuffer::Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize)
		{
			OpenGLFrameBuffer* targetBuffer = (OpenGLFrameBuffer*)otherBuffer;
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, otherBuffer ? targetBuffer->m_RendererID : 0);

			glBlitFramebuffer(0, 0, (GLint)sourceSize.x, (GLint)sourceSize.y,
				0, 0, (GLint)targetSize.x, (GLint)targetSize.y,
				GL_COLOR_BUFFER_BIT,
				GL_LINEAR);

			m_Size = targetSize;
		}
	}
}