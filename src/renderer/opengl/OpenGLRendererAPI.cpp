#include <pch.h>
#include <glad/glad.h>

#include "OpenGLRendererAPI.h"
#include "editor/Window.h"
#include <GLFW/glfw3.h>

namespace Ainan {
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

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		void OpenGLRendererAPI::DrawInstanced(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount, const unsigned int& objectCount)
		{
			glDrawArraysInstanced(GetOpenGLPrimitive(primitive), 0, vertexCount, objectCount);
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer)
		{
			glDrawElements(GetOpenGLPrimitive(primitive), indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const IndexBuffer& indexBuffer, int vertexCount)
		{
			glDrawElements(GetOpenGLPrimitive(primitive), vertexCount, GL_UNSIGNED_INT, nullptr);
		}

		void OpenGLRendererAPI::ClearScreen()
		{
			glClear(GL_COLOR_BUFFER_BIT);
		}

		void OpenGLRendererAPI::SetViewport(const Rectangle& viewport)
		{
			glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
		}

		Rectangle OpenGLRendererAPI::GetCurrentViewport()
		{
			Rectangle viewport;

			glGetIntegerv(GL_VIEWPORT, &viewport.X);

			return viewport;
		}

		void OpenGLRendererAPI::SetScissor(const Rectangle& scissor)
		{
			glScissor(scissor.X, scissor.Y, scissor.Width, scissor.Height);
		}

		Rectangle OpenGLRendererAPI::GetCurrentScissor()
		{
			Rectangle scissor;

			glGetIntegerv(GL_SCISSOR_BOX, &scissor.X);

			return scissor;
		}

		void OpenGLRendererAPI::SetBlendMode(RenderingBlendMode blendMode)
		{
			glEnable(GL_BLEND);

			switch (blendMode)
			{
			case RenderingBlendMode::Additive:
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				break;
			case RenderingBlendMode::Screen:
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				break;
			}
		}

		void OpenGLRendererAPI::Present()
		{
			glfwSwapBuffers(Window::Ptr);
			Window::WindowSizeChangedSinceLastFrame = false;
		}

		void OpenGLRendererAPI::Draw(ShaderProgram& shader, const Primitive& primitive, const unsigned int& vertexCount)
		{
			glDrawArrays(GetOpenGLPrimitive(primitive), 0, vertexCount);
		}
	}
}