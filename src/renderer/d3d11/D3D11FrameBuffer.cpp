#include <pch.h>

#include "D3D11FrameBuffer.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

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
			return Image();
		}

		void D3D11FrameBuffer::Bind() const
		{
			Context->DeviceContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
		}

		void D3D11FrameBuffer::Resize(const glm::vec2& newSize)
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
