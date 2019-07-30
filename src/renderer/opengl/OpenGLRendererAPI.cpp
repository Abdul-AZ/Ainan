#include <pch.h>
#include <glad/glad.h>

#include "OpenGLRendererAPI.h"

namespace ALZ {
	namespace OpenGL {
		
		static void opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
		{
			if(type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || type == GL_DEBUG_TYPE_PERFORMANCE || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
				std::cout << message << std::endl;
		};

		OpenGLRendererAPI::OpenGLRendererAPI()
		{
#ifdef DEBUG
			glDebugMessageCallback(&opengl_debug_message_callback, nullptr);
#endif // DEBUG
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
}