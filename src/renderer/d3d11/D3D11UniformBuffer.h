#pragma once

#include "renderer/UniformBuffer.h"
#include "renderer/RendererAPI.h"
#include "D3D11VertexBuffer.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;
		
		class D3D11UniformBuffer : public UniformBuffer
		{
		public:
			D3D11UniformBuffer(const std::string name, uint32_t reg, const VertexLayout& layout, void* data, RendererContext* context);
			virtual ~D3D11UniformBuffer();

			virtual void UpdateData(void* data) override;
			virtual void UpdateDataUnsafe(void* data) override;

			virtual std::string GetName() const override { return Name; };
			virtual uint32_t GetPackedSize() const override { return PackedSize; };
			virtual uint32_t GetAlignedSize() const override { return AlignedSize; };

		public:
			ID3D11Buffer* Buffer = nullptr;
			uint32_t BufferSize = 0;
			VertexLayout Layout;
			D3D11RendererContext* Context;
			uint32_t PackedSize = 0;
			uint32_t AlignedSize = 0;
			std::string Name;
		};
	}
}