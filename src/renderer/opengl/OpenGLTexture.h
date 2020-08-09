#pragma once

#include "renderer/Texture.h"
#include "renderer/Image.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLTexture : public Texture
		{
		public:
			OpenGLTexture(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);
			virtual ~OpenGLTexture();

			// Inherited via Texture
			virtual void SetImage(std::shared_ptr<Image> image) override;
			virtual void SetImageUnsafe(std::shared_ptr<Image> image) override;

			virtual glm::vec2 GetSize() const override { return m_Size; }
			virtual void* GetTextureID() override { return (void*)(uintptr_t)m_RendererID; };

		private:
			void inline AllocateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);

		public:
			unsigned int m_RendererID;
			glm::vec2 m_Size = { 0.0f, 0.0f };

			
		};
	}
}