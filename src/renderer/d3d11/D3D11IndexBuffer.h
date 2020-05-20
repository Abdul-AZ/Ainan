#pragma once

#include "D3D11RendererContext.h"
#include "renderer/IndexBuffer.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11IndexBuffer : public IndexBuffer
		{
		public:
			D3D11IndexBuffer(unsigned int* data, int count, RendererContext* context);
			virtual ~D3D11IndexBuffer();

			// Inherited via IndexBuffer
			virtual unsigned int GetRendererID() override;
			virtual int GetCount() const override;
			virtual void Bind() const override;
			virtual void Unbind() const override;

		public:
			ID3D11Buffer* Buffer;
			D3D11RendererContext* Context;
			uint32_t IndexCount;
		};
	}
}