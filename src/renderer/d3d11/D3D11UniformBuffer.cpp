#include <pch.h>
#include "D3D11UniformBuffer.h"

#include "D3D11RendererContext.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

namespace Ainan {
	namespace D3D11 {

		D3D11UniformBuffer::D3D11UniformBuffer(const std::string name, uint32_t reg, const VertexLayout& layout, void* data, RendererContext* context)
		{
			Context = (D3D11RendererContext*)context;
			Layout = layout;

			//calculate buffer size with alignment
			{
				for (auto& layoutPart : layout)
				{
					
					if (BufferSize % sizeof(glm::vec4) != 0)
					{
						BufferSize += sizeof(glm::vec4) - BufferSize % sizeof(glm::vec4);
					}
					BufferSize += layoutPart.GetSize();
				}
			}

			//create buffer
			{
				D3D11_BUFFER_DESC desc{};
				desc.ByteWidth = BufferSize;
				desc.Usage = D3D11_USAGE_DYNAMIC;
				desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				if (data)
				{
					D3D11_SUBRESOURCE_DATA initialData{};
					initialData.pSysMem = data;

					ASSERT_D3D_CALL(Context->Device->CreateBuffer(&desc, &initialData, &Buffer));
				}
				else
				{
					ASSERT_D3D_CALL(Context->Device->CreateBuffer(&desc, 0, &Buffer));
				}
			}
		}

		D3D11UniformBuffer::~D3D11UniformBuffer()
		{
			Buffer->Release();
		}

		void D3D11UniformBuffer::UpdateData(void* data)
		{
			D3D11_MAPPED_SUBRESOURCE subresource{};
			ASSERT_D3D_CALL(Context->DeviceContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource));

			uint32_t alignedDataIndex = 0;
			uint32_t unalignedDataIndex = 0;
			uint8_t* unalignedData = (uint8_t*)data;
			uint8_t* alignedData = (uint8_t*)subresource.pData;
			for (auto& layoutPart : Layout)
			{
				uint32_t size = layoutPart.GetSize();
				if (alignedDataIndex % sizeof(glm::vec4) != 0)
					alignedDataIndex += sizeof(glm::vec4) - alignedDataIndex % sizeof(glm::vec4);

				memcpy(&alignedData[alignedDataIndex], &unalignedData[unalignedDataIndex], size);
				alignedDataIndex += size;
				unalignedDataIndex += size;
			}

			Context->DeviceContext->Unmap(Buffer, 0);
		}

		void D3D11UniformBuffer::Bind(uint32_t index, RenderingStage stageBindTarget)
		{
			if (stageBindTarget == RenderingStage::VertexShader)
				Context->DeviceContext->VSSetConstantBuffers(index, 1, &Buffer);
			else if (stageBindTarget == RenderingStage::FragmentShader)
				Context->DeviceContext->PSSetConstantBuffers(index, 1, &Buffer);
			else
				assert(false);
		}
	}
}