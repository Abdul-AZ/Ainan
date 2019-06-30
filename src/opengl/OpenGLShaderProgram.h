#include "graphics/ShaderProgram.h"

namespace ALZ {

	class OpenGLShaderProgram : public ShaderProgram
	{
	public:
		OpenGLShaderProgram(const std::string& vertPath, const std::string& fragPath);
		~OpenGLShaderProgram();

		void Bind() const override;
		void Unbind() const override;

		void SetUniform1i(const char* name, const int& value)                                  override;
		void SetUniform1f(const char* name, const float& value)                                override;
		void SetUniform1fs(const char* name, float* value, const int& count)                   override;
		void SetUniformVec2(const char* name, const glm::vec2& value)                          override;
		void SetUniformVec2s(const char* name, const glm::vec2* const value, const int& count) override;
		void SetUniformVec3(const char* name, const glm::vec3& value)                          override;
		void SetUniformVec3s(const char* name, const glm::vec3* const value, const int& count) override;
		void SetUniformVec4(const char* name, const glm::vec4& value)                          override;
		void SetUniformVec4s(const char* name, const glm::vec4* const value, const int& count) override;
		void SetUniformMat4(const char* name, const glm::mat4& value)                          override;
		void SetUniformMat4s(const char* name, const glm::mat4* const value, const int& count) override;

		int GetUniformLocation(const char* name);

	private:
		unsigned int m_RendererID;

		//this is to avoid getting uniform location repeatedly which is not very performant
		std::unordered_map<std::string, int> m_UniformLocationMap;
	};

}