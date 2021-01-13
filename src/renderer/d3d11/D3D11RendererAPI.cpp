#include "D3D11RendererAPI.h"

#include "renderer/IndexBuffer.h"

#ifdef PLATFORM_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3dcompiler.h>

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

		constexpr DXGI_FORMAT GetD3D11FormatFromShaderType(const ShaderVariableType& type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return DXGI_FORMAT_R32_SINT;

			case ShaderVariableType::UnsignedInt:
				return DXGI_FORMAT_R32_UINT;

			case ShaderVariableType::Float:
				return DXGI_FORMAT_R32_FLOAT;

			case ShaderVariableType::Vec2:
				return DXGI_FORMAT_R32G32_FLOAT;

			case ShaderVariableType::Vec3:
			case ShaderVariableType::Mat3:
				return DXGI_FORMAT_R32G32B32_FLOAT;

			case ShaderVariableType::Vec4:
			case ShaderVariableType::Mat4:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;

			default:
				assert(false);
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		static DXGI_FORMAT D3DFormat(TextureFormat format)
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

		void D3D11RendererAPI::ClearScreen()
		{
			float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			ID3D11RenderTargetView* view;
			Context.DeviceContext->OMGetRenderTargets(1, &view, nullptr);
			Context.DeviceContext->ClearRenderTargetView(view, clearColor);
			view->Release();
		}

		void D3D11RendererAPI::RecreateSwapchain(const RenderCommand& cmd)
		{
			Context.DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
			Context.BackbufferView->Release();
			Context.Backbuffer->Release();
			Context.DeviceContext->Flush();

			Context.Swapchain->ResizeBuffers(2, cmd.RecreateSweapchainCmdDesc.Width, cmd.RecreateSweapchainCmdDesc.Height, DXGI_FORMAT_UNKNOWN, 0);

			ASSERT_D3D_CALL(Context.Swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&Context.Backbuffer));
			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView(Context.Backbuffer, 0, &Context.BackbufferView));

			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, 0);
		}

		void D3D11RendererAPI::SetViewport(const Rectangle& viewport)
		{
			D3D11_VIEWPORT d3d_viewport{};
			d3d_viewport.TopLeftX = viewport.X;
			d3d_viewport.TopLeftY = viewport.Y;
			d3d_viewport.Width = viewport.Width;
			d3d_viewport.Height = viewport.Height;
			d3d_viewport.MaxDepth = 1;
			Context.DeviceContext->RSSetViewports(1, &d3d_viewport);
		}

		void D3D11RendererAPI::SetViewport(const RenderCommand& cmd)
		{
			D3D11_VIEWPORT d3d_viewport{};
			d3d_viewport.TopLeftX = cmd.SetViewportCmdDesc.X;
			d3d_viewport.TopLeftY = cmd.SetViewportCmdDesc.Y;
			d3d_viewport.Width = cmd.SetViewportCmdDesc.Width;
			d3d_viewport.Height = cmd.SetViewportCmdDesc.Height;
			d3d_viewport.MinDepth = cmd.SetViewportCmdDesc.MinDepth;
			d3d_viewport.MaxDepth = cmd.SetViewportCmdDesc.MaxDepth;
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

			ImGui::UpdatePlatformWindows();

			auto func2 = [this]()
			{
				DrawImGui(ImGui::GetDrawData());
				ImGui::RenderPlatformWindowsDefault();
			};

			Renderer::PushCommand(func2);
			Renderer::WaitUntilRendererIdle();
		}

		void D3D11RendererAPI::ExecuteCommand(RenderCommand cmd)
		{
			switch (cmd.Type)
			{
			case RenderCommandType::Clear:
				ClearScreen();
				break;

			case RenderCommandType::Present:
				Present();
				break;

			case RenderCommandType::RecreateSweapchain:
				RecreateSwapchain(cmd);
				break;

			case RenderCommandType::SetViewport:
				SetViewport(cmd);
				break;

			case RenderCommandType::SetBlendMode:
				SetBlendMode(cmd.SetBlendModeCmdDesc.Mode);
				break;

			case RenderCommandType::CreateShaderProgram:
				CreateShaderProgramNew(cmd);
				break;

			case RenderCommandType::DestroyShaderProgram:
				DestroyShaderProgramNew(cmd);
				break;

			case RenderCommandType::CreateVertexBuffer:
				CreateVertexBufferNew(cmd);
				break;

			case RenderCommandType::UpdateVertexBuffer:
				UpdateVertexBufferNew(cmd);
				break;

			case RenderCommandType::DestroyVertexBuffer:
				DestroyVertexBufferNew(cmd);
				break;

			case RenderCommandType::CreateIndexBuffer:
				CreateIndexBufferNew(cmd);
				break;

			case RenderCommandType::DestroyIndexBuffer:
				DestroyIndexBufferNew(cmd);
				break;

			case RenderCommandType::CreateUniformBuffer:
				CreateUniformBufferNew(cmd);
				break;

			case RenderCommandType::BindUniformBuffer:
				BindUniformBufferNew(cmd);
				break;

			case RenderCommandType::UpdateUniformBuffer:
				UpdateUniformBufferNew(cmd);
				break;

			case RenderCommandType::DestroyUniformBuffer:
				DestroyUniformBufferNew(cmd);
				break;

			case RenderCommandType::CreateFrameBuffer:
				CreateFrameBufferNew(cmd);
				break;

			case RenderCommandType::BindFrameBufferAsTexture:
				BindFrameBufferAsTextureNew(cmd);
				break;

			case RenderCommandType::BindFrameBufferAsRenderTarget:
				BindFrameBufferAsRenderTargetNew(cmd);
				break;

			case RenderCommandType::BindBackBufferAsRenderTarget:
				BindWindowFrameBufferAsRenderTargetNew(cmd);
				break;

			case RenderCommandType::ResizeFrameBuffer:
				ResizeFrameBufferNew(cmd);
				break;

			case RenderCommandType::ReadFrameBuffer:
				ReadFrameBufferNew(cmd);
				break;

			case RenderCommandType::DestroyFrameBuffer:
				DestroyFrameBufferNew(cmd);
				break;

			case RenderCommandType::CreateTexture:
				CreateTextureNew(cmd);
				break;	
			
			case RenderCommandType::BindTexture:
				BindTextureNew(cmd);
				break;

			case RenderCommandType::UpdateTexture:
				UpdateTextureNew(cmd);
				break;

			case RenderCommandType::DestroyTexture:
				DestroyTextureNew(cmd);
				break;

			case RenderCommandType::DrawNew:
				DrawNew(cmd);
				break;

			case RenderCommandType::DrawIndexedNew:
				DrawIndexedNew(cmd);
				break;

			case RenderCommandType::DrawIndexedNewWithCustomNumberOfVertices:
				DrawIndexedNewWithCustomNumberOfVertices(cmd);
				break;

			default:
				break;
			}
		}

		void D3D11RendererAPI::CreateShaderProgramNew(const RenderCommand& cmd)
		{
			ShaderProgramCreationInfo* info = cmd.CreateShaderProgramCmdDesc.Info;
			ShaderProgramDataView* output = cmd.CreateShaderProgramCmdDesc.Output;

			//load batch renderer shader
			uint8_t* fragmentByteCode = nullptr;
			uint32_t fragmentByteCodeSize = 0;
			{
				FILE* file = nullptr;
				auto err = fopen_s(&file, (info->vertPath + "_vs.cso").c_str(), "rb");
				if (err != 0 || !file)
					assert(false);

				fseek(file, 0, SEEK_END);
				output->VertexByteCodeSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				output->VertexByteCode = new uint8_t[output->VertexByteCodeSize];
				fread(output->VertexByteCode, output->VertexByteCodeSize, 1, file);
				fclose(file);
			}
			{
				FILE* file = nullptr;
				auto err = fopen_s(&file, (info->fragPath + "_fs.cso").c_str(), "rb");
				if (err != 0)
					assert(false);

				fseek(file, 0, SEEK_END);
				fragmentByteCodeSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				fragmentByteCode = new uint8_t[fragmentByteCodeSize];
				fread(fragmentByteCode, fragmentByteCodeSize, 1, file);
				fclose(file);
			}

			ASSERT_D3D_CALL(Context.Device->CreateVertexShader(output->VertexByteCode, output->VertexByteCodeSize, 0, (ID3D11VertexShader**)&output->Identifier));
			ASSERT_D3D_CALL(Context.Device->CreatePixelShader(fragmentByteCode, fragmentByteCodeSize, 0, (ID3D11PixelShader**)&output->Identifier_1));

			delete[] fragmentByteCode;
			delete info;
		}

		void D3D11RendererAPI::DestroyShaderProgramNew(const RenderCommand& cmd)
		{
			auto vShader = (ID3D11VertexShader*)cmd.DestroyShaderProgramCmdDesc.Program->Identifier;
			vShader->Release();

			auto fShader = (ID3D11PixelShader*)cmd.DestroyShaderProgramCmdDesc.Program->Identifier_1;
			fShader->Release();

			delete[] cmd.DestroyShaderProgramCmdDesc.Program->VertexByteCode;
			cmd.DestroyShaderProgramCmdDesc.Program->Deleted = true;
		}

		void D3D11RendererAPI::CreateVertexBufferNew(const RenderCommand& cmd)
		{
			VertexBufferCreationInfo* info = cmd.CreateVertexBufferCmdDesc.Info;
			VertexBufferDataView* output = cmd.CreateVertexBufferCmdDesc.Output;

			output->Stride = 0;
			uint32_t Memory = info->Size;

			//create buffer
			{
				D3D11_BUFFER_DESC desc{};
				desc.ByteWidth = info->Size;
				desc.Usage = info->Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = info->Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
				if (info->InitialData)
				{
					D3D11_SUBRESOURCE_DATA initialData{};
					initialData.pSysMem = info->InitialData;

					ASSERT_D3D_CALL(Context.Device->CreateBuffer(&desc, &initialData, (ID3D11Buffer**)&output->Identifier));
				}
				else
				{
					ASSERT_D3D_CALL(Context.Device->CreateBuffer(&desc, 0, (ID3D11Buffer**)&output->Identifier));
				}
			}

			//set layout
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> desc(info->Layout.size());

				for (size_t i = 0; i < info->Layout.size(); i++)
				{
					desc[i].SemanticName = info->Layout[i].SemanticName.c_str();
					desc[i].SemanticIndex = info->Layout[i].SemanticIndex;
					desc[i].Format = GetD3D11FormatFromShaderType(info->Layout[i].Type);
					desc[i].InputSlot = 0;
					desc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					desc[i].InstanceDataStepRate = 0;
					desc[i].AlignedByteOffset = output->Stride;
					output->Stride += info->Layout[i].GetSize();
				}

				ASSERT_D3D_CALL(Context.Device->CreateInputLayout(desc.data(), desc.size(), info->Shader->VertexByteCode, info->Shader->VertexByteCodeSize, (ID3D11InputLayout**)&output->Layout));
			}

			delete[] info->InitialData;
			delete info;
		}

		void D3D11RendererAPI::UpdateVertexBufferNew(const RenderCommand& cmd)
		{
			D3D11_MAPPED_SUBRESOURCE resource{};

			ASSERT_D3D_CALL(Context.DeviceContext->Map((ID3D11Resource*)cmd.UpdateVertexBufferCmdDesc.VertexBuffer->Identifier, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));

			memcpy((uint8_t*)resource.pData + cmd.UpdateVertexBufferCmdDesc.Offset, cmd.UpdateVertexBufferCmdDesc.Data, cmd.UpdateVertexBufferCmdDesc.Size);

			Context.DeviceContext->Unmap((ID3D11Resource*)cmd.UpdateVertexBufferCmdDesc.VertexBuffer->Identifier, 0);

			delete[] cmd.UpdateVertexBufferCmdDesc.Data;
		}

		void D3D11RendererAPI::DestroyVertexBufferNew(const RenderCommand& cmd)
		{
			((ID3D11Buffer*)cmd.DestroyVertexBufferCmdDesc.Buffer->Identifier)->Release();
			((ID3D11InputLayout*)cmd.DestroyVertexBufferCmdDesc.Buffer->Layout)->Release();
			cmd.DestroyVertexBufferCmdDesc.Buffer->Deleted = true;
		}

		void D3D11RendererAPI::CreateIndexBufferNew(const RenderCommand& cmd)
		{
			IndexBufferCreationInfo* info = cmd.CreateIndexBufferCmdDesc.Info;
			IndexBufferDataView* output = cmd.CreateIndexBufferCmdDesc.Output;

			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = info->Count * sizeof(uint32_t);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			if (info->InitialData)
			{
				D3D11_SUBRESOURCE_DATA initialData{};
				initialData.pSysMem = info->InitialData;

				ASSERT_D3D_CALL(Context.Device->CreateBuffer(&desc, &initialData, (ID3D11Buffer**)&output->Identifier));
			}
			else
			{
				ASSERT_D3D_CALL(Context.Device->CreateBuffer(&desc, 0, (ID3D11Buffer**)&output->Identifier));
			}

			delete[] info->InitialData;
			delete info;
		}

		void D3D11RendererAPI::DestroyIndexBufferNew(const RenderCommand& cmd)
		{
			((ID3D11Buffer*)cmd.DestroyIndexBufferCmdDesc.Buffer->Identifier)->Release();
			cmd.DestroyIndexBufferCmdDesc.Buffer->Deleted = true;
		}

		void D3D11RendererAPI::CreateUniformBufferNew(const RenderCommand& cmd)
		{
			UniformBufferCreationInfo* info = cmd.CreateUniformBufferCmdDesc.Info;
			UniformBufferDataView* output = cmd.CreateUniformBufferCmdDesc.Output;

			uint32_t BufferSize = 0;

			//calculate buffer size with alignment
			for (auto& layoutPart : info->layout)
			{
				if (layoutPart.Count == 1)
				{
					if (BufferSize % sizeof(glm::vec4) != 0 && (sizeof(glm::vec4) - (BufferSize % sizeof(glm::vec4)) < layoutPart.GetSize()))
					{
						BufferSize += sizeof(glm::vec4) - BufferSize % sizeof(glm::vec4);
					}
					BufferSize += layoutPart.GetSize();
				}
				else
				{
					if (BufferSize % sizeof(glm::vec4) != 0)
					{
						BufferSize += sizeof(glm::vec4) - BufferSize % sizeof(glm::vec4);
					}
					BufferSize += layoutPart.Count * sizeof(glm::vec4);
				}
			}

			if (BufferSize % 16 != 0)
				BufferSize += 16 - (BufferSize % 16);

			output->AlignedSize = BufferSize;
			output->PackedSize = std::accumulate(info->layout.begin(), info->layout.end(), 0,
				[](uint32_t a, const VertexLayoutElement& b) -> uint32_t
				{
					return a + b.GetSize();
				});

			//create buffer
			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = output->AlignedSize;
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			ASSERT_D3D_CALL(Context.Device->CreateBuffer(&desc, 0, (ID3D11Buffer **)&output->Identifier));
			output->Name = info->Name;

			output->Layout = info->layout;
			delete info;
		}

		void D3D11RendererAPI::BindUniformBufferNew(const RenderCommand& cmd)
		{
			switch (cmd.BindUniformBufferCmdDesc.Stage)
			{
			case RenderingStage::VertexShader:
				Context.DeviceContext->VSSetConstantBuffers(cmd.BindUniformBufferCmdDesc.Slot, 1, (ID3D11Buffer**)&cmd.BindUniformBufferCmdDesc.Buffer->Identifier);
				break;

			case RenderingStage::FragmentShader:
				Context.DeviceContext->PSSetConstantBuffers(cmd.BindUniformBufferCmdDesc.Slot, 1, (ID3D11Buffer**)&cmd.BindUniformBufferCmdDesc.Buffer->Identifier);
				break;
			}
		}

		void D3D11RendererAPI::UpdateUniformBufferNew(const RenderCommand& cmd)
		{
			D3D11_MAPPED_SUBRESOURCE subresource{};
			ASSERT_D3D_CALL(Context.DeviceContext->Map((ID3D11Resource*)cmd.UpdateUniformBufferCmdDesc.Buffer->Identifier, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

			uint32_t alignedDataIndex = 0;
			uint32_t unalignedDataIndex = 0;
			uint8_t* unalignedData = (uint8_t*)cmd.UpdateUniformBufferCmdDesc.Data;
			uint8_t* alignedData = (uint8_t*)subresource.pData;
			for (auto& layoutPart : cmd.UpdateUniformBufferCmdDesc.Buffer->Layout)
			{
				if (layoutPart.Count == 1)
				{
					uint32_t size = layoutPart.GetSize();

					if (alignedDataIndex % sizeof(glm::vec4) != 0 && (alignedDataIndex % sizeof(glm::vec4)) < layoutPart.GetSize())
						alignedDataIndex += sizeof(glm::vec4) - alignedDataIndex % sizeof(glm::vec4);

					memcpy(&alignedData[alignedDataIndex], &unalignedData[unalignedDataIndex], size);
					alignedDataIndex += size;
					unalignedDataIndex += size;
				}
				else
				{
					size_t elementSize = layoutPart.GetSize() / layoutPart.Count;
					for (size_t i = 0; i < layoutPart.Count; i++)
					{
						if (alignedDataIndex % sizeof(glm::vec4) != 0)
						{
							alignedDataIndex += sizeof(glm::vec4) - alignedDataIndex % sizeof(glm::vec4);
						}
						memcpy(&alignedData[alignedDataIndex], &unalignedData[unalignedDataIndex], elementSize);

						alignedDataIndex += elementSize;
						unalignedDataIndex += elementSize;
					}
				}
			}

			Context.DeviceContext->Unmap((ID3D11Resource*)cmd.UpdateUniformBufferCmdDesc.Buffer->Identifier, 0);
			delete[] unalignedData;
		}

		void D3D11RendererAPI::DestroyUniformBufferNew(const RenderCommand& cmd)
		{
			((ID3D11Buffer*)cmd.DestroyUniformBufferCmdDesc.Buffer->Identifier)->Release();
			cmd.DestroyUniformBufferCmdDesc.Buffer->Deleted = true;
		}

		void D3D11RendererAPI::CreateFrameBufferNew(const RenderCommand& cmd)
		{
			FrameBufferCreationInfo* info = cmd.CreateFrameBufferCmdDesc.Info;
			FrameBufferDataView* output = cmd.CreateFrameBufferCmdDesc.Output;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = info->Size.x;
			desc.Height = info->Size.y;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 0;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ASSERT_D3D_CALL(Context.Device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&output->TextureIdentifier));

			D3D11_RENDER_TARGET_VIEW_DESC viewDesc{};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView((ID3D11Texture2D*)output->TextureIdentifier, &viewDesc, (ID3D11RenderTargetView**)&output->Identifier));

			D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc{};
			textureViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.Texture2D.MipLevels = 1;

			ASSERT_D3D_CALL(Context.Device->CreateShaderResourceView((ID3D11Texture2D*)output->TextureIdentifier, &textureViewDesc, (ID3D11ShaderResourceView**)&output->ResourceIdentifier));

			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

			ASSERT_D3D_CALL(Context.Device->CreateSamplerState(&samplerDesc, (ID3D11SamplerState**)&output->SamplerIdentifier));

			delete info;
		}

		void D3D11RendererAPI::BindFrameBufferAsTextureNew(const RenderCommand& cmd)
		{
			uint32_t slot = cmd.BindFrameBufferAsTextureCmdDesc.Slot;
			FrameBufferDataView* buffer = cmd.BindFrameBufferAsTextureCmdDesc.Buffer;
			switch (cmd.BindFrameBufferAsTextureCmdDesc.Stage)
			{
			case RenderingStage::VertexShader:
				Context.DeviceContext->VSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&buffer->ResourceIdentifier);
				Context.DeviceContext->VSSetSamplers(slot, 1, (ID3D11SamplerState**)&buffer->SamplerIdentifier);
				break;

			case RenderingStage::FragmentShader:
				Context.DeviceContext->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&buffer->ResourceIdentifier);
				Context.DeviceContext->PSSetSamplers(slot, 1, (ID3D11SamplerState**)&buffer->SamplerIdentifier);
				break;
			}
		}

		void D3D11RendererAPI::BindFrameBufferAsRenderTargetNew(const RenderCommand& cmd)
		{
			Context.DeviceContext->OMSetRenderTargets(1, (ID3D11RenderTargetView**)&cmd.BindFrameBufferAsRenderTargetCmdDesc.Buffer->Identifier, nullptr);
		}

		void D3D11RendererAPI::BindWindowFrameBufferAsRenderTargetNew(const RenderCommand& cmd)
		{
			Context.DeviceContext->OMSetRenderTargets(1, &Context.BackbufferView, nullptr);
		}

		void D3D11RendererAPI::ResizeFrameBufferNew(const RenderCommand& cmd)
		{
			((ID3D11RenderTargetView*)cmd.ResizeFrameBufferCmdDesc.Buffer->Identifier)->Release();
			((ID3D11Texture2D*)cmd.ResizeFrameBufferCmdDesc.Buffer->TextureIdentifier)->Release();
			((ID3D11ShaderResourceView*)cmd.ResizeFrameBufferCmdDesc.Buffer->ResourceIdentifier)->Release();

			cmd.ResizeFrameBufferCmdDesc.Buffer->Size = glm::vec2(cmd.ResizeFrameBufferCmdDesc.Width, cmd.ResizeFrameBufferCmdDesc.Height);

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = cmd.ResizeFrameBufferCmdDesc.Buffer->Size.x;
			desc.Height = cmd.ResizeFrameBufferCmdDesc.Buffer->Size.y;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ASSERT_D3D_CALL(Context.Device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&cmd.ResizeFrameBufferCmdDesc.Buffer->TextureIdentifier));

			D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc{};
			textureViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			textureViewDesc.Texture2D.MipLevels = 1;
			ASSERT_D3D_CALL(Context.Device->CreateShaderResourceView((ID3D11Texture2D*)cmd.ResizeFrameBufferCmdDesc.Buffer->TextureIdentifier, &textureViewDesc, (ID3D11ShaderResourceView**)&cmd.ResizeFrameBufferCmdDesc.Buffer->ResourceIdentifier));


			D3D11_RENDER_TARGET_VIEW_DESC viewDesc{};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

			ASSERT_D3D_CALL(Context.Device->CreateRenderTargetView((ID3D11Texture2D*)cmd.ResizeFrameBufferCmdDesc.Buffer->TextureIdentifier, &viewDesc, (ID3D11RenderTargetView**)&cmd.ResizeFrameBufferCmdDesc.Buffer->Identifier));
		}

		void D3D11RendererAPI::ReadFrameBufferNew(const RenderCommand& cmd)
		{
			Image* img = cmd.ReadFrameBufferCmdDesc.Output;
			glm::vec2 bottomLeftPixel = { cmd.ReadFrameBufferCmdDesc.BottomLeftX, cmd.ReadFrameBufferCmdDesc.BottomLeftY };
			glm::vec2 topRightPixel = { cmd.ReadFrameBufferCmdDesc.TopRightX, cmd.ReadFrameBufferCmdDesc.TopRightY };

			topRightPixel = cmd.ReadFrameBufferCmdDesc.Buffer->Size;

			//create a staging texture
			ID3D11Texture2D* stagingTexture = nullptr;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = topRightPixel.x;
			desc.Height = topRightPixel.y;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			ASSERT_D3D_CALL(Context.Device->CreateTexture2D(&desc, nullptr, &stagingTexture));

			//TODO support cpying parts of the framebuffer and not just all of it
			Context.DeviceContext->CopyResource(stagingTexture, (ID3D11Resource*)cmd.ReadFrameBufferCmdDesc.Buffer->TextureIdentifier);

			D3D11_MAPPED_SUBRESOURCE resource{};

			Context.DeviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &resource);

			img->Format = TextureFormat::RGBA;
			img->m_Width = topRightPixel.x;
			img->m_Height = topRightPixel.y;
			img->m_Data = new uint8_t[topRightPixel.x * topRightPixel.y * 4];

			uint8_t* srcPtr = (uint8_t*)resource.pData;
			uint8_t* destPtr = (uint8_t*)img->m_Data;
			const uint32_t unpaddedRowSize = topRightPixel.x * 4;
			for (size_t i = 0; i < topRightPixel.y - 1; i++)
			{
				memcpy(destPtr, srcPtr, unpaddedRowSize);
				if (i == topRightPixel.y - 2)
					break;
				srcPtr += resource.RowPitch;
				destPtr += unpaddedRowSize;
			}

			Context.DeviceContext->Unmap(stagingTexture, 0);
			stagingTexture->Release();
		}

		void D3D11RendererAPI::DestroyFrameBufferNew(const RenderCommand& cmd)
		{
			((ID3D11SamplerState*)cmd.DestroyFrameBufferCmdDesc.Buffer->SamplerIdentifier)->Release();
			((ID3D11ShaderResourceView*)cmd.DestroyFrameBufferCmdDesc.Buffer->ResourceIdentifier)->Release();
			((ID3D11RenderTargetView*)cmd.DestroyFrameBufferCmdDesc.Buffer->Identifier)->Release();
			((ID3D11Texture2D*)cmd.DestroyFrameBufferCmdDesc.Buffer->TextureIdentifier)->Release();
			cmd.DestroyFrameBufferCmdDesc.Buffer->Deleted = true;
		}

		void D3D11RendererAPI::CreateTextureNew(const RenderCommand& cmd)
		{
			TextureCreationInfo* info = cmd.CreateTextureProgramCmdDesc.Info;
			TextureDataView* output = cmd.CreateTextureProgramCmdDesc.Output;

			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = info->Size.x;
			desc.Height = info->Size.y;
			desc.SampleDesc.Count = 1;
			//TODO pass dynamic parameter
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = D3DFormat(info->Format);

			if (info->InitialData)
			{
				D3D11_SUBRESOURCE_DATA subresource{};
				subresource.pSysMem = info->InitialData;
				subresource.SysMemPitch = info->Size.x * sizeof(uint8_t) * GetBytesPerPixel(info->Format);
				ASSERT_D3D_CALL(Context.Device->CreateTexture2D(&desc, &subresource, (ID3D11Texture2D**)&output->Identifier));
			}
			else
				ASSERT_D3D_CALL(Context.Device->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&output->Identifier));

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = 1;
			viewDesc.Format = D3DFormat(info->Format);

			ASSERT_D3D_CALL(Context.Device->CreateShaderResourceView((ID3D11Resource*)output->Identifier, &viewDesc, (ID3D11ShaderResourceView**)&output->View));

			D3D11_SAMPLER_DESC samplerDesc{};
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[3] = 1.0f;
			samplerDesc.MaxLOD = 1.0f;

			ASSERT_D3D_CALL(Context.Device->CreateSamplerState(&samplerDesc, (ID3D11SamplerState**)&output->Sampler));

			output->Size = info->Size;

			delete[] info->InitialData;
			delete info;
		}

		void D3D11RendererAPI::BindTextureNew(const RenderCommand& cmd)
		{
			uint32_t slot = cmd.BindTextureProgramCmdDesc.Slot;
			TextureDataView* tex = cmd.BindTextureProgramCmdDesc.Texture;

			switch (cmd.BindTextureProgramCmdDesc.Stage)
			{
			case RenderingStage::VertexShader:
				Context.DeviceContext->VSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&tex->View);
				Context.DeviceContext->VSSetSamplers(slot, 1, (ID3D11SamplerState**)&tex->Sampler);
				break;

			case RenderingStage::FragmentShader:
				Context.DeviceContext->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)&tex->View);
				Context.DeviceContext->PSSetSamplers(slot, 1, (ID3D11SamplerState**)&tex->Sampler);
				break;
			}
		}

		void D3D11RendererAPI::UpdateTextureNew(const RenderCommand& cmd)
		{
			ID3D11Texture2D* stagingTexture;
			D3D11_TEXTURE2D_DESC desc{};
			desc.Width = cmd.UpdateTextureCmdDesc.Width;
			desc.Height = cmd.UpdateTextureCmdDesc.Height;
			desc.SampleDesc.Count = 1;
			//TODO pass dynamic parameter
			desc.Usage = D3D11_USAGE_STAGING;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.ArraySize = 1;
			desc.MipLevels = 1;
			desc.Format = D3DFormat(cmd.UpdateTextureCmdDesc.Format);

			D3D11_SUBRESOURCE_DATA subresource{};
			subresource.pSysMem = cmd.UpdateTextureCmdDesc.Data;
			subresource.SysMemPitch = desc.Width * sizeof(uint8_t) * GetBytesPerPixel(cmd.UpdateTextureCmdDesc.Format);
			ASSERT_D3D_CALL(Context.Device->CreateTexture2D(&desc, &subresource, &stagingTexture));
			Context.DeviceContext->CopyResource((ID3D11Texture2D*)cmd.UpdateTextureCmdDesc.Texture->Identifier, stagingTexture);
			stagingTexture->Release();

			cmd.UpdateTextureCmdDesc.Texture->Size = { cmd.UpdateTextureCmdDesc.Width , cmd.UpdateTextureCmdDesc.Height };
			delete[] cmd.UpdateTextureCmdDesc.Data;
		}

		void D3D11RendererAPI::DestroyTextureNew(const RenderCommand& cmd)
		{
			((ID3D11SamplerState*)cmd.DestroyTextureCmdDesc.Texture->Sampler)->Release();
			((ID3D11ShaderResourceView*)cmd.DestroyTextureCmdDesc.Texture->View)->Release();
			((ID3D11Texture2D*)cmd.DestroyTextureCmdDesc.Texture->Identifier)->Release();
			cmd.DestroyTextureCmdDesc.Texture->Deleted = true;
		}

		void D3D11RendererAPI::DrawNew(const RenderCommand& cmd)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(cmd.DrawNewCmdDesc.DrawingPrimitive));

			Context.DeviceContext->VSSetShader((ID3D11VertexShader*)cmd.DrawNewCmdDesc.Shader->Identifier, 0, 0);
			Context.DeviceContext->PSSetShader((ID3D11PixelShader*)cmd.DrawNewCmdDesc.Shader->Identifier_1, 0, 0);

			uint32_t offset = 0;
			Context.DeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&cmd.DrawNewCmdDesc.VertexBuffer->Identifier, &cmd.DrawNewCmdDesc.VertexBuffer->Stride, &offset);
			Context.DeviceContext->IASetInputLayout((ID3D11InputLayout*)cmd.DrawNewCmdDesc.VertexBuffer->Layout);

			Context.DeviceContext->Draw(cmd.DrawNewCmdDesc.VertexCount, 0);
		}

		void D3D11RendererAPI::DrawIndexedNew(const RenderCommand& cmd)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(cmd.DrawIndexedCmdDesc.DrawingPrimitive));

			Context.DeviceContext->VSSetShader((ID3D11VertexShader*)cmd.DrawIndexedCmdDesc.Shader->Identifier, 0, 0);
			Context.DeviceContext->PSSetShader((ID3D11PixelShader*)cmd.DrawIndexedCmdDesc.Shader->Identifier_1, 0, 0);

			uint32_t offset = 0;
			Context.DeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&cmd.DrawIndexedCmdDesc.VertexBuffer->Identifier, &cmd.DrawIndexedCmdDesc.VertexBuffer->Stride, &offset);
			Context.DeviceContext->IASetInputLayout((ID3D11InputLayout*)cmd.DrawIndexedCmdDesc.VertexBuffer->Layout);

			Context.DeviceContext->IASetIndexBuffer((ID3D11Buffer*)cmd.DrawIndexedCmdDesc.IndexBuffer->Identifier, DXGI_FORMAT_R32_UINT, 0);

			Context.DeviceContext->DrawIndexed(cmd.DrawIndexedCmdDesc.IndexBuffer->Count, 0, 0);
		}

		void D3D11RendererAPI::DrawIndexedNewWithCustomNumberOfVertices(const RenderCommand& cmd)
		{
			Context.DeviceContext->IASetPrimitiveTopology(GetD3DPrimitive(cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.DrawingPrimitive));

			Context.DeviceContext->VSSetShader((ID3D11VertexShader*)cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.Shader->Identifier, 0, 0);
			Context.DeviceContext->PSSetShader((ID3D11PixelShader*)cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.Shader->Identifier_1, 0, 0);

			uint32_t offset = 0;
			Context.DeviceContext->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.VertexBuffer->Identifier, &cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.VertexBuffer->Stride, &offset);
			Context.DeviceContext->IASetInputLayout((ID3D11InputLayout*)cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.VertexBuffer->Layout);

			Context.DeviceContext->IASetIndexBuffer((ID3D11Buffer*)cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.IndexBuffer->Identifier, DXGI_FORMAT_R32_UINT, 0);

			Context.DeviceContext->DrawIndexed(cmd.DrawIndexedWithCustomNumberOfVerticesCmdDesc.IndexCount, 0, 0);
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
	}
}



#endif