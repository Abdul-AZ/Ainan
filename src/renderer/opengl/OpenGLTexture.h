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

			virtual void SetImage(std::shared_ptr<Image> image) override;
			virtual void SetImageUnsafe(std::shared_ptr<Image> image) override;

			virtual uint32_t GetMemorySize() const override { return m_AllocatedGPUMem; };
			virtual void* GetTextureID() override           { return (void*)(uintptr_t)m_RendererID; };

		private:
			void inline AllocateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);

		public:
			uint32_t m_RendererID;
			uint32_t m_AllocatedGPUMem = 0;
		};
	}
}