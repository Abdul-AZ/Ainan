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
		virtual void InitImGui() = 0;
		virtual void TerminateImGui() = 0;
		virtual void ImGuiNewFrame() = 0;
		virtual void ImGuiEndFrame() = 0;
		virtual void DrawImGui(ImDrawData* drawData) = 0;
		virtual void ExecuteCommand(RenderCommand cmd) = 0;
		virtual void SetBlendMode(RenderingBlendMode blendMode) = 0;
		virtual RendererContext* GetContext() = 0;
	};
}