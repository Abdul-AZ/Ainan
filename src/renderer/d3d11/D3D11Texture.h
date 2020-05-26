#pragma once

#include "renderer/Texture.h"
#include "renderer/RendererAPI.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;

		class D3D11Texture : public Texture
		{
		public:
			D3D11Texture(const glm::vec2& size, uint8_t* data, RendererContext* context);
			~D3D11Texture();

			// Inherited via Texture
			virtual void SetImage(const Image& image) override;
			virtual void SetImage(const glm::vec2& size, int comp = 4) override;
			virtual void SetDefaultTextureSettings() override;
			virtual glm::vec2 GetSize() const override;
			virtual void Bind(const int& slot = 0) const override;
			virtual void Unbind(const int& slot = 0) const override;
			virtual unsigned int GetRendererID() const override;

		private:
			ID3D11Texture2D* D3DTexture;
			ID3D11ShaderResourceView* D3DResourceView;
			ID3D11SamplerState* D3DSampler;
			D3D11RendererContext* Context;
		};
	}
}