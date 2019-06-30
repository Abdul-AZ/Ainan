#include <pch.h>
#include <glad/glad.h>

#include "OpenGLRendererAPI.h"

namespace ALZ {

	OpenGLRendererAPI::OpenGLRendererAPI()
	{
	}

	void OpenGLRendererAPI::DrawInstanced(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount, const unsigned int& objectCount)
	{
		glDrawArraysInstanced(GetOpenGLPrimitive(primitive), 0, vertexCount, objectCount);
	}

	void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
	{
		glDrawElements(GetOpenGLPrimitive(primitive), indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::ClearScreen()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetViewportSize(const glm::ivec2 lowerLeftCornerPoint, const glm::ivec2& size)
	{
		glViewport(lowerLeftCornerPoint.x, lowerLeftCornerPoint.y, size.x, size.y);
	}

	void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount)
	{
		glDrawArrays(GetOpenGLPrimitive(primitive), 0, vertexCount);
	}
}