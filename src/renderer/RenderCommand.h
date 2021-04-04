#pragma once

#include "VertexBuffer.h"
#include "ShaderProgram.h"
#include "IndexBuffer.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "UniformBuffer.h"

namespace Ainan {

	//see structs in RenderCommand union for info on what parameters are needed for each command
	enum class RenderCommandType : int32_t
	{
		Clear,
		Present,
		RecreateSweapchain,
		SetViewport,
		SetBlendMode,

		CreateShaderProgram,
		DestroyShaderProgram,

		CreateVertexBuffer,
		UpdateVertexBuffer,
		DestroyVertexBuffer,

		CreateIndexBuffer,
		DestroyIndexBuffer,

		CreateUniformBuffer,
		BindUniformBuffer,
		UpdateUniformBuffer,
		DestroyUniformBuffer,

		CreateFramebuffer,
		BindFramebufferAsTexture,
		BindFramebufferAsRenderTarget, 
		BindBackBufferAsRenderTarget,
		ResizeFramebuffer,
		ReadFramebuffer,
		DestroyFramebuffer, 

		CreateTexture,
		BindTexture,
		UpdateTexture,
		DestroyTexture,

		DrawNew,
		DrawIndexedNew,
		DrawIndexedNewWithCustomNumberOfVertices,

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
		ShaderProgramDataView* Shader;
		bool Dynamic;
	};

	struct IndexBufferCreationInfo
	{
		void* InitialData;
		uint32_t Count;
	};

	struct FramebufferCreationInfo
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

	const char* RenderingBlendModeToStr(RenderingBlendMode mode);
	RenderingBlendMode StrToRenderingBlendMode(std::string str);

	class VertexBuffer;
	class ShaderProgram;
	class IndexBuffer;
	class Framebuffer;
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
		union
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//general commands
			struct ClearCmdDescStruct
			{
			} ClearCmdDesc;

			struct PresentCmdDescStruct
			{
			} PresentCmdDesc;
			
			struct RecreateSweapchainCmdDescStruct
			{
				uint32_t Width;
				uint32_t Height;
			} RecreateSweapchainCmdDesc;
			
			struct SetViewportCmdDescStruct
			{
				uint32_t X;
				uint32_t Y;
				uint32_t Width;
				uint32_t Height;
				float MinDepth;
				float MaxDepth;
			} SetViewportCmdDesc;
			
			struct SetBlendModeCmdDescStruct
			{
				RenderingBlendMode Mode;
			} SetBlendModeCmdDesc;
			
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//shader commands
			struct CreateShaderProgramCmdDescStruct
			{
				//create with new, memory is handled internally
				ShaderProgramCreationInfo* Info;
				ShaderProgramDataView* Output;
			} CreateShaderProgramCmdDesc;

			struct DestroyShaderProgramCmdDescStruct
			{
				ShaderProgramDataView* Program;
			} DestroyShaderProgramCmdDesc;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//vertex buffer commands
			struct CreateVertexBufferCmdDescStruct
			{
				//create with new, memory is handled internally
				VertexBufferCreationInfo* Info;
				VertexBufferDataView* Output;
			} CreateVertexBufferCmdDesc;

			struct UpdateVertexBufferCmdDescStruct
			{
				VertexBufferDataView* VertexBuffer;
				void* Data;
				uint32_t Size;
				uint32_t Offset;
			} UpdateVertexBufferCmdDesc;

			struct DestroyVertexBufferCmdDescStruct
			{
				VertexBufferDataView* Buffer;
			} DestroyVertexBufferCmdDesc;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//index buffer commands
			struct CreateIndexBufferCmdDescStruct
			{
				//create with new, memory is handled internally
				IndexBufferCreationInfo* Info;
				IndexBufferDataView* Output;
			} CreateIndexBufferCmdDesc;

			struct DestroyIndexBufferCmdDescStruct
			{
				IndexBufferDataView* Buffer;
			} DestroyIndexBufferCmdDesc;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//uniform buffer commands
			struct CreateUniformBufferCmdDescStruct
			{
				//create with new, memory is handled internally
				UniformBufferCreationInfo* Info;
				UniformBufferDataView* Output;
			} CreateUniformBufferCmdDesc;

