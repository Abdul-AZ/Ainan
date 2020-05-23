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
			~D3D11UniformBuffer();

			virtual void UpdateData(void* data) override;
			virtual void Bind(uint32_t index, RenderingStage stageBindTarget) override;

		private:
			ID3D11Buffer* Buffer = nullptr;
			uint32_t BufferSize = 0;
			VertexLayout Layout;
			D3D11RendererContext* Context;
		};
	}
}