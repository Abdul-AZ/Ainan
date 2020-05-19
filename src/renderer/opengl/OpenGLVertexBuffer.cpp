#include <pch.h>
#include <glad/glad.h>

#include "OpenGLVertexBuffer.h"

namespace Ainan {
	namespace OpenGL {

		constexpr GLenum GetOpenglTypeFromShaderType(const ShaderVariableType& type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return GL_INT;

			case ShaderVariableType::UnsignedInt:
				return GL_UNSIGNED_INT;

			case ShaderVariableType::Float:
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

		OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, unsigned int size, const VertexLayout& layout, bool dynamic)
		{
			//create buffer
			glGenBuffers(1, &m_RendererID);
			Bind();
			if(dynamic)
				glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
			else
				glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

			//set layout
			int index = 0;
			int offset = 0;
			int stride = 0;

			for (auto& layoutPart : layout)
			{
				stride += GetShaderVariableSize(layoutPart.Type);
			}

			for (auto& layoutPart : layout)
			{
				int size = GetShaderVariableSize(layoutPart.Type);
				int componentCount = GetShaderVariableComponentCount(layoutPart.Type);
				GLenum openglType = GetOpenglTypeFromShaderType(layoutPart.Type);

				glVertexAttribPointer(index, componentCount, openglType, false, stride, (void*)(uintptr_t)offset);
				offset += size;

				glEnableVertexAttribArray(index);
				index++;
			}
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
	}
}