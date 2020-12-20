#include "D3D11RendererAPI.h"

#include "renderer/IndexBuffer.h"

#ifdef PLATFORM_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3dcompiler.h>

#include "D3D11ShaderProgram.h"

#include "renderer/d3d11/D3D11RendererAPI.h"
#include "renderer/Renderer.h"

// DirectX data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGIFactory* g_pFactory = NULL;
static ID3D11Buffer* g_pVB = NULL;
static ID3D11Buffer* g_pIB = NULL;
static ID3D10Blob* g_pVertexShaderBlob = NULL;
static ID3D11VertexShader* g_pVertexShader = NULL;
static ID3D11InputLayout* g_pInputLayout = NULL;
static ID3D11Buffer* g_pVertexConstantBuffer = NULL;
static ID3D10Blob* g_pPixelShaderBlob = NULL;
static ID3D11PixelShader* g_pPixelShader = NULL;
static ID3D11SamplerState* g_pFontSampler = NULL;
static ID3D11ShaderResourceView* g_pFontTextureView = NULL;
static ID3D11RasterizerState* g_pRasterizerState = NULL;
static ID3D11BlendState* g_pBlendState = NULL;
static ID3D11DepthStencilState* g_pDepthStencilState = NULL;
static int g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct VERTEX_CONSTANT_BUFFER
{
	float   mvp[4][4];
};

struct ImGuiViewportDataDx11
{
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* RTView;

