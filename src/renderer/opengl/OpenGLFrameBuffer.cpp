#include <pch.h>
#include <glad/glad.h>

#include "OpenGLFrameBuffer.h"

namespace Ainan {
	namespace OpenGL {

		OpenGLFrameBuffer::OpenGLFrameBuffer()
		{
			glGenFramebuffers(1, &m_RendererID);
		}

		OpenGLFrameBuffer::~OpenGLFrameBuffer()
		{
			glDeleteFramebuffers(1, &m_RendererID);
		}

		void OpenGLFrameBuffer::Bind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		}

		void OpenGLFrameBuffer::Unbind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void OpenGLFrameBuffer::SetActiveTexture(const Texture& texture)
		{
			Bind();
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.GetRendererID(), 0);
			m_Size = texture.GetSize();
		}

		Image OpenGLFrameBuffer::ReadPixels(glm::vec2 bottomLeftPixel, glm::vec2 topRightPixel)
		{
			Image image;
			image.m_Width = (unsigned int)m_Size.x;
			image.m_Height = (unsigned int)m_Size.y;
			image.m_Data = new unsigned char[image.m_Width * image.m_Height * 4];
			image.Format = TextureFormat::RGBA;

			Bind();
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
			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, otherBuffer ? otherBuffer->GetRendererID() : 0);

			glBlitFramebuffer(0, 0, (GLint)sourceSize.x, (GLint)sourceSize.y,
				0, 0, (GLint)targetSize.x, (GLint)targetSize.y,
				GL_COLOR_BUFFER_BIT,
				GL_LINEAR);

			m_Size = targetSize;
		}
	}
}