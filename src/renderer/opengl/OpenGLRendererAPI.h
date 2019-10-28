#pragma once

#include "renderer/RendererAPI.h"
#include "renderer/ShaderProgram.h"
#include "renderer/IndexBuffer.h"
#include "renderer/Viewport.h"

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
			virtual void DrawInstanced(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount, const unsigned int& objectCount) override;
			virtual void Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer) override;

			virtual void ClearScreen() override;
			virtual void SetViewport(const Viewport& viewport) override;
			virtual Viewport GetCurrentViewport() override;
			virtual RendererType GetType() const override { return RendererType::OpenGL; }
		};
	}
}
