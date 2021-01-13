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
			virtual void ImGuiEndFrame() override;

		private:
			void ClearScreen();
			void RecreateSwapchain(const RenderCommand& cmd);
			void SetRenderTargetApplicationWindow();
			void CreateShaderProgramNew(const RenderCommand& cmd);
			void DestroyShaderProgramNew(const RenderCommand& cmd);
			void CreateVertexBufferNew(const RenderCommand& cmd);
			void UpdateVertexBufferNew(const RenderCommand& cmd);
			void DestroyVertexBufferNew(const RenderCommand& cmd);
			void CreateIndexBufferNew(const RenderCommand& cmd);
			void DestroyIndexBufferNew(const RenderCommand& cmd);
			void CreateUniformBufferNew(const RenderCommand& cmd);
			void BindUniformBufferNew(const RenderCommand& cmd);
			void UpdateUniformBufferNew(const RenderCommand& cmd);
			void DestroyUniformBufferNew(const RenderCommand& cmd);
			void CreateFrameBufferNew(const RenderCommand& cmd);
			void BindFrameBufferAsTextureNew(const RenderCommand& cmd);
			void BindFrameBufferAsRenderTargetNew(const RenderCommand& cmd);
			void BindWindowFrameBufferAsRenderTargetNew(const RenderCommand& cmd);
			void ResizeFrameBufferNew(const RenderCommand& cmd);
			void ReadFrameBufferNew(const RenderCommand& cmd);
			void DestroyFrameBufferNew(const RenderCommand& cmd);
			void CreateTextureNew(const RenderCommand& cmd);
			void BindTextureNew(const RenderCommand& cmd);
			void UpdateTextureNew(const RenderCommand& cmd);
			void DestroyTextureNew(const RenderCommand& cmd);
			void DrawNew(const RenderCommand& cmd);
			void DrawIndexedNew(const RenderCommand& cmd);
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