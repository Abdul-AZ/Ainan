#pragma once

#include <pch.h>
#include "D3D11ShaderProgram.h"

#include "D3D11RendererContext.h"
#include "D3D11UniformBuffer.h"
#include "D3D11Texture.h"
#include "D3D11FrameBuffer.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

#include "renderer/Renderer.h"

namespace Ainan {
	namespace D3D11 {
		D3D11ShaderProgram::D3D11ShaderProgram(const std::string& vertPath, const std::string& fragPath, RendererContext* context)
		{
			assert(context->GetType() == RendererType::D3D11);
			Context = (D3D11RendererContext*)context;

			//load batch renderer shader
			uint8_t* fragmentByteCode = nullptr;
			uint32_t fragmentByteCodeSize = 0;
			{
				FILE* file = nullptr;
				auto err = fopen_s(&file, (vertPath + "-v.cso").c_str(), "rb");
				if (err != 0)
					assert(false);

				fseek(file, 0, SEEK_END);
				VertexByteCodeSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				VertexByteCode = new uint8_t[VertexByteCodeSize];
				fread(VertexByteCode, VertexByteCodeSize, 1, file);
				fclose(file);
			}
			{
				FILE* file = nullptr;
				auto err = fopen_s(&file, (fragPath + "-f.cso").c_str(), "rb");
				if (err != 0)
					assert(false);

				fseek(file, 0, SEEK_END);
				fragmentByteCodeSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				fragmentByteCode = new uint8_t[fragmentByteCodeSize];
				fread(fragmentByteCode, fragmentByteCodeSize, 1, file);
				fclose(file);
			}

			ASSERT_D3D_CALL(Context->Device->CreateVertexShader(VertexByteCode, VertexByteCodeSize, 0, &VertexShader));
			ASSERT_D3D_CALL(Context->Device->CreatePixelShader(fragmentByteCode, fragmentByteCodeSize, 0, &FragmentShader));

			delete[] fragmentByteCode;
		}

		D3D11ShaderProgram::~D3D11ShaderProgram()
		{
			delete[] VertexByteCode;
			VertexShader->Release();
			FragmentShader->Release();
		}

		void D3D11ShaderProgram::BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage)
		{
			auto func = [this, &buffer, slot, stage]()
			{
				BindUniformBufferUnsafe(buffer, slot, stage);
			};
			Renderer::PushCommand(func);
		}

		void D3D11ShaderProgram::BindUniformBufferUnsafe(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage)
		{
			std::shared_ptr<D3D11UniformBuffer> d3dBuffer = std::static_pointer_cast<D3D11UniformBuffer>(buffer);

			switch (stage)
			{
			case Ainan::RenderingStage::VertexShader:
				Context->DeviceContext->VSSetConstantBuffers(slot, 1, &d3dBuffer->Buffer);
				break;

			case Ainan::RenderingStage::FragmentShader:
				Context->DeviceContext->PSSetConstantBuffers(slot, 1, &d3dBuffer->Buffer);
				break;
			}
		}

		void D3D11ShaderProgram::BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage)
		{
			auto d3dTexture = std::static_pointer_cast<D3D11Texture>(texture);

			switch (stage)
			{
			case RenderingStage::VertexShader:
				Context->DeviceContext->VSSetShaderResources(slot, 1, &d3dTexture->D3DResourceView);
				Context->DeviceContext->VSSetSamplers(slot, 1, &d3dTexture->D3DSampler);
				break;

			case RenderingStage::FragmentShader:
				Context->DeviceContext->PSSetShaderResources(slot, 1, &d3dTexture->D3DResourceView);
				Context->DeviceContext->PSSetSamplers(slot, 1, &d3dTexture->D3DSampler);
				break;
			}

			
		}

		void D3D11ShaderProgram::BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage)
		{
			auto d3dframebuffer = std::static_pointer_cast<D3D11FrameBuffer>(framebuffer);

			switch (stage)
			{
			case RenderingStage::VertexShader:
				Context->DeviceContext->VSSetShaderResources(slot, 1, &d3dframebuffer->RenderTargetTextureView);
				Context->DeviceContext->VSSetSamplers(slot, 1, &d3dframebuffer->RenderTargetTextureSampler);
				break;

			case RenderingStage::FragmentShader:
				Context->DeviceContext->PSSetShaderResources(slot, 1, &d3dframebuffer->RenderTargetTextureView);
				Context->DeviceContext->PSSetSamplers(slot, 1, &d3dframebuffer->RenderTargetTextureSampler);
				break;
			}
		}
	}
}