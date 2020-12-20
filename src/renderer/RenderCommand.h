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
		BindUniformBuffer,   //Misc 1 is slot
		UpdateUniformBuffer,

		CreateVertexBuffer,  //requires heap allocated VertexBufferCreationInfo passed in ExtraData
		UpdateVertexBuffer, //VBuffer is populated, Misc 1 is size, Misc2 is offset and ExtraData is a heap allocated memory buffer

		CreateFrameBuffer,  //requires heap allocated FrameBufferCreationInfo passed in ExtraData
		BindFrameBufferAsTexture, //
		BindFrameBufferAsRenderTarget,
		ResizeFrameBuffer, //Misc1 is width, Misc2 is Height
		BlitFrameBuffer, //Misc1 source size and misc 2 is target size, to be interpreted as glm::vec2 each -ExtraData must contain a heap allocated FrameBufferDataView

		CreateTexture,  //requires heap allocated TextureCreationInfo passed in ExtraData
		BindTexture, 
		UpdateTexture,  //Misc1 size to be interpreted as glm::vec2, Misc2 is Format and Extradata is a heap allocated array that will be freed by delete[]

		Draw_NewShader,
		DrawNew, // Requires NewVBuffer, Shader, DrawingPrimitive and Misc1 represents vertex draw count
		DrawIndexedNew,
		DrawIndexedNewWithCustomNumberOfVertices, //Misc 1 is index count

		CustomCommand,
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
		VertexBufferDataView NewVBuffer;
		IndexBufferDataView NewIBuffer;
		ShaderProgramDataView NewShader;
		FrameBufferDataView NewFBuffer;
		Primitive DrawingPrimitive;
		void* ExtraData = nullptr;
		void* Output = nullptr;
		std::function<void()> CustomCommand = nullptr;
		RenderingStage Stage;
		uint64_t Misc1 = 0;
		uint64_t Misc2 = 0;
	};
}