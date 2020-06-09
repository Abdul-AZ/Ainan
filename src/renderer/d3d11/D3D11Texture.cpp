#include <pch.h>

#include "D3D11Texture.h"

#include "D3D11RendererContext.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

#include "renderer/Image.h"

namespace Ainan {
	namespace D3D11 {
		DXGI_FORMAT D3DFormat(TextureFormat format)
		{
			switch (format)
			{
			case Ainan::TextureFormat::RGBA:
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			case Ainan::TextureFormat::RGB:
				assert(false, "Format not supported");

			case Ainan::TextureFormat::RG:
				return DXGI_FORMAT_R8G8_UNORM;

			case Ainan::TextureFormat::R:
				return DXGI_FORMAT_R8_UNORM;

			case Ainan::TextureFormat::Unspecified:
				return DXGI_FORMAT_UNKNOWN;

			default:
				assert(false);
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		int32_t FormatBytesPerPixel(TextureFormat format)
		{
			switch (format)
			{
			case Ainan::TextureFormat::RGBA:
				return 4;

			case Ainan::TextureFormat::RGB:
				assert(false, "Format not supported");

			case Ainan::TextureFormat::RG:
				return 2;

			case Ainan::TextureFormat::R:
				return 1;

			case Ainan::TextureFormat::Unspecified:
				return 0;

			default:
				assert(false);
				return 0;
			}
		}

		D3D11Texture::D3D11Texture(const glm::vec2& size, TextureFormat format, uint8_t* data, RendererContext* context)
		{
			Context = (D3D11RendererContext*)context;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = size.x;
			desc.Height = size.y;
			desc.SampleDesc.Count = 1;
			//TODO pass dynamic parameter
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = D3DFormat(format);

			if (data)
			{
				D3D11_SUBRESOURCE_DATA subresource{};
				subresource.pSysMem = data;
				subresource.SysMemPitch = size.x * sizeof(uint8_t) * FormatBytesPerPixel(format);
				ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, &subresource, &D3DTexture));
			}
			else
				ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, nullptr, &D3DTexture));

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = 1;
			viewDesc.Format = D3DFormat(format);
			
			ASSERT_D3D_CALL(Context->Device->CreateShaderResourceView(D3DTexture, &viewDesc, &D3DResourceView));

			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[3] = 1.0f;
			samplerDesc.MaxLOD = 1.0f;

			ASSERT_D3D_CALL(Context->Device->CreateSamplerState(&samplerDesc, &D3DSampler));
		}

		D3D11Texture::~D3D11Texture()
		{
			D3DSampler->Release();
			D3DResourceView->Release();
			D3DTexture->Release();
		}

		void D3D11Texture::SetImage(const Image& image)
		{
			D3DTexture->Release();
			D3DResourceView->Release();

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = image.m_Width;
			desc.Height = image.m_Height;
			desc.Format = D3DFormat(image.Format);
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;

			if (image.m_Data)
			{
				D3D11_SUBRESOURCE_DATA subresource{};
				subresource.pSysMem = image.m_Data;
				subresource.SysMemPitch = image.m_Width * sizeof(uint8_t) * FormatBytesPerPixel(image.Format);
				ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, &subresource, &D3DTexture));
			}
			else
				ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, nullptr, &D3DTexture));

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = 1;
			viewDesc.Format = D3DFormat(image.Format);

			ASSERT_D3D_CALL(Context->Device->CreateShaderResourceView(D3DTexture, &viewDesc, &D3DResourceView));
		}

		void D3D11Texture::SetDefaultTextureSettings()
		{
		}

		glm::vec2 D3D11Texture::GetSize() const
		{
			return glm::vec2();
		}
	}
}