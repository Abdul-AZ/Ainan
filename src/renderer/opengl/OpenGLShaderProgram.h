#include "renderer/ShaderProgram.h"
#include "renderer/UniformBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLShaderProgram : public ShaderProgram
		{
		public:
			OpenGLShaderProgram(const std::string& vertPath, const std::string& fragPath);
			OpenGLShaderProgram() { m_RendererID = 0; }
			static std::shared_ptr<OpenGLShaderProgram> CreateRaw(const std::string& vertSrc, const std::string& fragSrc);
			~OpenGLShaderProgram();

			virtual void BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage) override;
			virtual void BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage) override;
			virtual void BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage) override;

			int GetUniformLocation(const char* name) override;
			virtual int GetRendererID() const override;

		public:
			unsigned int m_RendererID;

			//this is to avoid getting uniform location repeatedly which is not very performant
			std::unordered_map<std::string, int> m_UniformLocationMap;
		};
	}
}