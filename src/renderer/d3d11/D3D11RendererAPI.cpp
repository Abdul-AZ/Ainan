#include <pch.h>
#include "D3D11RendererAPI.h"

#ifdef PLATFORM_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

namespace Ainan {
	namespace D3D11 {
		D3D11RendererAPI::D3D11RendererAPI()
		{
			DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
			DXGI_MODE_DESC bufferDesc = {};
			DXGI_RATIONAL refreshRate = {};
			refreshRate.Numerator = 60;
			refreshRate.Denominator = 1;

			bufferDesc.Width = 0;
			bufferDesc.Height = 0;
			bufferDesc.RefreshRate = refreshRate;
			bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
			bufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;

			swapchainDesc.BufferDesc = bufferDesc;
			swapchainDesc.SampleDesc.Count = 4;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 1;
			swapchainDesc.OutputWindow = glfwGetWin32Window(Window::Ptr);
			swapchainDesc.Windowed = 1;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			ASSERT_D3D_CALL(D3D11CreateDeviceAndSwapChain(
				0,
				D3D_DRIVER_TYPE_HARDWARE,
				0,
				D3D11_CREATE_DEVICE_DEBUG,
				0,
				0,
				D3D11_SDK_VERSION,
				&swapchainDesc,
				&Swapchain,
				&Device,
				0,
				&DeviceContext));

			ID3D11Texture2D* backbuffer;
			ASSERT_D3D_CALL(Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer));
			ASSERT_D3D_CALL(Device->CreateRenderTargetView(backbuffer, 0, &BackbufferView));
			backbuffer->Release();

			DeviceContext->OMSetRenderTargets(1, &BackbufferView, 0);

			D3D11_VIEWPORT viewport{};
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = Window::FramebufferSize.x;
			viewport.Height = Window::FramebufferSize.y;
			DeviceContext->RSSetViewports(1, &viewport);
		}

		D3D11RendererAPI::~D3D11RendererAPI()
		{
			BackbufferView->Release();
			Swapchain->Release();
			DeviceContext->Release();
			Device->Release();
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
		{
		}

		void D3D11RendererAPI::DrawInstanced(ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount, const unsigned int& objectCount)
		{
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const IndexBuffer& indexBuffer)
		{
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const IndexBuffer& indexBuffer, int vertexCount)
		{
		}

		void D3D11RendererAPI::ClearScreen()
		{
			float clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
			DeviceContext->ClearRenderTargetView(BackbufferView, clearColor);
		}

		void D3D11RendererAPI::SetViewport(const Rectangle& viewport)
		{
		}

		Rectangle D3D11RendererAPI::GetCurrentViewport()
		{
			return Rectangle();
		}

		void D3D11RendererAPI::SetScissor(const Rectangle& scissor)
		{
		}

		Rectangle D3D11RendererAPI::GetCurrentScissor()
		{
			return Rectangle();
		}

		RendererType D3D11RendererAPI::GetType() const
		{
			return RendererType::D3D11;
		}

		void D3D11RendererAPI::SetBlendMode(RenderingBlendMode blendMode)
		{
		}

		void D3D11RendererAPI::Present()
		{
			Swapchain->Present(0, 0);
		}
	}
}



#endif