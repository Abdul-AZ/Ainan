#pragma once

#include "renderer/Texture.h"
#include "renderer/RendererAPI.h"
#include "renderer/Image.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;

		class D3D11Texture : public Texture
		{
		public:
			D3D11Texture(const glm::vec2& size, TextureFormat format, uint8_t* data, RendererContext* context);
			virtual ~D3D11Texture();

			virtual void SetImage(std::shared_ptr<Image> image) override;
			virtual void SetImageUnsafe(std::shared_ptr<Image> image) override;
			virtual glm::vec2 GetSize() const override;
			virtual void* GetTextureID() override { return D3DResourceView; };

		public:
			ID3D11Texture2D* D3DTexture;
			ID3D11ShaderResourceView* D3DResourceView;
			ID3D11SamplerState* D3DSampler;
			D3D11RendererContext* Context;
		};
	}
}