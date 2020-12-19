#pragma once

//TEMP
#include "VertexBuffer.h"
#include "ShaderProgram.h"
#include "IndexBuffer.h"
#include "FrameBuffer.h"
#include "Texture.h"

namespace Ainan {

	enum class RenderCommandType : int32_t
	{
		ClearScreen,
		Present,
		CreateShaderProgram, //requires heap allocated ShaderProgramCreationInfo passed in ExtraData
		CreateIndexBuffer, //requires heap allocated IndexBufferCreationInfo passed in ExtraData
		CreateUniformBuffer, //requires heap allocated UniformBufferCreationInfo passed in ExtraData
		CreateVertexBuffer,  //requires heap allocated VertexBufferCreationInfo passed in ExtraData
		CreateFrameBuffer,  //requires heap allocated FrameBufferCreationInfo passed in ExtraData
		CreateTexture,  //requires heap allocated TextureCreationInfo passed in ExtraData
		UpdateTexture,  //Misc1 size to be interpreted as glm::vec2, Misc2 is Format and Extradata is a heap allocated array that will be freed by delete[]
		UpdateVertexBuffer, //VBuffer is populated, Misc 1 is size, Misc2 is offset and ExtraData is a heap allocated memory buffer
		UpdateUniformBuffer,
		BindUniformBuffer, //Misc 1 is slot
		BindTexture, //
		BindFrameBufferAsTexture, //
		BindFrameBufferAsRenderTarget,
		CustomCommand,
		Draw_NewShader,
		DrawNew, // Requires NewVBuffer, Shader, DrawingPrimitive and Misc1 represents vertex draw count
		DrawIndexed, //Requires VBuffer, IBuffer, Shader and DrawingPrimitive set
		DrawIndexedNew,
		DrawIndexedNewWithCustomNumberOfVertices, //Misc 1 is index count
		ResizeFrameBuffer, //Misc1 is width, Misc2 is Height
		BlitFrameBuffer, //Misc1 source size and misc 2 is target size, to be interpreted as glm::vec2 each -ExtraData must contain a heap allocated FrameBufferDataView
		Unspecified
	};

	struct ShaderProgramCreationInfo
	{
		std::string vertPath;
		std::string fragPath;
	};

	struct TextureCreationInfo
	{
		glm::vec2 Size;
		TextureFormat Format;
		uint8_t* InitialData;
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

	struct IndexBufferCreationInfo
	{
		void* InitialData;
		uint32_t Count;
	};

	struct FrameBufferCreationInfo
	{
		glm::vec2 Size;
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
		TextureDataView* NewTex;
		std::shared_ptr<ShaderProgram> Shader = nullptr;
		std::shared_ptr<VertexBuffer> VBuffer = nullptr;
		VertexBufferDataView NewVBuffer;
		std::shared_ptr<IndexBuffer> IBuffer = nullptr;
		IndexBufferDataView NewIBuffer;
		ShaderProgramDataView NewShader;
		std::shared_ptr<FrameBuffer> FBuffer = nullptr;
		FrameBufferDataView NewFBuffer;
		std::array<std::shared_ptr<UniformBuffer>, MAX_UNIFORM_BUFFERS_PER_COMMAND> UBuffers;
		std::array<void*, MAX_UNIFORM_BUFFERS_PER_COMMAND> UBuffersData;
		bool FreeUniformDataAfterUse = false;
		Primitive DrawingPrimitive;
		void* ExtraData = nullptr;
		void* Output = nullptr;
		std::function<void()> CustomCommand = nullptr;

		RenderingStage Stage;

		uint64_t Misc1 = 0;
		uint64_t Misc2 = 0;
	};
}