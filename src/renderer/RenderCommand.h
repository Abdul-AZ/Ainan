#pragma once

//TEMP
#include "VertexBuffer.h"

namespace Ainan {

	enum class RenderCommandType : int32_t
	{
		ClearScreen,
		Present,
		CreateShaderProgram, //requires heap allocated ShaderProgramCreationInfo passed in ExtraData
		CreateUniformBuffer, //requires heap allocated UniformBufferCreationInfo passed in ExtraData
		CreateVertexBuffer,  //requires heap allocated VertexBufferCreationInfo passed in ExtraData
		UpdateUniformBuffer,
		BindUniformBuffer,
		CustomCommand,
		Draw_NewShader,
		DrawIndexed, //Requires VBuffer, IBuffer, Shader and DrawingPrimitive set
		Unspecified
	};

	struct ShaderProgramCreationInfo
	{
		std::string vertPath;
		std::string fragPath;
	};

	struct UniformBufferCreationInfo
	{
		std::string Name;
		uint32_t reg;
		VertexLayout layout;
	};

	struct VertexBufferCreationInfo
	{
		void* InitialData;
		uint32_t Size;
		VertexLayout Layout;
		uint64_t Shader;
		bool Dynamic;
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

	enum class RenderingStage : uint32_t
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
		VertexBufferDataView NewVBuffer;
		std::shared_ptr<IndexBuffer> IBuffer = nullptr;
		std::shared_ptr<Texture> Tex = nullptr;
		std::shared_ptr<FrameBuffer> FBuffer = nullptr;
		std::array<std::shared_ptr<UniformBuffer>, MAX_UNIFORM_BUFFERS_PER_COMMAND> UBuffers;
		std::array<void*, MAX_UNIFORM_BUFFERS_PER_COMMAND> UBuffersData;
		bool FreeUniformDataAfterUse = false;
		Primitive DrawingPrimitive;
		void* ExtraData = nullptr;
		void* Output = nullptr;
		std::function<void()> CustomCommand = nullptr;

		uint64_t Misc1 = 0;
		uint64_t Misc2 = 0;
	};
}