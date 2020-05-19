#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/RendererAPI.h"

#include <d3d11.h>

namespace Ainan {
	namespace D3D11 {
		class D3D11RendererContext;

		class D3D11ShaderProgram : public ShaderProgram
		{
		public:
			D3D11ShaderProgram(const std::string& vertPath, const std::string& fragPath, RendererContext* context);
			virtual ~D3D11ShaderProgram();

			virtual void Bind() const override;
			virtual void Unbind() const override;
			virtual void SetUniform1i(const char* name, const int& value) override;
			virtual void SetUniform1f(const char* name, const float& value) override;
			virtual void SetUniform1fs(const char* name, float* value, const int& count) override;
			virtual void SetUniformVec2(const char* name, const glm::vec2& value) override;
			virtual void SetUniformVec2s(const char* name, const glm::vec2* const value, const int& count) override;
			virtual void SetUniformVec3(const char* name, const glm::vec3& value) override;
			virtual void SetUniformVec3s(const char* name, const glm::vec3* const value, const int& count) override;
			virtual void SetUniformVec4(const char* name, const glm::vec4& value) override;
			virtual void SetUniformVec4s(const char* name, const glm::vec4* const value, const int& count) override;
			virtual void SetUniformMat4(const char* name, const glm::mat4& value) override;
			virtual void SetUniformMat4s(const char* name, const glm::mat4* const value, const int& count) override;
			virtual int GetUniformLocation(const char* name) override;
			virtual int GetRendererID() const override;

		public:
			D3D11RendererContext* Context;
			ID3D11VertexShader* VertexShader;
			ID3D11PixelShader* FragmentShader;


			//this is needed for creating vertex buffers
			uint8_t* VertexByteCode = nullptr;
			uint32_t VertexByteCodeSize = 0;
			//there is no reason to keep the fragment shader code so it is immdediately freed after use
		};
	}
}