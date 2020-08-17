#include <pch.h>
#include <glad/glad.h>

#include "OpenGLTexture.h"
#include "renderer/Renderer.h"

namespace Ainan {
	namespace OpenGL {

		OpenGLTexture::OpenGLTexture(const glm::vec2& size, TextureFormat format, uint8_t* data)
		{
			glGenTextures(1, &m_RendererID);

			AllocateTexture(size, format, data);
		}

		OpenGLTexture::~OpenGLTexture()
		{
			glDeleteTextures(1, &m_RendererID);
		}

		inline void OpenGLTexture::AllocateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data)
		{
			glBindTexture(GL_TEXTURE_2D, m_RendererID);

			switch (format)
			{
			case TextureFormat::RGBA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				m_AllocatedGPUMem = 4 * size.x * size.y;
				break;

			case TextureFormat::RGB:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				m_AllocatedGPUMem = 3 * size.x * size.y;
				break;

			case TextureFormat::RG:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, size.x, size.y, 0, GL_RG, GL_UNSIGNED_BYTE, data);
				m_AllocatedGPUMem = 2 * size.x * size.y;
				break;

			case TextureFormat::R:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, size.x, size.y, 0, GL_RED, GL_UNSIGNED_BYTE, data);
				m_AllocatedGPUMem = size.x * size.y;
				break;

			case TextureFormat::Unspecified:
				assert(false);
			}

			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		void OpenGLTexture::SetImage(std::shared_ptr<Image> image)
		{
			auto func = [this, image]()
			{
				SetImageUnsafe(image);
			};
			Renderer::PushCommand(func);
		}

		void OpenGLTexture::SetImageUnsafe(std::shared_ptr<Image> image)
		{
			AllocateTexture({ image->m_Width, image->m_Height }, image->Format, image->m_Data);
		}
	}
}