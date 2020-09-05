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

		OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, uint32_t size, const VertexLayout& layout, bool dynamic) :
			Memory(size)
		{
			glGenVertexArrays(1, &m_VertexArray);
			glBindVertexArray(m_VertexArray);

			//create buffer
			glGenBuffers(1, &m_RendererID);
			Bind();
			if(dynamic)
				glBufferData(GL_ARRAY_BUFFER, Memory, data, GL_DYNAMIC_DRAW);
			else
				glBufferData(GL_ARRAY_BUFFER, Memory, data, GL_STATIC_DRAW);

			//set layout
			int32_t index = 0;
			int32_t offset = 0;
			int32_t stride = 0;

			for (auto& layoutPart : layout)
			{
				stride += layoutPart.GetSize();
			}

			for (auto& layoutPart : layout)
			{
				int32_t size = layoutPart.GetSize();
				int32_t componentCount = GetShaderVariableComponentCount(layoutPart.Type);
				GLenum openglType = GetOpenglTypeFromShaderType(layoutPart.Type);

				glVertexAttribPointer(index, componentCount, openglType, false, stride, (void*)(uintptr_t)offset);
				offset += size;

				glEnableVertexAttribArray(index);
				index++;
			}
		}

		OpenGLVertexBuffer::~OpenGLVertexBuffer()
		{
			uint32_t rendererID = m_RendererID;
			auto func = [rendererID]()
			{
				glDeleteBuffers(1, &rendererID);
			};

			Renderer::PushCommand(func);
		}

		void OpenGLVertexBuffer::Bind() const
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
			glBindVertexArray(m_VertexArray);
		}

		void OpenGLVertexBuffer::Unbind() const
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		void OpenGLVertexBuffer::UpdateData(int32_t offset, int32_t size, void* data)
		{
			auto func = [this, offset, size, data]()
			{
				UpdateDataUnsafe(offset, size, data);
			};

			Renderer::PushCommand(func);
			Renderer::WaitUntilRendererIdle();
		}

		void OpenGLVertexBuffer::UpdateDataUnsafe(int32_t offset, int32_t size, void* data)
		{
			Bind();
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
		}
	}
}