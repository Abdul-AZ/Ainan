#pragma once

#include <pch.h>
#include "D3D11ShaderProgram.h"

#include "D3D11RendererContext.h"
#define ASSERT_D3D_CALL(func) { auto result = func; if (result != S_OK) assert(false); }

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
				auto err = fopen_s(&file, (vertPath + ".cso").c_str(), "rb");
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
				auto err = fopen_s(&file, (fragPath + ".cso").c_str(), "rb");
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

		void D3D11ShaderProgram::Bind() const
		{
			Context->DeviceContext->VSSetShader(VertexShader, 0, 0);
			Context->DeviceContext->PSSetShader(FragmentShader, 0, 0);
		}

		void D3D11ShaderProgram::Unbind() const
		{
			Context->DeviceContext->VSSetShader(0, 0, 0);
			Context->DeviceContext->PSSetShader(0, 0, 0);
		}

		//TODO
		void D3D11ShaderProgram::SetUniform1i(const char* name, const int& value)
		{
		}
		void D3D11ShaderProgram::SetUniform1f(const char* name, const float& value)
		{
		}
		void D3D11ShaderProgram::SetUniform1fs(const char* name, float* value, const int& count)
		{
		}
		void D3D11ShaderProgram::SetUniformVec2(const char* name, const glm::vec2& value)
		{
		}
		void D3D11ShaderProgram::SetUniformVec2s(const char* name, const glm::vec2* const value, const int& count)
		{
		}
		void D3D11ShaderProgram::SetUniformVec3(const char* name, const glm::vec3& value)
		{
		}
		void D3D11ShaderProgram::SetUniformVec3s(const char* name, const glm::vec3* const value, const int& count)
		{
		}
		void D3D11ShaderProgram::SetUniformVec4(const char* name, const glm::vec4& value)
		{
		}
		void D3D11ShaderProgram::SetUniformVec4s(const char* name, const glm::vec4* const value, const int& count)
		{
		}
		void D3D11ShaderProgram::SetUniformMat4(const char* name, const glm::mat4& value)
		{
		}
		void D3D11ShaderProgram::SetUniformMat4s(const char* name, const glm::mat4* const value, const int& count)
		{
		}
		int D3D11ShaderProgram::GetUniformLocation(const char* name)
		{
			return 0;
		}
		int D3D11ShaderProgram::GetRendererID() const
		{
			return 0;
		}
		void D3D11ShaderProgram::BindUniformBuffer(const char* name, uint32_t slot)
		{
		}
	}
}