	ImGuiViewportDataDx11() { SwapChain = NULL; RTView = NULL; }
	~ImGuiViewportDataDx11() { IM_ASSERT(SwapChain == NULL && RTView == NULL); }
};

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

			D3D11_RENDER_TARGET_BLEND_DESC overlayBlendDesc{};
			overlayBlendDesc.BlendEnable = true;
			overlayBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
			overlayBlendDesc.SrcBlend = D3D11_BLEND_ONE;
			overlayBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
			overlayBlendDesc.DestBlend = D3D11_BLEND_ZERO;
			overlayBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
			overlayBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			overlayBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			D3D11_BLEND_DESC blendDesc{};
			blendDesc.RenderTarget[0] = additiveBlendDesc; 
			blendDesc.AlphaToCoverageEnable = 0;
			blendDesc.IndependentBlendEnable = 0;

			ASSERT_D3D_CALL(Context.Device->CreateBlendState(&blendDesc, &AdditiveBlendMode));

			blendDesc.RenderTarget[0] = screenBlendDesc;
			ASSERT_D3D_CALL(Context.Device->CreateBlendState(&blendDesc, &ScreenBlendMode));

			blendDesc.RenderTarget[0] = overlayBlendDesc;
			ASSERT_D3D_CALL(Context.Device->CreateBlendState(&blendDesc, &OverlayBlendMode));
		}

		static void SetupRenderState(ImDrawData* draw_data, ID3D11DeviceContext* ctx)
		{
			// Setup viewport
			D3D11_VIEWPORT vp;
			memset(&vp, 0, sizeof(D3D11_VIEWPORT));
			vp.Width = draw_data->DisplaySize.x;
			vp.Height = draw_data->DisplaySize.y;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = vp.TopLeftY = 0;
			ctx->RSSetViewports(1, &vp);

			// Setup shader and vertex buffers
			unsigned int stride = sizeof(ImDrawVert);
			unsigned int offset = 0;
			ctx->IASetInputLayout(g_pInputLayout);
			ctx->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);
			ctx->IASetIndexBuffer(g_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ctx->VSSetShader(g_pVertexShader, NULL, 0);
			ctx->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
			ctx->PSSetShader(g_pPixelShader, NULL, 0);
			ctx->PSSetSamplers(0, 1, &g_pFontSampler);
			ctx->GSSetShader(NULL, NULL, 0);
			ctx->HSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
			ctx->DSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..
			ctx->CSSetShader(NULL, NULL, 0); // In theory we should backup and restore this as well.. very infrequently used..

			// Setup blend state
			const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
			ctx->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
			ctx->OMSetDepthStencilState(g_pDepthStencilState, 0);
			ctx->RSSetState(g_pRasterizerState);
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

		void InvalidateDeviceObjects()
		{
			if (!g_pd3dDevice)
				return;

			if (g_pFontSampler) { g_pFontSampler->Release(); g_pFontSampler = NULL; }
			if (g_pFontTextureView) { g_pFontTextureView->Release(); g_pFontTextureView = NULL; ImGui::GetIO().Fonts->TexID = NULL; } // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
			if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
			if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }

			if (g_pBlendState) { g_pBlendState->Release(); g_pBlendState = NULL; }
			if (g_pDepthStencilState) { g_pDepthStencilState->Release(); g_pDepthStencilState = NULL; }
			if (g_pRasterizerState) { g_pRasterizerState->Release(); g_pRasterizerState = NULL; }
			if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = NULL; }
			if (g_pPixelShaderBlob) { g_pPixelShaderBlob->Release(); g_pPixelShaderBlob = NULL; }
			if (g_pVertexConstantBuffer) { g_pVertexConstantBuffer->Release(); g_pVertexConstantBuffer = NULL; }
			if (g_pInputLayout) { g_pInputLayout->Release(); g_pInputLayout = NULL; }
			if (g_pVertexShader) { g_pVertexShader->Release(); g_pVertexShader = NULL; }
			if (g_pVertexShaderBlob) { g_pVertexShaderBlob->Release(); g_pVertexShaderBlob = NULL; }
		}

		D3D11RendererAPI::~D3D11RendererAPI()
		{
			ImGui::DestroyPlatformWindows();
			InvalidateDeviceObjects();
			if (g_pFactory) { g_pFactory->Release(); g_pFactory = NULL; }
			if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
			if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
			AdditiveBlendMode->Release();
			ScreenBlendMode->Release();
			OverlayBlendMode->Release();
			Context.BackbufferView->Release();
			Context.Backbuffer->Release();
			Context.Swapchain->Release();
			Context.DeviceContext->Release();
			Context.Device->Release();
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, Primitive mode, uint32_t vertexCount)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(mode));

			D3D11ShaderProgram* d3dShader = (D3D11ShaderProgram*)&shader;;

			Context.DeviceContext->VSSetShader(d3dShader->VertexShader, 0, 0);
			Context.DeviceContext->PSSetShader(d3dShader->FragmentShader, 0, 0);

			Context.DeviceContext->Draw(vertexCount, 0);
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(mode));

			D3D11ShaderProgram* d3dShader = (D3D11ShaderProgram*)&shader;;

			Context.DeviceContext->VSSetShader(d3dShader->VertexShader, 0, 0);
			Context.DeviceContext->PSSetShader(d3dShader->FragmentShader, 0, 0);

			Context.DeviceContext->DrawIndexed(indexBuffer.GetCount(), 0, 0);
		}

		void D3D11RendererAPI::Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer, uint32_t vertexCount)
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
			ID3D11RenderTargetView* view;
			Context.DeviceContext->OMGetRenderTargets(1, &view, nullptr);
			Context.DeviceContext->ClearRenderTargetView(view, clearColor);
			view->Release();
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

			case Ainan::RenderingBlendMode::Overlay:
				Context.DeviceContext->OMSetBlendState(OverlayBlendMode, 0, 0xffffffff);
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
			auto func = []()
			{
				if (!g_pFontSampler)
				{
					if (!g_pd3dDevice)
						return false;
					if (g_pFontSampler)
						InvalidateDeviceObjects();

					// By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
					// If you would like to use this DX11 sample code but remove this dependency you can:
					//  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
					//  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL.
					// See https://github.com/ocornut/imgui/pull/638 for sources and details.

					// Create the vertex shader
					{
						static const char* vertexShader =
							"cbuffer vertexBuffer : register(b0) \
            {\
            float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
            float2 pos : POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = input.col;\
            output.uv  = input.uv;\
            return output;\
            }";

						D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pVertexShaderBlob, NULL);
						if (g_pVertexShaderBlob == NULL) // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
							return false;
						if (g_pd3dDevice->CreateVertexShader((DWORD*)g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), NULL, &g_pVertexShader) != S_OK)
							return false;

						// Create the input layout
						D3D11_INPUT_ELEMENT_DESC local_layout[] =
						{
							{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
							{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
						};
						if (g_pd3dDevice->CreateInputLayout(local_layout, 3, g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), &g_pInputLayout) != S_OK)
							return false;

						// Create the constant buffer
						{
							D3D11_BUFFER_DESC desc;
							desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
							desc.Usage = D3D11_USAGE_DYNAMIC;
							desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
							desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
							desc.MiscFlags = 0;
							g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVertexConstantBuffer);
						}
					}

					// Create the pixel shader
					{
						static const char* pixelShader =
							"struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            sampler sampler0;\
            Texture2D texture0;\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
            return out_col; \
            }";

						D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pPixelShaderBlob, NULL);
						if (g_pPixelShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
							return false;
						if (g_pd3dDevice->CreatePixelShader((DWORD*)g_pPixelShaderBlob->GetBufferPointer(), g_pPixelShaderBlob->GetBufferSize(), NULL, &g_pPixelShader) != S_OK)
							return false;
					}

					// Create the blending setup
					{
						D3D11_BLEND_DESC desc;
						ZeroMemory(&desc, sizeof(desc));
						desc.AlphaToCoverageEnable = false;
						desc.RenderTarget[0].BlendEnable = true;
						desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
						desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
						desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
						desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
						desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
						desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
						desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
						g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState);
					}

					// Create the rasterizer state
					{
						D3D11_RASTERIZER_DESC desc;
						ZeroMemory(&desc, sizeof(desc));
						desc.FillMode = D3D11_FILL_SOLID;
						desc.CullMode = D3D11_CULL_NONE;
						desc.ScissorEnable = true;
						desc.DepthClipEnable = true;
						g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
					}

					// Create depth-stencil State
					{
						D3D11_DEPTH_STENCIL_DESC desc;
						ZeroMemory(&desc, sizeof(desc));
						desc.DepthEnable = false;
						desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
						desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
						desc.StencilEnable = false;
						desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
						desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
						desc.BackFace = desc.FrontFace;
						g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState);
					}

					// Build texture atlas
					ImGuiIO& io = ImGui::GetIO();
					unsigned char* pixels;
					int width, height;
					io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

					// Upload texture to graphics system
					{
						D3D11_TEXTURE2D_DESC desc;
						ZeroMemory(&desc, sizeof(desc));
						desc.Width = width;
						desc.Height = height;
						desc.MipLevels = 1;
						desc.ArraySize = 1;
						desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						desc.SampleDesc.Count = 1;
						desc.Usage = D3D11_USAGE_DEFAULT;
						desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
						desc.CPUAccessFlags = 0;

						ID3D11Texture2D* pTexture = NULL;
						D3D11_SUBRESOURCE_DATA subResource;
						subResource.pSysMem = pixels;
						subResource.SysMemPitch = desc.Width * 4;
						subResource.SysMemSlicePitch = 0;
						g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

						// Create texture view
						D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
						ZeroMemory(&srvDesc, sizeof(srvDesc));
						srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MipLevels = desc.MipLevels;
						srvDesc.Texture2D.MostDetailedMip = 0;
						g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &g_pFontTextureView);
						pTexture->Release();
					}

					// Store our identifier
					io.Fonts->TexID = (ImTextureID)g_pFontTextureView;

					// Create texture sampler
					{
						D3D11_SAMPLER_DESC desc;
						ZeroMemory(&desc, sizeof(desc));
						desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
						desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
						desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
						desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
						desc.MipLODBias = 0.f;
						desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
						desc.MinLOD = 0.f;
						desc.MaxLOD = 0.f;
						g_pd3dDevice->CreateSamplerState(&desc, &g_pFontSampler);
					}
				}
			};
			Renderer::PushCommand(func);
			Renderer::WaitUntilRendererIdle();
		}

		void D3D11RendererAPI::ImGuiEndFrame()
		{
			ImGui::Render();
			ImGuiIO& io = ImGui::GetIO(); (void)io;

			auto func2 = [this]()
			{
				DrawImGui(ImGui::GetDrawData());
			};

			Renderer::PushCommand(func2);
			Renderer::WaitUntilRendererIdle();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
			
		}

		void D3D11RendererAPI::ExecuteCommand(RenderCommand cmd)
		{
			switch (cmd.Type)
			{
			case RenderCommandType::ClearScreen:
				ClearScreen();
				break;

			case RenderCommandType::Present:
				Present();
				break;

			default:
				break;
			}
		}

		void D3D11RendererAPI::InitImGui()
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable viewports

			// Setup back-end capabilities flags
			io.BackendRendererName = "imgui_impl_dx11";
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

			// Get factory from device
			IDXGIDevice* pDXGIDevice = NULL;
			IDXGIAdapter* pDXGIAdapter = NULL;
			IDXGIFactory* pFactory = NULL;

			if (Context.Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
				if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
					if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
					{
						g_pd3dDevice = Context.Device;
						g_pd3dDeviceContext = Context.DeviceContext;
						g_pFactory = pFactory;
					}
			if (pDXGIDevice) pDXGIDevice->Release();
			if (pDXGIAdapter) pDXGIAdapter->Release();
			g_pd3dDevice->AddRef();
			g_pd3dDeviceContext->AddRef();

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
				auto createWindowFunc = [](ImGuiViewport* viewport)
				{
					ImGuiViewportDataDx11* data = IM_NEW(ImGuiViewportDataDx11)();
					viewport->RendererUserData = data;

					// PlatformHandleRaw should always be a HWND, whereas PlatformHandle might be a higher-level handle (e.g. GLFWWindow*, SDL_Window*).
					// Some back-end will leave PlatformHandleRaw NULL, in which case we assume PlatformHandle will contain the HWND.
					HWND hwnd = viewport->PlatformHandleRaw ? (HWND)viewport->PlatformHandleRaw : (HWND)viewport->PlatformHandle;
					IM_ASSERT(hwnd != 0);

					// Create swap chain
					DXGI_SWAP_CHAIN_DESC sd;
					ZeroMemory(&sd, sizeof(sd));
					sd.BufferDesc.Width = (UINT)viewport->Size.x;
					sd.BufferDesc.Height = (UINT)viewport->Size.y;
					sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					sd.SampleDesc.Count = 1;
					sd.SampleDesc.Quality = 0;
					sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
					sd.BufferCount = 1;
					sd.OutputWindow = hwnd;
					sd.Windowed = TRUE;
					sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
					sd.Flags = 0;

					IM_ASSERT(data->SwapChain == NULL && data->RTView == NULL);
					g_pFactory->CreateSwapChain(g_pd3dDevice, &sd, &data->SwapChain);

					// Create the render target
					if (data->SwapChain)
					{
						ID3D11Texture2D* pBackBuffer;
						data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
						g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &data->RTView);
						pBackBuffer->Release();
					}
				};
				platform_io.Renderer_CreateWindow = createWindowFunc;

				auto destroyWindowFunc = [](ImGuiViewport* viewport)
				{
					// The main viewport (owned by the application) will always have RendererUserData == NULL since we didn't create the data for it.
					if (ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData)
					{
						if (data->SwapChain)
							data->SwapChain->Release();
						data->SwapChain = NULL;
						if (data->RTView)
							data->RTView->Release();
						data->RTView = NULL;
						IM_DELETE(data);
					}
					viewport->RendererUserData = NULL;
				};
				platform_io.Renderer_DestroyWindow = destroyWindowFunc;

				auto setWindowSizeFunc = [](ImGuiViewport* viewport, ImVec2 size)
				{
					ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData;
					if (data->RTView)
					{
						data->RTView->Release();
						data->RTView = NULL;
					}
					if (data->SwapChain)
					{
						ID3D11Texture2D* pBackBuffer = NULL;
						data->SwapChain->ResizeBuffers(0, (UINT)size.x, (UINT)size.y, DXGI_FORMAT_UNKNOWN, 0);
						data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
						if (pBackBuffer == NULL) { fprintf(stderr, "ImGui_ImplDX11_SetWindowSize() failed creating buffers.\n"); return; }
						g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &data->RTView);
						pBackBuffer->Release();
					}
				};
				platform_io.Renderer_SetWindowSize = setWindowSizeFunc;

				auto renderWindowFunc = [](ImGuiViewport* viewport, void* render_arg)
				{

					ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData;
					ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
					g_pd3dDeviceContext->OMSetRenderTargets(1, &data->RTView, NULL);
					if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
						g_pd3dDeviceContext->ClearRenderTargetView(data->RTView, (float*)&clear_color);
					auto& drawData = viewport->DrawData;

					// Avoid rendering when minimized
					if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
						return;

					ID3D11DeviceContext* ctx = g_pd3dDeviceContext;

					// Create and grow vertex/index buffers if needed
					if (!g_pVB || g_VertexBufferSize < drawData->TotalVtxCount)
					{
						if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
						g_VertexBufferSize = drawData->TotalVtxCount + 5000;
						D3D11_BUFFER_DESC desc;
						memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
						desc.Usage = D3D11_USAGE_DYNAMIC;
						desc.ByteWidth = g_VertexBufferSize * sizeof(ImDrawVert);
						desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
						desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
						desc.MiscFlags = 0;
						if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVB) < 0)
							return;
					}
					if (!g_pIB || g_IndexBufferSize < drawData->TotalIdxCount)
					{
						if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
						g_IndexBufferSize = drawData->TotalIdxCount + 10000;
						D3D11_BUFFER_DESC desc;
						memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
						desc.Usage = D3D11_USAGE_DYNAMIC;
						desc.ByteWidth = g_IndexBufferSize * sizeof(ImDrawIdx);
						desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
						desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
						if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pIB) < 0)
							return;
					}

					// Upload vertex/index data into a single contiguous GPU buffer
					D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
					if (ctx->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
						return;
					if (ctx->Map(g_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
						return;
					ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
					ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
					for (int n = 0; n < drawData->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = drawData->CmdLists[n];
						memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
						memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
						vtx_dst += cmd_list->VtxBuffer.Size;
						idx_dst += cmd_list->IdxBuffer.Size;
					}
					ctx->Unmap(g_pVB, 0);
					ctx->Unmap(g_pIB, 0);

					// Setup orthographic projection matrix into our constant buffer
					// Our visible imgui space lies from drawData->DisplayPos (top left) to drawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
					{
						D3D11_MAPPED_SUBRESOURCE mapped_resource;
						if (ctx->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
							return;
						VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
						float L = drawData->DisplayPos.x;
						float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
						float T = drawData->DisplayPos.y;
						float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
						float mvp[4][4] =
						{
							{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
							{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
							{ 0.0f,         0.0f,           0.5f,       0.0f },
							{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
						};
						memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
						ctx->Unmap(g_pVertexConstantBuffer, 0);
					}

					// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
					struct BACKUP_DX11_STATE
					{
						UINT                        ScissorRectsCount, ViewportsCount;
						D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
						D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
						ID3D11RasterizerState* RS;
						ID3D11BlendState* BlendState;
						FLOAT                       BlendFactor[4];
						UINT                        SampleMask;
						UINT                        StencilRef;
						ID3D11DepthStencilState* DepthStencilState;
						ID3D11ShaderResourceView* PSShaderResource;
						ID3D11SamplerState* PSSampler;
						ID3D11PixelShader* PS;
						ID3D11VertexShader* VS;
						ID3D11GeometryShader* GS;
						UINT                        PSInstancesCount, VSInstancesCount, GSInstancesCount;
						ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];   // 256 is max according to PSSetShader documentation
						D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
						ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
						UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
						DXGI_FORMAT                 IndexBufferFormat;
						ID3D11InputLayout* InputLayout;
					};
					BACKUP_DX11_STATE old;
					old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
					ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
					ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
					ctx->RSGetState(&old.RS);
					ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
					ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
					ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
					ctx->PSGetSamplers(0, 1, &old.PSSampler);
					old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;
					ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
					ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
					ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
					ctx->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);

					ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
					ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
					ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
					ctx->IAGetInputLayout(&old.InputLayout);

					// Setup desired DX state
					SetupRenderState(drawData, ctx);

					// Render command lists
					// (Because we merged all buffers into a single one, we maintain our own offset into them)
					int global_idx_offset = 0;
					int global_vtx_offset = 0;
					ImVec2 clip_off = drawData->DisplayPos;
					for (int n = 0; n < drawData->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = drawData->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
							if (pcmd->UserCallback != NULL)
							{
								// User callback, registered via ImDrawList::AddCallback()
								// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
								if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
									SetupRenderState(drawData, ctx);
								else
									pcmd->UserCallback(cmd_list, pcmd);
							}
							else
							{
								// Apply scissor/clipping rectangle
								const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
								ctx->RSSetScissorRects(1, &r);

								// Bind texture, Draw
								ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)pcmd->TextureId;
								ctx->PSSetShaderResources(0, 1, &texture_srv);
								ctx->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
							}
						}
						global_idx_offset += cmd_list->IdxBuffer.Size;
						global_vtx_offset += cmd_list->VtxBuffer.Size;
					}

					// Restore modified DX state
					ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
					ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
					ctx->RSSetState(old.RS); if (old.RS) old.RS->Release();
					ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
					ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
					ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
					ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
					ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
					for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
					ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
					ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
					ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount); if (old.GS) old.GS->Release();
					for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
					ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
					ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
					ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
					ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
				};
				platform_io.Renderer_RenderWindow = renderWindowFunc;

				auto swapBuffersFunc = [](ImGuiViewport* viewport, void* render_arg)
				{
					ImGuiViewportDataDx11* data = (ImGuiViewportDataDx11*)viewport->RendererUserData;
					data->SwapChain->Present(0, 0); // Present without vsync
				};
				platform_io.Renderer_SwapBuffers = swapBuffersFunc;
			}

			ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			main_viewport->PlatformHandle = (void*)Window::Ptr;
