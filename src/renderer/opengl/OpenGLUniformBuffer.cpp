#include <pch.h>
#include <glad/glad.h>

#include "OpenGLUniformBuffer.h"

#include <numeric>

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
				return 4 * 4;
				break;
			}
		}

		OpenGLUniformBuffer::OpenGLUniformBuffer(const std::string& name, const VertexLayout& layout, void* data) :
			m_Name(name), 
			m_Layout(layout)
		{
			m_PackedSize = std::accumulate(layout.begin(), layout.end(), 0,
				[](const uint32_t& a, const VertexLayoutPart& b)
				{
					return a + b.GetSize();
				});

			//calculate std140 layout size
			for (auto& layoutPart : m_Layout)
			{
				uint32_t size = layoutPart.GetSize();

				//if its an array
				if (layoutPart.Count > 1)
				{
					for (size_t i = 0; i < layoutPart.Count; i++)
					{
						m_AlignedSize += m_AlignedSize % 16 == 0 ? 0 : 16 - (m_AlignedSize % 16);
						m_AlignedSize += size / layoutPart.Count;
					}
				}
				else
				{
					uint32_t baseAlignment = GetBasestd140Alignemnt(layoutPart.Type);
					m_AlignedSize += m_AlignedSize % baseAlignment == 0 ? 0 : baseAlignment - (m_AlignedSize % baseAlignment);
					m_AlignedSize += size;
				}
			}
			m_AlignedSize += m_AlignedSize % 16 == 0 ? 0 : 16 - (m_AlignedSize % 16);
			m_BufferMemory = new uint8_t[m_AlignedSize]();

			glGenBuffers(1, &m_RendererID);
			glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
			glBufferData(GL_UNIFORM_BUFFER, m_AlignedSize, data, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		OpenGLUniformBuffer::~OpenGLUniformBuffer()
		{
			auto rendererID = m_RendererID;
			auto func = [rendererID]()
			{
				glDeleteBuffers(1, &rendererID);
			};
			Renderer::PushCommand(func);
			delete[] m_BufferMemory;
		}

		void OpenGLUniformBuffer::UpdateData(void* data)
		{
			auto func = [this, data]()
			{
				UpdateDataUnsafe(data);
			};
			Renderer::PushCommand(func);
			Renderer::WaitUntilRendererIdle(); //TODO reference count the data instead of waiting
		}

		void OpenGLUniformBuffer::UpdateDataUnsafe(void* data)
		{
			//align data with std140 uniform layouts and put the result in m_BufferMemory
			uint32_t unalignedDataIndex = 0;
			uint32_t alignedDataIndex = 0;
			uint8_t* unalignedData = (uint8_t*)data;
			for (auto& layoutPart : m_Layout)
			{
				uint32_t size = layoutPart.GetSize();
				if (layoutPart.Count > 1)
				{
					for (size_t i = 0; i < layoutPart.Count; i++)
					{
						alignedDataIndex += alignedDataIndex % 16 == 0 ? 0 : 16 - (alignedDataIndex % 16);
						memcpy(&m_BufferMemory[alignedDataIndex], &unalignedData[unalignedDataIndex], size / layoutPart.Count);
						unalignedDataIndex += size / layoutPart.Count;
						alignedDataIndex += size / layoutPart.Count;
					}
				}
				else
				{
					uint32_t baseAlignment = GetBasestd140Alignemnt(layoutPart.Type);

					alignedDataIndex += alignedDataIndex % baseAlignment == 0 ? 0 : baseAlignment - (alignedDataIndex % baseAlignment);
					memcpy(&m_BufferMemory[alignedDataIndex], &unalignedData[unalignedDataIndex], size);
					alignedDataIndex += size;
					unalignedDataIndex += size;
				}
			}

			glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, m_AlignedSize, m_BufferMemory);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}
}