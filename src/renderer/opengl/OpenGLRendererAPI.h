#pragma once

#include "renderer/RendererAPI.h"
#include "renderer/ShaderProgram.h"
#include "renderer/IndexBuffer.h"
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

			// Inherited via RendererAPI
			virtual void Draw(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount) override;
			virtual void Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer) override;
			virtual void Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer, int vertexCount) override;
			virtual void ClearScreen() override;
			virtual void Present() override;
			virtual void RecreateSwapchain(const glm::vec2& newSwapchainSize) override;

			virtual void SetViewport(const Rectangle& viewport) override;
			virtual Rectangle GetCurrentViewport() override;

			virtual void SetScissor(const Rectangle& scissor) override;
			virtual Rectangle GetCurrentScissor() override;

			virtual RendererContext* GetContext() override { return &Context; };

			virtual void SetBlendMode(RenderingBlendMode blendMode) override;

			OpenGLRendererContext Context;
		};
	}
}
