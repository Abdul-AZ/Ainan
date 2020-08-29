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

	//lighting constants
	const int32_t c_MaxRadialLightCount = 10;
	const int32_t c_MaxSpotLightCount = 10;

	//batch renderer constants
	const int32_t c_MaxQuadsPerBatch = 5000;
	const int32_t c_MaxQuadVerticesPerBatch = c_MaxQuadsPerBatch * 4;
	const int32_t c_MaxQuadTexturesPerBatch = 16;

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
		std::shared_ptr<FrameBuffer>* SceneDrawTarget = nullptr;   //Required
		bool Blur = false;										   //Required
		float BlurRadius = 0.0f;								   //Required if Blur == true

		void AddRadialLight(const glm::vec2 position, const glm::vec4 color, float intensity)
		{
			RadialLightPositions[NumRadialLightsUsed] = position;
			RadialLightColors[NumRadialLightsUsed] = color;
			RadialLightIntensities[NumRadialLightsUsed] = intensity;
			NumRadialLightsUsed++;
		}

		void ClearLightingData()
		{
			NumRadialLightsUsed = 0;
			memset(RadialLightPositions.data(), 0, sizeof(RadialLightPositions));
			memset(RadialLightColors.data(), 0, sizeof(RadialLightColors));
			memset(RadialLightIntensities.data(), 0, sizeof(RadialLightIntensities));
		}
		
		int32_t NumRadialLightsUsed = 0;
		std::array<glm::vec2, c_MaxRadialLightCount> RadialLightPositions;
		std::array<glm::vec4, c_MaxRadialLightCount> RadialLightColors;
		std::array<float, c_MaxRadialLightCount> RadialLightIntensities;
	};

	//this class is completely api agnostic, meaning NO gl calls, NO direct3D calls etc
	class Renderer 
	{
	public:
		//this initilizes the renderer and starts the rendering thread
		static void Init(RendererType api);

		//this terminates the renderer and stops the rendering thread
		static void Terminate();

		//This will be changed to only render in end scene by putting draw commands to a command buffer 
		static void BeginScene(const SceneDescription& desc);
		static void AddRadialLight(const glm::vec2& pos, const glm::vec4& color, float intensity);
		static void AddSpotLight(const glm::vec2& pos, const glm::vec4 color, float angle, float innerCutoff, float outerCutoff, float intensity);
		static void EndScene();

		static void WaitUntilRendererIdle();

		//position is in world coordinates
		static void DrawQuad(glm::vec2 position, glm::vec4 color, float scale, std::shared_ptr<Texture> texture = nullptr);
		static void DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, std::shared_ptr<Texture> texture = nullptr);
		static void DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int32_t count, std::shared_ptr<Texture> texture = nullptr);

		//these overloads DO NOT use an index buffer
		static void Draw(const std::shared_ptr<VertexBuffer>& vertexBuffer, std::shared_ptr<ShaderProgram>& shader, Primitive mode,
						 const uint32_t vertexCount);

		//these overloads DO use an index buffer
		static void Draw(const std::shared_ptr<VertexBuffer>& vertexBuffer, std::shared_ptr<ShaderProgram>& shader, Primitive primitive,
						 const std::shared_ptr<IndexBuffer>& indexBuffer);
		static void Draw(const std::shared_ptr<VertexBuffer>& vertexBuffer, std::shared_ptr<ShaderProgram>& shader, Primitive primitive,
						 const std::shared_ptr<IndexBuffer>& indexBuffer, int32_t vertexCount);

		static void ImGuiNewFrame();
		static void ImGuiEndFrame();

		static uint32_t GetUsedGPUMemory();

		static void ClearScreen();
		static void ClearScreenUnsafe();

		static void Present();

		static void RecreateSwapchain(const glm::vec2& newSwapchainSize);

		static void PushCommand(std::function<void()> func);

		static void SetBlendMode(RenderingBlendMode blendMode);

		static void SetViewport(const Rectangle& viewport);
		static Rectangle GetCurrentViewport();

		static void SetRenderTargetApplicationWindow();

		static std::shared_ptr<VertexBuffer> CreateVertexBuffer(void* data, uint32_t size,
			const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
			bool dynamic = false);

		//data should ALWAYS a uint32_t array
		static std::shared_ptr<IndexBuffer> CreateIndexBuffer(uint32_t* data, uint32_t count);

		static std::shared_ptr<UniformBuffer> CreateUniformBuffer(const std::string& name, uint32_t reg,
			const VertexLayout& layout, void* data);

		//manually create a shader program (mostly used for testing new shaders)
		//to properly add shaders add them to the CompileOnInit list in the cpp file and access them from the ShaderLibrary member
		static std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertPath, const std::string& fragPath);
		//this is used when you want to create shaders with source code and not from files
		static std::shared_ptr<ShaderProgram> CreateShaderProgramRaw(const std::string& vertSrc, const std::string& fragSrc);

		static std::shared_ptr<FrameBuffer> CreateFrameBuffer(const glm::vec2& size);

		static std::shared_ptr<Texture> CreateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);
		static std::shared_ptr<Texture> CreateTexture(Image& img);

		static void FlushQuadBatch();

		//because quad vertices are different in each API depending on if the y axis is pointing up or down
		//this returns 6 quad vertices that are used to draw a quad WITHOUT using an index buffer
		static std::array<glm::vec2, 6> GetQuadVertices();								//layout: vec2 position
		static std::array<std::pair<glm::vec2, glm::vec2>, 6> GetTexturedQuadVertices();//layout: vec2 position vec2 texture_coordinates

		struct RendererData
		{
			//sync objects
			std::thread Thread;
			bool DestroyThread = false;
			std::mutex DataMutex;
			std::queue<std::function<void()>> CommandBuffer;
			
			std::mutex QueueMutex;
			std::condition_variable cv;
			std::atomic_bool payload = false;
			std::condition_variable WorkDoneCV;
			std::mutex WorkDoneMutex;

			//scene data
			RendererAPI* CurrentActiveAPI = nullptr;
			SceneDescription CurrentSceneDescription = {};
			std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> ShaderLibrary;
			std::shared_ptr<UniformBuffer> SceneUniformbuffer = nullptr;
			RenderingBlendMode m_CurrentBlendMode = RenderingBlendMode::Additive;
			Rectangle CurrentViewport = { 0, 0, 0, 0 };

			//SceneUniformBuffer
			struct SceneUniformBuffer
			{
				glm::mat4 CurrentViewProjection = glm::mat4(1.0f);


				//point lights
				std::array<glm::vec2, c_MaxRadialLightCount> RadialLightPositions;
				std::array<glm::vec4, c_MaxRadialLightCount> RadialLightColors;
				std::array<float, c_MaxRadialLightCount> RadialLightIntensities;

				//spot lights
				std::array<glm::vec2, c_MaxSpotLightCount>  SpotLightPositions;
				std::array<glm::vec4, c_MaxSpotLightCount>  SpotLightColors;
				std::array<float, c_MaxSpotLightCount> SpotLightAngles;
				std::array<float, c_MaxSpotLightCount> SpotLightInnerCutoffs;
				std::array<float, c_MaxSpotLightCount> SpotLightOuterCutoffs;
				std::array<float, c_MaxSpotLightCount> SpotLightIntensities;
			};
			SceneUniformBuffer SceneBuffer;
			int32_t RadialLightSubmissionCount = 0;
			int32_t SpotLightSubmissionCount = 0;

			//batch renderer data
			std::shared_ptr<VertexBuffer> QuadBatchVertexBuffer = nullptr;
			std::shared_ptr<IndexBuffer> QuadBatchIndexBuffer = nullptr;
			QuadVertex* QuadBatchVertexBufferDataOrigin = nullptr;
			QuadVertex* QuadBatchVertexBufferDataPtr = nullptr;
			//first one is reserved for blank white texture, so we have c_MaxQuadTexturesPerBatch - 1 textures in total
			std::array<std::shared_ptr<Texture>, c_MaxQuadTexturesPerBatch> QuadBatchTextures;
			uint32_t QuadBatchTextureSlotsUsed = 0;

			//Postprocessing data
			std::shared_ptr<FrameBuffer> BlurFrameBuffer = nullptr;
			std::shared_ptr<VertexBuffer> BlurVertexBuffer = nullptr;
			std::shared_ptr<UniformBuffer> BlurUniformBuffer = nullptr;

			//profiling data
			uint32_t NumberOfDrawCallsLastScene = 0;
			uint32_t CurrentNumberOfDrawCalls = 0;
			//refrences to created objects
			std::vector<std::weak_ptr<Texture>> ReservedTextures;
			std::vector<std::weak_ptr<VertexBuffer>> ReservedVertexBuffers;
			std::vector<std::weak_ptr<IndexBuffer>> ReservedIndexBuffers;
			std::vector<std::weak_ptr<UniformBuffer>> ReservedUniformBuffers;
			double Time = 0.0;
		};

		static RendererData* Rdata;

		static decltype(Rdata->ShaderLibrary)& ShaderLibrary() { return Rdata->ShaderLibrary; }

	private:
		static std::shared_ptr<VertexBuffer> CreateVertexBufferUnsafe(void* data, uint32_t size,
			const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
			bool dynamic = false);

		static std::shared_ptr<IndexBuffer> CreateIndexBufferUnsafe(uint32_t* data, uint32_t count);

		static std::shared_ptr<UniformBuffer> CreateUniformBufferUnsafe(const std::string& name, uint32_t reg,
			const VertexLayout& layout, void* data);

		static std::shared_ptr<FrameBuffer> CreateFrameBufferUnsafe(const glm::vec2& size);

		static std::shared_ptr<Texture> CreateTextureUnsafe(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);

		static void InternalInit(RendererType api);
		static void RendererThreadLoop();
		static void InternalTerminate();
		static void DrawImGui(ImDrawData* drawData);
		static void Blur(std::shared_ptr<FrameBuffer>& target, float radius);
	};

	struct ImGuiViewportDataGlfw
	{
		GLFWwindow* Window;
		bool        WindowOwned;

		ImGuiViewportDataGlfw() { Window = NULL; WindowOwned = false; }
		~ImGuiViewportDataGlfw() { IM_ASSERT(Window == NULL); }
	};
}
