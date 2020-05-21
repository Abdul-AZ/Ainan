#include <pch.h>
#include <glad/glad.h>

#include "OpenGLUniformBuffer.h"

namespace Ainan {
	namespace OpenGL {

		uint32_t GetBasestd140Alignemnt(ShaderVariableType type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return 4;
				break;
			case ShaderVariableType::UnsignedInt:
				return 4;
				break;
			case ShaderVariableType::Float:
				return 4;
				break;
			case ShaderVariableType::Vec2:
				return 8;
				break;
			case ShaderVariableType::Vec3:
				return 16;
				break;
			case ShaderVariableType::Vec4:
				return 16;
				break;
			case ShaderVariableType::Mat3:
				return 16;
				break;
			case ShaderVariableType::Mat4:
				return 16;
				break;
			default:
				break;
			}
		}

		OpenGLUniformBuffer::OpenGLUniformBuffer(const std::string& name, const VertexLayout& layout, void* data) :
			m_Name(name), 
			m_Layout(layout)
		{
			//calculate std140 layout size
			for (auto& layoutPart : m_Layout)
			{
				uint32_t size = GetShaderVariableSize(layoutPart.Type);
				uint32_t baseAlignment = GetBasestd140Alignemnt(layoutPart.Type);

				m_Size += m_Size % baseAlignment == 0 ? 0 : baseAlignment - (m_Size % baseAlignment);
				m_Size += size;
			}
			m_Size += m_Size % 16 == 0 ? 0 : 16 - (m_Size % 16);
			m_BufferMemory = new uint8_t[m_Size];

			glGenBuffers(1, &m_RendererID);
			glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
			glBufferData(GL_UNIFORM_BUFFER, m_Size, data, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		OpenGLUniformBuffer::~OpenGLUniformBuffer()
		{
			glDeleteBuffers(1, &m_RendererID);
			delete[] m_BufferMemory;
		}

		void OpenGLUniformBuffer::UpdateData(void* data)
		{
			//align data with std140 uniform layouts and put the result in m_BufferMemory
			uint32_t unalignedDataIndex = 0;
			uint32_t alignedDataIndex = 0;
			uint8_t* unalignedData = (uint8_t*)data;
			for (auto& layoutPart : m_Layout)
			{
				uint32_t size = GetShaderVariableSize(layoutPart.Type);
				uint32_t baseAlignment = GetBasestd140Alignemnt(layoutPart.Type);

				alignedDataIndex += m_Size % baseAlignment == 0 ? 0 : baseAlignment - (m_Size % baseAlignment);
				memcpy(&m_BufferMemory[alignedDataIndex], &unalignedData[unalignedDataIndex], size);
				alignedDataIndex += size;
				unalignedDataIndex += size;
			}

			glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, m_Size, m_BufferMemory);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void OpenGLUniformBuffer::Bind(uint32_t index)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, index, m_RendererID, 0, m_Size);
		}
	}
}