#ifdef _WIN32
			main_viewport->PlatformHandleRaw = glfwGetWin32Window(Window::Ptr);
#endif
		}

		void D3D11RendererAPI::DrawImGui(ImDrawData* drawData)
		{
			// Avoid rendering when minimized
			if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
				return;

			ID3D11DeviceContext* ctx = g_pd3dDeviceContext;

			// Create and grow vertex/index buffers if needed
			if (!g_pVB || g_VertexBufferSize < drawData->TotalVtxCount)
			{
				if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
				g_VertexBufferSize = drawData->TotalVtxCount + 5000;
				D3D11_BUFFER_DESC desc;
				memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.ByteWidth = g_VertexBufferSize * sizeof(ImDrawVert);
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				desc.MiscFlags = 0;
				if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pVB) < 0)
					return;
			}
			if (!g_pIB || g_IndexBufferSize < drawData->TotalIdxCount)
			{
				if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
				g_IndexBufferSize = drawData->TotalIdxCount + 10000;
				D3D11_BUFFER_DESC desc;
				memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.ByteWidth = g_IndexBufferSize * sizeof(ImDrawIdx);
				desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				if (g_pd3dDevice->CreateBuffer(&desc, NULL, &g_pIB) < 0)
					return;
			}

			// Upload vertex/index data into a single contiguous GPU buffer
			D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
			if (ctx->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
				return;
			if (ctx->Map(g_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
				return;
			ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource.pData;
			ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource.pData;
			for (int n = 0; n < drawData->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = drawData->CmdLists[n];
				memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				vtx_dst += cmd_list->VtxBuffer.Size;
				idx_dst += cmd_list->IdxBuffer.Size;
			}
			ctx->Unmap(g_pVB, 0);
			ctx->Unmap(g_pIB, 0);

			// Setup orthographic projection matrix into our constant buffer
			// Our visible imgui space lies from drawData->DisplayPos (top left) to drawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
			{
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				if (ctx->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
					return;
				VERTEX_CONSTANT_BUFFER* constant_buffer = (VERTEX_CONSTANT_BUFFER*)mapped_resource.pData;
				float L = drawData->DisplayPos.x;
				float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
				float T = drawData->DisplayPos.y;
				float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
				float mvp[4][4] =
				{
					{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
					{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
					{ 0.0f,         0.0f,           0.5f,       0.0f },
					{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
				};
				memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
				ctx->Unmap(g_pVertexConstantBuffer, 0);
			}

			// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
			struct BACKUP_DX11_STATE
			{
				UINT                        ScissorRectsCount, ViewportsCount;
				D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				ID3D11RasterizerState* RS;
				ID3D11BlendState* BlendState;
				FLOAT                       BlendFactor[4];
				UINT                        SampleMask;
				UINT                        StencilRef;
				ID3D11DepthStencilState* DepthStencilState;
				ID3D11ShaderResourceView* PSShaderResource;
				ID3D11SamplerState* PSSampler;
				ID3D11PixelShader* PS;
				ID3D11VertexShader* VS;
				ID3D11GeometryShader* GS;
				UINT                        PSInstancesCount, VSInstancesCount, GSInstancesCount;
				ID3D11ClassInstance* PSInstances[256], * VSInstances[256], * GSInstances[256];   // 256 is max according to PSSetShader documentation
				D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
				ID3D11Buffer* IndexBuffer, * VertexBuffer, * VSConstantBuffer;
				UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
				DXGI_FORMAT                 IndexBufferFormat;
				ID3D11InputLayout* InputLayout;
			};
			BACKUP_DX11_STATE old;
			old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
			ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
			ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
			ctx->RSGetState(&old.RS);
			ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
			ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
			ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
			ctx->PSGetSamplers(0, 1, &old.PSSampler);
			old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;
			ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
			ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
			ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
			ctx->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);

			ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
			ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
			ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
			ctx->IAGetInputLayout(&old.InputLayout);

			// Setup desired DX state
			SetupRenderState(drawData, ctx);

			// Render command lists
			// (Because we merged all buffers into a single one, we maintain our own offset into them)
			int global_idx_offset = 0;
			int global_vtx_offset = 0;
			ImVec2 clip_off = drawData->DisplayPos;
			for (int n = 0; n < drawData->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = drawData->CmdLists[n];
				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
					if (pcmd->UserCallback != NULL)
					{
						// User callback, registered via ImDrawList::AddCallback()
						// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
						if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
							SetupRenderState(drawData, ctx);
						else
							pcmd->UserCallback(cmd_list, pcmd);
					}
					else
					{
						// Apply scissor/clipping rectangle
						const D3D11_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
						ctx->RSSetScissorRects(1, &r);

						// Bind texture, Draw
						ID3D11ShaderResourceView* texture_srv = (ID3D11ShaderResourceView*)pcmd->TextureId;
						ctx->PSSetShaderResources(0, 1, &texture_srv);
						ctx->DrawIndexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);
					}
				}
				global_idx_offset += cmd_list->IdxBuffer.Size;
				global_vtx_offset += cmd_list->VtxBuffer.Size;
			}

			// Restore modified DX state
			ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
			ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
			ctx->RSSetState(old.RS); if (old.RS) old.RS->Release();
			ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
			ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
			ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
			ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
			ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
			for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
			ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
			ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
			ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount); if (old.GS) old.GS->Release();
			for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
			ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
			ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
			ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
			ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
		}

		void D3D11RendererAPI::Present()
		{
			Context.Swapchain->Present(1, 0);
			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, nullptr);
		}

		void D3D11RendererAPI::TerminateImGui()
		{
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