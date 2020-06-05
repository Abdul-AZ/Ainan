#include "renderer/VertexBuffer.h"
#include "renderer/ShaderProgram.h"

namespace Ainan
{
	namespace OpenGL {
		class OpenGLVertexBuffer : public VertexBuffer
		{
		public:
			//size is in bytes
			OpenGLVertexBuffer(void* data, unsigned int size, const VertexLayout& layout, bool dynamic);
			~OpenGLVertexBuffer();

			// Inherited via VertexBuffer
			virtual void UpdateData(const int& offset, const int& size, void* data) override;
			virtual unsigned int GetRendererID() override { return m_RendererID; }
			virtual void Bind() const override;
			virtual void Unbind() const override;

		private:
			uint32_t m_RendererID;
			uint32_t m_VertexArray;
		};
	}
}