#include <pch.h>

#include "D3D11RendererAPI.h"

#include "renderer/IndexBuffer.h"

#ifdef PLATFORM_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "D3D11ShaderProgram.h"


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
			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 2;
			swapchainDesc.OutputWindow = glfwGetWin32Window(Window::Ptr);
			swapchainDesc.Windowed = 1;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
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
				&Context.Swapchain,
				&Context.Device,
				0,
				&Context.DeviceContext));

			ID3D11Texture2D* backbuffer;
			ASSERT_D3D_CALL(Context.Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer));
			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView(backbuffer, 0, &Context.BackbufferView));
			backbuffer->Release();

			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, 0);

			Rectangle viewport{};
			viewport.Width = Window::FramebufferSize.x;
			viewport.Height = Window::FramebufferSize.y;
			SetViewport(viewport);
		}

		constexpr D3D11_PRIMITIVE_TOPOLOGY GetD3DPrimitive(Primitive primitive)
		{
			switch (primitive)
			{
			case Primitive::Triangles:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				break;

			case Primitive::TriangleFan:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
				break;

			case Primitive::Lines:
				return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
				break;

			default:
				assert(false);
				return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
		}

		D3D11RendererAPI::~D3D11RendererAPI()
		{
			Context.BackbufferView->Release();
			Context.Swapchain->Release();
			Context.DeviceContext->Release();
			Context.Device->Release();
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
		{
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const IndexBuffer& indexBuffer)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(mode));

			D3D11ShaderProgram* d3dShader = (D3D11ShaderProgram*)&shader;;

			Context.DeviceContext->VSSetShader(d3dShader->VertexShader, 0, 0);
			Context.DeviceContext->PSSetShader(d3dShader->FragmentShader, 0, 0);

			Context.DeviceContext->DrawIndexed(indexBuffer.GetCount(), 0, 0);
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const IndexBuffer& indexBuffer, int vertexCount)
		{
		}

		void D3D11RendererAPI::ClearScreen()
		{
			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			Context.DeviceContext->ClearRenderTargetView(Context.BackbufferView, clearColor);
		}

		void D3D11RendererAPI::SetViewport(const Rectangle& viewport)
		{
			D3D11_VIEWPORT d3d_viewport{};
			d3d_viewport.TopLeftX = 0;
			d3d_viewport.TopLeftY = 0;
			d3d_viewport.Width = viewport.Width;
			d3d_viewport.Height = viewport.Height;
			Context.DeviceContext->RSSetViewports(1, &d3d_viewport);
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

		void D3D11RendererAPI::SetBlendMode(RenderingBlendMode blendMode)
		{
		}

		void D3D11RendererAPI::SetRenderTargetApplicationWindow()
		{
			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, nullptr);
		}

		void D3D11RendererAPI::Present()
		{
			Context.Swapchain->Present(1, 0);
			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, nullptr);
		}

		void D3D11RendererAPI::RecreateSwapchain(const glm::vec2& newSwapchainSize)
		{
			Context.DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
			Context.BackbufferView->Release();
			Context.DeviceContext->Flush();

			Context.Swapchain->ResizeBuffers(2, newSwapchainSize.x, newSwapchainSize.y, DXGI_FORMAT_UNKNOWN, 0);

			ID3D11Texture2D* backbuffer;
			ASSERT_D3D_CALL(Context.Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer));
			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView(backbuffer, 0, &Context.BackbufferView));
			backbuffer->Release();

			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, 0);
		}
	}
}



#endif