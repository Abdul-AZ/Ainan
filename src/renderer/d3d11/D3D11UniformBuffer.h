#pragma once

#include "renderer/UniformBuffer.h"
#include "renderer/RendererAPI.h"
#include "D3D11VertexBuffer.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;
		
		//TODO support arrays in layout
		class D3D11UniformBuffer : public UniformBuffer
		{
		public:
			D3D11UniformBuffer(const std::string name, uint32_t reg, const VertexLayout& layout, void* data, RendererContext* context);
			virtual ~D3D11UniformBuffer();

			virtual void UpdateData(void* data) override;

			//TODO
			virtual std::string GetName() const override { return ""; };
			virtual uint32_t GetPackedSize() const override { return 0; };
			virtual uint32_t GetAlignedSize() const override { return 0; };

		public:
			ID3D11Buffer* Buffer = nullptr;
			uint32_t BufferSize = 0;
			VertexLayout Layout;
			D3D11RendererContext* Context;

			// Inherited via UniformBuffer
			virtual void UpdateDataUnsafe(void* data) override;
		};
	}
}