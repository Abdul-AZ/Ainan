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

			virtual void InitImGui() override;
			virtual void TerminateImGui() override;
			virtual void DrawImGui(ImDrawData* drawData) override;
			virtual RendererContext* GetContext() override { return &Context; };
			virtual void SetBlendMode(RenderingBlendMode blendMode) override;
			virtual void ImGuiNewFrame() override;
			virtual void ImGuiEndFrame(bool redraw) override;

		private:
			void ClearScreen();
			void RecreateSwapchain(const RenderCommand& cmd);
			void SetRenderTargetApplicationWindow();
			void CreateShaderProgram(const RenderCommand& cmd);
			void DestroyShaderProgram(const RenderCommand& cmd);
			void CreateVertexBuffer(const RenderCommand& cmd);
			void UpdateVertexBuffer(const RenderCommand& cmd);
			void DestroyVertexBuffer(const RenderCommand& cmd);
			void CreateIndexBuffer(const RenderCommand& cmd);
			void DestroyIndexBuffer(const RenderCommand& cmd);
			void CreateUniformBuffer(const RenderCommand& cmd);
			void BindUniformBuffer(const RenderCommand& cmd);
			void UpdateUniformBuffer(const RenderCommand& cmd);
			void DestroyUniformBuffer(const RenderCommand& cmd);
			void CreateFramebuffer(const RenderCommand& cmd);
			void BindFramebufferAsTexture(const RenderCommand& cmd);
			void BindFramebufferAsRenderTarget(const RenderCommand& cmd);
			void BindWindowFramebufferAsRenderTarget(const RenderCommand& cmd);
			void ResizeFramebuffer(const RenderCommand& cmd);
			void ReadFramebuffer(const RenderCommand& cmd);
			void DestroyFramebuffer(const RenderCommand& cmd);
			void CreateTexture(const RenderCommand& cmd);
			void BindTexture(const RenderCommand& cmd);
			void UpdateTexture(const RenderCommand& cmd);
			void DestroyTexture(const RenderCommand& cmd);
			void DrawNew(const RenderCommand& cmd);
			void DrawIndexed(const RenderCommand& cmd);
			void DrawIndexedNewWithCustomNumberOfVertices(const RenderCommand& cmd);
			void SetViewport(const Rectangle& viewport);
			void SetViewport(const RenderCommand& cmd);
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