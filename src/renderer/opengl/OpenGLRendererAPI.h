#pragma once

#include "renderer/RendererAPI.h"
#include "renderer/ShaderProgram.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/Rectangle.h"

#include "OpenGLRendererContext.h"
#include "OpenGLShaderProgram.h"

#include <glad/glad.h>

namespace Ainan {
	namespace OpenGL {

		constexpr GLenum GetOpenGLPrimitive(const Primitive& primitive)
		{
			switch (primitive)
			{
			case Primitive::Triangles:
				return GL_TRIANGLES;

			case Primitive::TriangleFan:
				return GL_TRIANGLE_FAN;

			case Primitive::Lines:
				return GL_LINES;

			default:
				assert(false);
				return 0;
			}
		}

		class OpenGLRendererAPI : public RendererAPI
		{
		public:
			OpenGLRendererAPI();
			virtual ~OpenGLRendererAPI();

			// Inherited via RendererAPI
			virtual void Draw(ShaderProgram& shader, Primitive primitive, uint32_t vertexCount) override;
			virtual void Draw(ShaderProgram& shader, Primitive primitive, const IndexBuffer& indexBuffer) override;
			virtual void Draw(ShaderProgram& shader, Primitive primitive, const IndexBuffer& indexBuffer, uint32_t vertexCount) override;
			virtual void InitImGui() override;
			virtual void TerminateImGui() override;
			virtual void ImGuiNewFrame() override;
			virtual void ImGuiEndFrame() override;
			virtual void DrawImGui(ImDrawData* drawData) override;
			virtual void ClearScreen() override;
			virtual void RecreateSwapchain(const glm::vec2& newSwapchainSize) override;
			virtual void SetRenderTargetApplicationWindow() override;

			virtual void SetViewport(const Rectangle& viewport) override;

			virtual RendererContext* GetContext() override { return &Context; };

			virtual void SetBlendMode(RenderingBlendMode blendMode) override;

			OpenGLRendererContext Context;

			static OpenGLRendererAPI& Snigleton() { assert(SingletonInstance); return *SingletonInstance; };

		private:
			void Present();

			static OpenGLRendererAPI* SingletonInstance;
			//imgui data
			int32_t AttribLocationVtxPos;
			int32_t AttribLocationVtxUV;
			int32_t AttribLocationVtxColor;
			ShaderProgramDataView ImGuiShader;
			IndexBufferDataView ImGuiIndexBuffer;
			VertexBufferDataView ImGuiVertexBuffer;
			uint32_t FontTexture = 0;

			virtual void ExecuteCommand(RenderCommand cmd) override;
			void DrawWithNewAPI(const RenderCommand& cmd);
			void DrawIndexedWithNewAPI(const RenderCommand& cmd);
			void CreateFrameBuffer(const RenderCommand& cmd);
			void CreateShaderProgramNew(const RenderCommand& cmd);
			void CreateUniformBuffer(const RenderCommand& cmd);
			void DestroyUniformBuffer(const RenderCommand& cmd);
			void CreateIndexBuffer(const RenderCommand& cmd);
			void UpdateUniformBufferNew(const RenderCommand& cmd);
			void BindUniformBufferNew(const RenderCommand& cmd);
			void CreateVertexBuffer(const RenderCommand& cmd);
			void DestroyVertexBuffer(const RenderCommand& cmd);
			void DestroyIndexBuffer(const RenderCommand& cmd);
			void UpdateVertexBufferNew(const RenderCommand& cmd);
			void DrawIndexedWithCustomNumberOfVertices(const RenderCommand& cmd);
			void BlitFrameBufferNew(const RenderCommand& cmd);
			void ReadFrameBuffer(const RenderCommand& cmd);
			void DestroyFrameBufferNew(const RenderCommand& cmd);
			void CreateTexture(const RenderCommand& cmd);
			void UpdateTextureNew(const RenderCommand& cmd);
			void DestroyTexture(const RenderCommand& cmd);
			void DrawNew(const RenderCommand& cmd);
		};
	}
}
