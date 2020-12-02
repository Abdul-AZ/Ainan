#pragma once

namespace Ainan {

	enum class RenderCommandType : int32_t
	{
		ClearScreen,
		Present,
		CustomCommand,
		DrawIndexed, //Requires VBuffer, IBuffer, Shader and DrawingPrimitive set
		Unspecified
	};


	enum class Primitive
	{
		Triangles,
		TriangleFan,
		Lines
	};

	enum class RenderingBlendMode
	{
		Additive,
		Screen,
		Overlay,
		NotSpecified //this will use the current mode it is set on
	};

	enum class RenderingStage
	{
		VertexShader,
		FragmentShader
	};

	class VertexBuffer;
	class ShaderProgram;
	class IndexBuffer;
	class FrameBuffer;
	class Texture;
	class UniformBuffer;

#define MAX_UNIFORM_BUFFERS_PER_COMMAND 5

	struct RenderCommand
	{
		RenderCommand() = default;

		RenderCommand(std::function<void()> func)
		{
			Type = RenderCommandType::CustomCommand;
			CustomCommand = func;
		}

		RenderCommandType Type = RenderCommandType::Unspecified;
		std::shared_ptr<ShaderProgram> Shader = nullptr;
		std::shared_ptr<VertexBuffer> VBuffer = nullptr;
		std::shared_ptr<IndexBuffer> IBuffer = nullptr;
		std::shared_ptr<Texture> Tex = nullptr;
		std::shared_ptr<FrameBuffer> FBuffer = nullptr;
		std::array<std::shared_ptr<UniformBuffer>, MAX_UNIFORM_BUFFERS_PER_COMMAND> UBuffers;
		std::array<void*, MAX_UNIFORM_BUFFERS_PER_COMMAND> UBuffersData;
		bool FreeUniformDataAfterUse = false;
		Primitive DrawingPrimitive;
		void* ExtraData = nullptr;
		std::function<void()> CustomCommand = nullptr;
	};
}