			struct BindUniformBufferCmdDescStruct
			{
				UniformBufferDataView* Buffer;
				RenderingStage Stage;
				uint32_t Slot; //This corrosponds to the slot/index in the shader
			} BindUniformBufferCmdDesc;

			struct UpdateUniformBufferCmdDescStruct
			{
				UniformBufferDataView* Buffer;
				void* Data;
			} UpdateUniformBufferCmdDesc;

			struct DestroyUniformBufferCmdDescStruct
			{
				UniformBufferDataView* Buffer;
			} DestroyUniformBufferCmdDesc;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//framebuffer commands
			struct CreateFramebufferCmdDescStruct
			{
				//create with new, memory is handled internally
				FramebufferCreationInfo* Info;
				FramebufferDataView* Output;
			} CreateFramebufferCmdDesc;
			
			struct BindFramebufferAsTextureCmdDescStruct
			{
				FramebufferDataView* Buffer;
				RenderingStage Stage;
				uint32_t Slot; //This corrosponds to the slot/index in the shader
			} BindFramebufferAsTextureCmdDesc;
			
			struct BindFramebufferAsRenderTargetCmdDescStruct
			{
				FramebufferDataView* Buffer;
			} BindFramebufferAsRenderTargetCmdDesc;

			struct BindBackBufferAsRenderTargetCmdDescStruct
			{
			} BindBackBufferAsRenderTargetCmdDesc;
			
			struct ResizeFramebufferCmdDescStruct
			{
				FramebufferDataView* Buffer;
				uint32_t Width;
				uint32_t Height;
			} ResizeFramebufferCmdDesc;
			
			struct ReadFramebufferCmdDescStruct
			{
				FramebufferDataView* Buffer;
				Image* Output;

				//rectangular area that is read from
				//TODO make these actually work instead of reading the whole buffer
				uint32_t BottomLeftX;
				uint32_t BottomLeftY;
				uint32_t TopRightX;
				uint32_t TopRightY;
			} ReadFramebufferCmdDesc;

			struct DestroyFramebufferCmdDescStruct
			{
				FramebufferDataView* Buffer;
			} DestroyFramebufferCmdDesc;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//texture commands
			struct CreateTextureCmdDescStruct
			{
				TextureCreationInfo* Info;
				TextureDataView* Output;
			} CreateTextureProgramCmdDesc;

			struct BindTextureCmdDescStruct
			{
				TextureDataView* Texture;
				RenderingStage Stage;
				uint32_t Slot; //This corrosponds to the slot/index in the shader
			} BindTextureProgramCmdDesc;
			
			struct UpdateTextureCmdDescStruct
			{
				TextureDataView* Texture;
				uint32_t Width;
				uint32_t Height;
				TextureFormat Format;
				//create with new, memory is handled internally
				void* Data;
			} UpdateTextureCmdDesc;

			struct DestroyTextureCmdDescStruct
			{
				TextureDataView* Texture;
			} DestroyTextureCmdDesc;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//draw commands
			struct DrawNewCmdDescStruct
			{
				VertexBufferDataView* VertexBuffer;
				ShaderProgramDataView* Shader;
				Primitive DrawingPrimitive;
				uint32_t VertexCount;
			} DrawNewCmdDesc;

			struct DrawIndexedCmdDescStruct
			{
				VertexBufferDataView* VertexBuffer;
				IndexBufferDataView* IndexBuffer;
				ShaderProgramDataView* Shader;
				Primitive DrawingPrimitive;
			} DrawIndexedCmdDesc;
			
			struct DrawIndexedWithCustomNumberOfVerticesCmdDescStruct
			{
				VertexBufferDataView* VertexBuffer;
				IndexBufferDataView* IndexBuffer;
				ShaderProgramDataView* Shader;
				Primitive DrawingPrimitive;
				uint32_t IndexCount;
			} DrawIndexedWithCustomNumberOfVerticesCmdDesc;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		};

		//TODO remove this
		std::function<void()> CustomCommand = nullptr;
	};
}