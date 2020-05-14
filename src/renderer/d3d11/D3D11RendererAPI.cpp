#include <pch.h>
#include "D3D11RendererAPI.h"

#ifdef PLATFORM_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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
			D3D11CreateDeviceAndSwapChain(
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
				&DeviceContext);
		}

		D3D11RendererAPI::~D3D11RendererAPI()
		{
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
	}
}



#endif