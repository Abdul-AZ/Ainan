#pragma once

#include "renderer/RendererAPI.h"
#include "renderer/ShaderProgram.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"
#include "renderer/Rectangle.h"

#include "OpenGLRendererContext.h"

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
			virtual void ImGuiNewFrame() override;
			virtual void ImGuiEndFrame() override;
			virtual void DrawImGui(ImDrawData* drawData) override;
			virtual void ClearScreen() override;
			virtual void Present() override;
			virtual void RecreateSwapchain(const glm::vec2& newSwapchainSize) override;
			virtual void SetRenderTargetApplicationWindow() override;

			virtual void SetViewport(const Rectangle& viewport) override;

			virtual RendererContext* GetContext() override { return &Context; };

			virtual void SetBlendMode(RenderingBlendMode blendMode) override;

			OpenGLRendererContext Context;

			static OpenGLRendererAPI& Snigleton() { assert(SingletonInstance); return *SingletonInstance; };

		private:
			static OpenGLRendererAPI* SingletonInstance;
			//imgui data
			double Time = 0.0;
			int32_t AttribLocationVtxPos;
			int32_t AttribLocationVtxUV;
			int32_t AttribLocationVtxColor;
			std::shared_ptr<ShaderProgram> ImGuiShader;
			std::shared_ptr<IndexBuffer> ImGuiIndexBuffer;
			std::shared_ptr<VertexBuffer> ImGuiVertexBuffer;
			uint32_t FontTexture = 0;
		};
	}
}
