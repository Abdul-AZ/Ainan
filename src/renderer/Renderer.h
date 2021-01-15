#pragma once

#include "editor/Camera.h"

#include "RenderCommand.h"
#include "RenderCommandQueue.h"
#include "RendererAPI.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Rectangle.h"
#include "UniformBuffer.h"

namespace Ainan {

	const int32_t c_ApplicationFramerate = 60;
	const double c_ApplicationMaxFramePeriod = 1.0 / c_ApplicationFramerate;
	extern double LastFrameFinishTime;
	extern double LastFrameDeltaTime;

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
		Framebuffer SceneDrawTarget;							   //Required
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
		static void DrawQuad(glm::vec2 position, glm::vec4 color, float scale, Texture texture);
		static void DrawQuad(glm::vec2 position, glm::vec4 color, float scale, float rotationInRadians, Texture texture);
		static void DrawQuadv(glm::vec2* position, glm::vec4* color, float* scale, int32_t count, Texture texture);

		static void Draw(VertexBuffer vertexBuffer, ShaderProgram shader, Primitive primitive, int32_t vertexCount);

		static void Draw(VertexBuffer vertexBuffer, ShaderProgram shader, Primitive primitive, IndexBuffer indexBuffer, uint32_t indexCount);

		static void Draw(VertexBuffer vertexBuffer, ShaderProgram shader, Primitive primitive, IndexBuffer indexBuffer);

		static void ImGuiNewFrame();
		static void ImGuiEndFrame();

		static uint32_t GetUsedGPUMemory();

		static void ClearScreen();

		static void Present();

		static void RecreateSwapchain(const glm::vec2& newSwapchainSize);

		static void PushCommand(RenderCommand cmd);
		static void PushCommand(std::function<void()> func) { return PushCommand(RenderCommand(func)); };

		static void SetBlendMode(RenderingBlendMode blendMode);

		static void SetViewport(const Rectangle& viewport);

		static void SetRenderTargetApplicationWindow();

		static VertexBuffer CreateVertexBuffer(void* data, uint32_t size,
			const VertexLayout& layout, ShaderProgram shaderProgram,
			bool dynamic = false);
		static void DestroyVertexBuffer(VertexBuffer vb);

		//data should ALWAYS a uint32_t array
		static IndexBuffer CreateIndexBuffer(uint32_t* data, uint32_t count);
		static void DestroyIndexBuffer(IndexBuffer ib);

		static UniformBuffer CreateUniformBuffer(const std::string& name, uint32_t reg, const VertexLayout& layout);
		static void DestroyUniformBuffer(UniformBuffer ub);

		//manually create a shader program (mostly used for testing new shaders)
		//to properly add shaders add them to the CompileOnInit list in the cpp file and access them from the ShaderLibrary member
		static ShaderProgram CreateShaderProgram(const std::string& vertPath, const std::string& fragPath);

		static Framebuffer CreateFramebuffer(const glm::vec2& size);
		static void DestroyFramebuffer(Framebuffer fb);

		static Texture CreateTexture(const glm::vec2& size, TextureFormat format, uint8_t* data = nullptr);
		static Texture CreateTexture(Image& img);
		static void DestroyTexture(Texture tex);

		static void FlushQuadBatch();

		//because quad vertices are different in each API depending on if the y axis is pointing up or down
		//this returns 6 quad vertices that are used to draw a quad WITHOUT using an index buffer
		static std::array<glm::vec2, 6> GetQuadVertices();								//layout: vec2 position
		static std::array<std::pair<glm::vec2, glm::vec2>, 6> GetTexturedQuadVertices();//layout: vec2 position vec2 texture_coordinates

		struct RendererData
		{
			RendererType API;

			//sync objects
			std::thread Thread;
			bool DestroyThread = false;
			RenderCommandQueue CommandQueue;

			//GPU objects
			std::unordered_map<uint32_t, VertexBufferDataView> VertexBuffers;
			std::unordered_map<uint32_t, IndexBufferDataView> IndexBuffers;
			std::unordered_map<uint32_t, UniformBufferDataView> UniformBuffers;
			std::unordered_map<uint32_t, ShaderProgramDataView> ShaderPrograms;
			std::unordered_map<uint32_t, FramebufferDataView> Framebuffers;
			std::unordered_map<uint32_t, TextureDataView> Textures;

			//scene data
			RendererAPI* CurrentActiveAPI = nullptr;
			SceneDescription CurrentSceneDescription = {};
			std::unordered_map<std::string, ShaderProgram> ShaderLibrary;
			UniformBuffer SceneUniformBuffer;
			RenderingBlendMode m_CurrentBlendMode = RenderingBlendMode::Additive;
			Rectangle CurrentViewport = { 0, 0, 0, 0 };

			//SceneUniformBuffer
			struct SceneUniformBufferData
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
			SceneUniformBufferData SceneBufferData;
			int32_t RadialLightSubmissionCount = 0;
			int32_t SpotLightSubmissionCount = 0;

			//batch renderer data
			VertexBuffer QuadBatchVertexBuffer;
			IndexBuffer QuadBatchIndexBuffer;
			QuadVertex* QuadBatchVertexBufferDataOrigin = nullptr;
			QuadVertex* QuadBatchVertexBufferDataPtr = nullptr;
			//first one is reserved for blank white texture, so we have c_MaxQuadTexturesPerBatch - 1 textures in total
			Texture WhiteTexture;
			std::array<Texture, c_MaxQuadTexturesPerBatch> QuadBatchTextures;
			uint32_t QuadBatchTextureSlotsUsed = 0;

			//Postprocessing data
			Framebuffer BlurFramebuffer;
			VertexBuffer BlurVertexBuffer;
			UniformBuffer BlurUniformBuffer;

			//profiling data
			uint32_t NumberOfDrawCallsLastScene = 0;
			uint32_t CurrentNumberOfDrawCalls = 0;
			double Time = 0.0;
		};

		static RendererData* Rdata;

		static decltype(Rdata->ShaderLibrary)& ShaderLibrary() { return Rdata->ShaderLibrary; }

	private:
		static void InternalInit(RendererType api);
		static void RendererThreadLoop(RendererType api);
		static void InternalTerminate();
		static void InitImGuiRendering();
		static void DrawImGui(ImDrawData* drawData);
		static void Blur(Framebuffer target, float radius);
		static void CleanupDeletedObjects();
	};

	struct ImGuiViewportDataGlfw
	{
		GLFWwindow* Window;
		bool        WindowOwned;

		ImGuiViewportDataGlfw() { Window = NULL; WindowOwned = false; }
		~ImGuiViewportDataGlfw() { IM_ASSERT(Window == NULL); }
	};
}
