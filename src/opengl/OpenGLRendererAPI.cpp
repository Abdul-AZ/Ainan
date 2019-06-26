#include <pch.h>

#include "OpenGLRendererAPI.h"

namespace ALZ {

	OpenGLRendererAPI::OpenGLRendererAPI()
	{
	}

	void OpenGLRendererAPI::DrawInstanced(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount, const unsigned int& objectCount)
	{
		glDrawArraysInstanced(GetOpenGLPrimitive(primitive), 0, vertexCount, objectCount);
	}

	void OpenGLRendererAPI::ClearScreen()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount)
	{
		glDrawArrays(GetOpenGLPrimitive(primitive), 0, vertexCount);
	}
}