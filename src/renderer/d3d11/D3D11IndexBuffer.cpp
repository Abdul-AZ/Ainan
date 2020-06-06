#pragma once

#include <pch.h>

#include "D3D11IndexBuffer.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

namespace Ainan {
	namespace D3D11 {
		D3D11IndexBuffer::D3D11IndexBuffer(unsigned int* data, int count, RendererContext* context)
		{
			Context = (D3D11RendererContext*)context;
			IndexCount = count;

			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = count * sizeof(uint32_t);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
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

		D3D11IndexBuffer::~D3D11IndexBuffer()
		{
			Buffer->Release();
		}

		void D3D11IndexBuffer::Bind() const
		{
			Context->DeviceContext->IASetIndexBuffer(Buffer, DXGI_FORMAT_R32_UINT, 0);
		}

		void D3D11IndexBuffer::Unbind() const
		{
			Context->DeviceContext->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		}
	}
}