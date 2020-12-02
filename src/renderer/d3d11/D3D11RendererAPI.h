#pragma once
#ifdef PLATFORM_WINDOWS

#include "renderer/RendererAPI.h"
#include "renderer/Rectangle.h"
#include "editor/Window.h"

#include <d3d11.h>

#include "D3D11RendererContext.h"

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererAPI : public RendererAPI
		{
		public:
			D3D11RendererAPI();
			virtual ~D3D11RendererAPI();

			virtual void ExecuteCommand(RenderCommand cmd) override;

			virtual void Draw(ShaderProgram& shader, Primitive mode, uint32_t vertexCount) override;
			virtual void Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer) override;
			virtual void Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer, uint32_t vertexCount) override;
			virtual void InitImGui() override;
			virtual void DrawImGui(ImDrawData* drawData) override;
			virtual void ClearScreen() override;
			virtual void RecreateSwapchain(const glm::vec2& newSwapchainSize) override;
			virtual void SetViewport(const Rectangle& viewport) override;
			virtual RendererContext* GetContext() override { return &Context; };
			virtual void SetBlendMode(RenderingBlendMode blendMode) override;
			virtual void SetRenderTargetApplicationWindow() override;
			virtual void ImGuiNewFrame() override;
			virtual void ImGuiEndFrame() override;

		public:
			void Present();
			D3D11RendererContext Context;
			ID3D11BlendState* AdditiveBlendMode;
			ID3D11BlendState* ScreenBlendMode;
			ID3D11BlendState* OverlayBlendMode;
		};

	}
}


#endif