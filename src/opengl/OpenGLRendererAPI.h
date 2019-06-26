#pragma once

#include "graphics/RendererAPI.h"
#include "graphics/ShaderProgram.h"

#include <glad/glad.h>

namespace ALZ {

	constexpr GLenum GetOpenGLPrimitive(const Primitive& primitive)
	{
		switch (primitive)
		{
		case Primitive::Triangles:
			return GL_TRIANGLES;

		case Primitive::TriangleFan:
			return GL_TRIANGLE_FAN;

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
		virtual void ClearScreen() override;

		// Inherited via RendererAPI
		virtual RendererType GetType() const override { return RendererType::OpenGL; }
	};

}
