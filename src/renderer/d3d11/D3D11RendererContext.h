#pragma once

#include "renderer/RendererContext.h"
#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext : public RendererContext
		{
		public:
			virtual RendererType GetType() const override;
			virtual std::string GetVersionString() override;
			virtual std::string GetPhysicalDeviceName() override;

			ID3D11Device* Device;
			ID3D11DeviceContext* DeviceContext;
			ID3D11Texture2D* Backbuffer;
			ID3D11RenderTargetView* BackbufferView;
			IDXGISwapChain* Swapchain;
		};
	}
}