#pragma once

#include "RendererContext.h"
#include "RenderCommand.h"

namespace Ainan {

	class ShaderProgram;
	class IndexBuffer;
	class Texture;
	struct Rectangle;

	//pure virtual class (interface) for each renderer api to inherit from
	class RendererAPI
	{
	public:
		virtual ~RendererAPI() {};
		virtual void Draw(ShaderProgram& shader, Primitive mode, uint32_t vertexCount) = 0;
		virtual void Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer) = 0;
		virtual void Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer, uint32_t vertexCount) = 0;

		virtual void InitImGui() = 0;
		virtual void ImGuiNewFrame() = 0;
		virtual void ImGuiEndFrame() = 0;
		virtual void DrawImGui(ImDrawData* drawData) = 0;

		virtual void ExecuteCommand(RenderCommand cmd) = 0;

		virtual void ClearScreen() = 0;

		virtual void SetRenderTargetApplicationWindow() = 0;

		virtual void RecreateSwapchain(const glm::vec2& newSwapchainSize) = 0;

		virtual void SetBlendMode(RenderingBlendMode blendMode) = 0;

		virtual void SetViewport(const Rectangle& viewport) = 0;

		virtual RendererContext* GetContext() = 0;
	};
}