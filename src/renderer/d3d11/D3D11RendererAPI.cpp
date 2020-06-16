#include <pch.h>

#include "D3D11RendererAPI.h"

#include "renderer/IndexBuffer.h"

#ifdef PLATFORM_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "D3D11ShaderProgram.h"

#include "imgui_impl_dx11.cpp"
#include "renderer/d3d11/D3D11RendererAPI.h"
#include "imgui_impl_glfw.cpp"


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
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchainDesc.Flags = 0;
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

			ASSERT_D3D_CALL(Context.Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Context.Backbuffer));
			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView(Context.Backbuffer, 0, &Context.BackbufferView));

			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, 0);

			Rectangle viewport{};
			viewport.Width = Window::FramebufferSize.x;
			viewport.Height = Window::FramebufferSize.y;
			SetViewport(viewport);


			D3D11_RENDER_TARGET_BLEND_DESC additiveBlendDesc{};
			additiveBlendDesc.BlendEnable = true;
			additiveBlendDesc.SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			additiveBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			additiveBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			additiveBlendDesc.DestBlend = D3D11_BLEND_ONE;
			additiveBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
			additiveBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			additiveBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_RENDER_TARGET_BLEND_DESC screenBlendDesc{};
			screenBlendDesc.BlendEnable = false;
			screenBlendDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
			screenBlendDesc.SrcBlend = D3D11_BLEND_ZERO;
			screenBlendDesc.DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			screenBlendDesc.DestBlend = D3D11_BLEND_DEST_COLOR;
			screenBlendDesc.BlendOp = D3D11_BLEND_OP_MAX;
			screenBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_MAX;
			screenBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_BLEND_DESC blendDesc{};
			blendDesc.RenderTarget[0] = additiveBlendDesc; 
			blendDesc.AlphaToCoverageEnable = 0;
			blendDesc.IndependentBlendEnable = 0;

			ASSERT_D3D_CALL(Context.Device->CreateBlendState(&blendDesc, &AdditiveBlendMode));

			blendDesc.RenderTarget[0] = screenBlendDesc;
			ASSERT_D3D_CALL(Context.Device->CreateBlendState(&blendDesc, &ScreenBlendMode));
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
			ImGui_ImplDX11_Shutdown();
			AdditiveBlendMode->Release();
			ScreenBlendMode->Release();
			Context.BackbufferView->Release();
			Context.Backbuffer->Release();
			Context.Swapchain->Release();
			Context.DeviceContext->Release();
			Context.Device->Release();
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, const Primitive& mode, const unsigned int& vertexCount)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(mode));

			D3D11ShaderProgram* d3dShader = (D3D11ShaderProgram*)&shader;;

			Context.DeviceContext->VSSetShader(d3dShader->VertexShader, 0, 0);
			Context.DeviceContext->PSSetShader(d3dShader->FragmentShader, 0, 0);

			Context.DeviceContext->Draw(vertexCount, 0);
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
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(mode));

			D3D11ShaderProgram* d3dShader = (D3D11ShaderProgram*)&shader;;

			Context.DeviceContext->VSSetShader(d3dShader->VertexShader, 0, 0);
			Context.DeviceContext->PSSetShader(d3dShader->FragmentShader, 0, 0);
			
			Context.DeviceContext->DrawIndexed(vertexCount, 0, 0);
		}

		void D3D11RendererAPI::ClearScreen()
		{
			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			Context.DeviceContext->ClearRenderTargetView(Context.BackbufferView, clearColor);
		}

		void D3D11RendererAPI::SetViewport(const Rectangle& viewport)
		{
			D3D11_VIEWPORT d3d_viewport{};
			d3d_viewport.TopLeftX = viewport.X;
			d3d_viewport.TopLeftY = viewport.Y;
			d3d_viewport.Width = viewport.Width;
			d3d_viewport.Height = viewport.Height;
			Context.DeviceContext->RSSetViewports(1, &d3d_viewport);
		}

		Rectangle D3D11RendererAPI::GetCurrentViewport()
		{
			D3D11_VIEWPORT d3dviewport{};
			uint32_t viewportCount = 1;
			Context.DeviceContext->RSGetViewports(&viewportCount, &d3dviewport);
			Rectangle viewport;

			viewport.X = d3dviewport.TopLeftX;
			viewport.Y = d3dviewport.TopLeftY;
			viewport.Width = d3dviewport.Width;
			viewport.Height = d3dviewport.Height;

			return viewport;
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
			switch (blendMode)
			{
			case Ainan::RenderingBlendMode::Additive:
				Context.DeviceContext->OMSetBlendState(AdditiveBlendMode, 0, 0xffffffff);
				break;

			case Ainan::RenderingBlendMode::Screen:
				Context.DeviceContext->OMSetBlendState(ScreenBlendMode, 0, 0xffffffff);
				break;

			case Ainan::RenderingBlendMode::NotSpecified:
			default:
				assert(false);
				break;
			}
		}

		void D3D11RendererAPI::SetRenderTargetApplicationWindow()
		{
			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, nullptr);
		}

		void D3D11RendererAPI::ImGuiNewFrame()
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

		void D3D11RendererAPI::ImGuiEndFrame()
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}

		void D3D11RendererAPI::InitImGui()
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable viewports

			//TEMPORARY
			ImGui_ImplDX11_Init(Context.Device, Context.DeviceContext);
			ImGui_ImplGlfw_Init(Window::Ptr, true, GlfwClientApi_Unknown);
		}

		//TODO
		void D3D11RendererAPI::DrawImGui(ImDrawData* drawData)
		{
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
			Context.Backbuffer->Release();
			Context.DeviceContext->Flush();

			Context.Swapchain->ResizeBuffers(2, newSwapchainSize.x, newSwapchainSize.y, DXGI_FORMAT_UNKNOWN, 0);

			ASSERT_D3D_CALL(Context.Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Context.Backbuffer));
			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView(Context.Backbuffer, 0, &Context.BackbufferView));

			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, 0);
		}
	}
}



#endif