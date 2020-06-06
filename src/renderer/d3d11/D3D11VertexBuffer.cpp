#include <pch.h>

#include "D3D11VertexBuffer.h"

#include "D3D11ShaderProgram.h"

#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

namespace Ainan {
	namespace D3D11 {
		constexpr DXGI_FORMAT GetD3D11FormatFromShaderType(const ShaderVariableType& type)
		{
			switch (type)
			{
			case ShaderVariableType::Int:
				return DXGI_FORMAT_R32_SINT;

			case ShaderVariableType::UnsignedInt:
				return DXGI_FORMAT_R32_UINT;

			case ShaderVariableType::Float:
				return DXGI_FORMAT_R32_FLOAT;

			case ShaderVariableType::Vec2:
				return DXGI_FORMAT_R32G32_FLOAT;

			case ShaderVariableType::Vec3:
			case ShaderVariableType::Mat3:
				return DXGI_FORMAT_R32G32B32_FLOAT;

			case ShaderVariableType::Vec4:
			case ShaderVariableType::Mat4:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;

			default:
				assert(false);
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		D3D11VertexBuffer::D3D11VertexBuffer(void* data, unsigned int size,
			const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
			bool dynamic, RendererContext* context)
		{
			Context = (D3D11RendererContext*)context;
			Stride = 0;

			//create buffer
			{
				D3D11_BUFFER_DESC desc{};
				desc.ByteWidth = size;
				desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
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

			//set layout
			{
				std::vector<D3D11_INPUT_ELEMENT_DESC> desc(layout.size());

				for (size_t i = 0; i < layout.size(); i++)
				{
					desc[i].SemanticName = layout[i].Name.c_str();
					desc[i].SemanticIndex = 0;
					desc[i].Format = GetD3D11FormatFromShaderType(layout[i].Type);
					desc[i].InputSlot = 0;
					desc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
					desc[i].InstanceDataStepRate = 0;
					desc[i].AlignedByteOffset = Stride;
					Stride += layout[i].GetSize();
				}

				auto d3dshader = std::static_pointer_cast<D3D11ShaderProgram>(shaderProgram);

				ASSERT_D3D_CALL(Context->Device->CreateInputLayout(desc.data(), desc.size(), d3dshader->VertexByteCode, d3dshader->VertexByteCodeSize, &Layout));
			}
		}

		D3D11VertexBuffer::~D3D11VertexBuffer()
		{
			Layout->Release();
			Buffer->Release();
		}

		void D3D11VertexBuffer::UpdateData(const int& offset, const int& size, void* data)
		{
			D3D11_MAPPED_SUBRESOURCE resource{};

			ASSERT_D3D_CALL(Context->DeviceContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource));

			memcpy((uint8_t*)resource.pData + offset, data, size);

			Context->DeviceContext->Unmap(Buffer, 0);
		}

		void D3D11VertexBuffer::Bind() const
		{
			uint32_t offset = 0;
			Context->DeviceContext->IASetVertexBuffers(0, 1, &Buffer, &Stride, &offset);
			Context->DeviceContext->IASetInputLayout(Layout);
		}

		void D3D11VertexBuffer::Unbind() const
		{
			Context->DeviceContext->IASetVertexBuffers(0, 0, 0, 0, 0);
			Context->DeviceContext->IASetInputLayout(0);
		}
	}
}
