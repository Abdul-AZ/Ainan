#include <pch.h>

#include "D3D11FrameBuffer.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

#include "renderer/Renderer.h"

namespace Ainan {
	namespace D3D11 {

		D3D11FrameBuffer::D3D11FrameBuffer(const glm::vec2& size, RendererContext* context) :
			Size(size)
		{
			Context = (D3D11RendererContext*)context;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = size.x;
			desc.Height = size.y;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, nullptr, &RenderTargetTexture));

			D3D11_RENDER_TARGET_VIEW_DESC viewDesc{};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

			ASSERT_D3D_CALL(Context->Device->CreateRenderTargetView(RenderTargetTexture, &viewDesc, &RenderTargetView));

			D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc{};
			textureViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.Texture2D.MipLevels = 1;

			ASSERT_D3D_CALL(Context->Device->CreateShaderResourceView(RenderTargetTexture, &textureViewDesc, &RenderTargetTextureView));

			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

			ASSERT_D3D_CALL(Context->Device->CreateSamplerState(&samplerDesc, &RenderTargetTextureSampler));
		}

		D3D11FrameBuffer::~D3D11FrameBuffer()
		{
			RenderTargetTextureSampler->Release();
			RenderTargetTextureView->Release();
			RenderTargetView->Release();
			RenderTargetTexture->Release();
		}

		void D3D11FrameBuffer::Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize)
		{
			if (otherBuffer)
			{
				D3D11FrameBuffer* d3dotherBuffer = (D3D11FrameBuffer*)otherBuffer;
				Context->DeviceContext->CopyResource(d3dotherBuffer->RenderTargetTexture, RenderTargetTexture);
			}
			else
			{
				Context->DeviceContext->CopyResource(Context->Backbuffer, RenderTargetTexture);
			}
		}

		Image Ainan::D3D11::D3D11FrameBuffer::ReadPixels(glm::vec2 bottomLeftPixel, glm::vec2 topRightPixel)
		{
			//create a staging texture
			ID3D11Texture2D* stagingTexture = nullptr;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = Size.x;
			desc.Height = Size.y;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, nullptr, &stagingTexture));

			//TODO support cpying parts of the framebuffer and not just all of it
			Context->DeviceContext->CopyResource(stagingTexture, RenderTargetTexture);

			D3D11_MAPPED_SUBRESOURCE resource{};

			Context->DeviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &resource);
			Image img;

			img.Format = TextureFormat::RGBA;
			img.m_Width = Size.x;
			img.m_Height = Size.y;
			img.m_Data = new uint8_t[Size.x * Size.y * 4];

			uint8_t* srcPtr = (uint8_t*)resource.pData;
			uint8_t* destPtr = (uint8_t*)img.m_Data;
			const uint32_t unpaddedRowSize = Size.x * 4;
			for (size_t i = 0; i < Size.y - 1; i++)
			{
				memcpy(destPtr, srcPtr, unpaddedRowSize);
				if (i == Size.y - 2)
					break;
				srcPtr += resource.RowPitch;
				destPtr += unpaddedRowSize;
			}

			Context->DeviceContext->Unmap(stagingTexture, 0);
			stagingTexture->Release();

			return img;
		}

		void D3D11FrameBuffer::Bind() const
		{
			auto func = [this]()
			{
				BindUnsafe();
			};
			Renderer::PushCommand(func);
		}

		void D3D11FrameBuffer::BindUnsafe() const
		{
			Context->DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
		}

		void D3D11FrameBuffer::Resize(const glm::vec2& newSize)
		{
			auto func = [this, newSize]()
			{
				ResizeUnsafe(newSize);
			};
			Renderer::PushCommand(func);
		}

		void D3D11FrameBuffer::ResizeUnsafe(const glm::vec2& newSize)
		{
			RenderTargetView->Release();
			RenderTargetTexture->Release();
			RenderTargetTextureView->Release();

			Size = newSize;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = newSize.x;
			desc.Height = newSize.y;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ASSERT_D3D_CALL(Context->Device->CreateTexture2D(&desc, nullptr, &RenderTargetTexture));

			D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc{};
			textureViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.Texture2D.MipLevels = 1;

			ASSERT_D3D_CALL(Context->Device->CreateShaderResourceView(RenderTargetTexture, &textureViewDesc, &RenderTargetTextureView));

			D3D11_RENDER_TARGET_VIEW_DESC viewDesc{};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

			ASSERT_D3D_CALL(Context->Device->CreateRenderTargetView(RenderTargetTexture, &viewDesc, &RenderTargetView));
		}
	}
}
