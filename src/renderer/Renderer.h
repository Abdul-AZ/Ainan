#pragma once

#include "editor/Camera.h"

#include "RendererAPI.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Rectangle.h"
#include "UniformBuffer.h"

namespace Ainan {

	//used internally for batch rendering
	struct QuadVertex 
	{
		glm::vec2 Position;
		glm::vec4 Color;
		float Texture;
		glm::vec2 TextureCoordinates;
	};

	struct SceneDescription
	{
		Camera SceneCamera = {};								   //Required
		std::shared_ptr<FrameBuffer> SceneDrawTarget = nullptr;    //Required
		std::shared_ptr<Texture> SceneDrawTargetTexture = nullptr; //Required if there is postprocessing effects
		bool Blur = false;										   //Required
		float BlurRadius = 0.0f;								   //Required if Blur == true
	};

	const int c_MaxQuadsPerBatch = 10000;
	const int c_MaxQuadVerticesPerBatch = c_MaxQuadsPerBatch * 4;
	const int c_MaxQuadTexturesPerBatch = 16;

	//this class is completely api agnostic, meaning NO gl calls, NO direct3D calls etc
	class Renderer 
	{
	public:
		//this will initilize all the shaders and put them in the shader library member
		static void Init(RendererType api);

		//this will destroy all the shaders in the shader library member
		static void Terminate();

		//This will be changed to only render in end scene by putting draw commands to a command buffer 
		static void BeginScene(const SceneDescription& desc);
		static void EndScene();

		//position is in world coordinates
		static void DrawQuad(glm::vec2 position, glm::vec4 color, float scale, std::shared_ptr<Texture> texture = nullptr);
		static void DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, std::shared_ptr<Texture> texture = nullptr);
		static void DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int count, std::shared_ptr<Texture> texture = nullptr);

		//these overloads DO NOT use an index buffer
		static void Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& mode,
						 const unsigned int& vertexCount);

		//these overloads DO use an index buffer
		static void Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& primitive,
						 const IndexBuffer& indexBuffer);
		static void Draw(const VertexBuffer& vertexBuffer, ShaderProgram& shader, const Primitive& primitive,
						 const IndexBuffer& indexBuffer, int vertexCount);

		static void ClearScreen();

		static void Present();

		static void RecreateSwapchain(const glm::vec2& newSwapchainSize);

		static void Blur(std::shared_ptr<FrameBuffer>& target, std::shared_ptr<Texture>& targetShader, float radius);

		static void SetBlendMode(RenderingBlendMode blendMode);

		static void SetViewport(const Rectangle& viewport);
		static Rectangle GetCurrentViewport();

		static void SetScissor(const Rectangle& scissor);
		static Rectangle GetCurrentScissor();

		static std::shared_ptr<VertexBuffer> CreateVertexBuffer(void* data, unsigned int size,
			const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
			bool dynamic = false);

		//data should ALWAYS an UNSIGNED INT array
		static std::shared_ptr<IndexBuffer> CreateIndexBuffer(unsigned int* data, const int& count);

		//manually create a shader program (mostly used for testing new shaders)
		//to properly add shaders add them to the CompileOnInit list in the cpp file and access them from the ShaderLibrary member
		static std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertPath, const std::string& fragPath);
		//this is used when you want to create shaders with source code and not from files
		static std::shared_ptr<ShaderProgram> CreateShaderProgramRaw(const std::string& vertSrc, const std::string& fragSrc);

		static std::shared_ptr<FrameBuffer> CreateFrameBuffer();

		static std::shared_ptr<Texture> CreateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);
		static std::shared_ptr<Texture> CreateTexture(Image& img);

		static std::shared_ptr<UniformBuffer> CreateUniformBuffer(const std::string& name, uint32_t reg,
			const VertexLayout& layout, void* data);

		struct RendererData
		{
			//scene data
			RendererAPI* m_CurrentActiveAPI = nullptr;
			SceneDescription m_CurrentSceneDescription = {};
			std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> ShaderLibrary;
			glm::mat4 m_CurrentViewProjection = glm::mat4(1.0f);
			std::shared_ptr<UniformBuffer> SceneUniformbuffer = nullptr;
			RenderingBlendMode m_CurrentBlendMode = RenderingBlendMode::Additive;

			//batch renderer data
			std::shared_ptr<VertexBuffer> m_QuadBatchVertexBuffer = nullptr;
			std::shared_ptr<IndexBuffer> m_QuadBatchIndexBuffer = nullptr;
			QuadVertex* m_QuadBatchVertexBufferDataOrigin = nullptr;
			QuadVertex* m_QuadBatchVertexBufferDataPtr = nullptr;
			//first one is reserved for blank white texture, so we have c_MaxQuadTexturesPerBatch - 1 textures in total
			std::array<std::shared_ptr<Texture>, c_MaxQuadTexturesPerBatch> m_QuadBatchTextures;
			uint32_t m_QuadBatchTextureSlotsUsed = 0;

			//Postprocessing data
			std::shared_ptr<Texture> m_BlurTexture = nullptr;
			std::shared_ptr<FrameBuffer> m_BlurFrameBuffer = nullptr;
			std::shared_ptr<VertexBuffer> m_BlurVertexBuffer = nullptr;
			std::shared_ptr<UniformBuffer> m_BlurUniformBuffer = nullptr;

			//profiling data
			uint32_t NumberOfDrawCallsLastScene = 0;
			uint32_t CurrentNumberOfDrawCalls = 0;
			//refrences to created objects
			std::vector<std::weak_ptr<Texture>> m_ReservedTextures;
			std::vector<std::weak_ptr<VertexBuffer>> m_ReservedVertexBuffers;
			std::vector<std::weak_ptr<IndexBuffer>> m_ReservedIndexBuffers;
		};

		static RendererData* Rdata;

		static decltype(Rdata->ShaderLibrary)& ShaderLibrary() { return Rdata->ShaderLibrary; }

	private:
		static void FlushQuadBatch();
	};

}
