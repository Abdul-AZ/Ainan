#include <pch.h>
#include <glad/glad.h>

#include "OpenGLVertexBuffer.h"

namespace ALZ {
	namespace OpenGL {

		constexpr GLenum GetOpenglTypeFromShaderType(const ShaderVariableType& type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return GL_INT;

			case ShaderVariableType::UnsignedInt:
				return GL_UNSIGNED_INT;

			case ShaderVariableType::Vec2:
			case ShaderVariableType::Vec3:
			case ShaderVariableType::Vec4:
			case ShaderVariableType::Mat3:
			case ShaderVariableType::Mat4:
				return GL_FLOAT;

			default:
				return 0;
			}
		}

		OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, unsigned int size)
		{
			glGenBuffers(1, &m_RendererID);
			Bind();
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
		}

		OpenGLVertexBuffer::~OpenGLVertexBuffer()
		{
			glDeleteBuffers(1, &m_RendererID);
		}

		void OpenGLVertexBuffer::Bind() const
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		}

		void OpenGLVertexBuffer::Unbind() const
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void OpenGLVertexBuffer::UpdateData(const int& offset, const int& size, void* data)
		{
			Bind();
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
		}

		void OpenGLVertexBuffer::SetLayout(const VertexLayout& layout)
		{
			Bind();

			int index = 0;
			int offset = 0;
			int stride = 0;

			for (auto& type : layout)
			{
				stride += GetShaderVariableSize(type);
			}

			for (auto& type : layout)
			{
				int size = GetShaderVariableSize(type);
				int componentCount = GetShaderVariableComponentCount(type);
				GLenum openglType = GetOpenglTypeFromShaderType(type);

				glVertexAttribPointer(index, componentCount, openglType, false, stride, (void*)offset);
				offset += size;

				glEnableVertexAttribArray(index);
				index++;
			}
		}
	}
}