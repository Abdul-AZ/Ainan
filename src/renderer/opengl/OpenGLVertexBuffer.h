#include "renderer/VertexBuffer.h"

namespace Ainan
{
	namespace OpenGL {
		class OpenGLVertexBuffer : public VertexBuffer
		{
		public:
			//size is in bytes
			OpenGLVertexBuffer(void* data, unsigned int size);
			~OpenGLVertexBuffer();

			// Inherited via VertexBuffer
			virtual void SetLayout(const VertexLayout& layout) override;
			virtual void UpdateData(const int& offset, const int& size, void* data) override;
			virtual unsigned int GetRendererID() override { return m_RendererID; }
			virtual void Bind() const override;
			virtual void Unbind() const override;

		private:
			unsigned int m_RendererID;
		};
	}
}