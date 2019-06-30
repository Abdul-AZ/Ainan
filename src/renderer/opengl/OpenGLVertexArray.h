#include "renderer/VertexArray.h"

namespace ALZ {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		// Inherited via VertexArray
		virtual unsigned int GetRendererID() override { return m_RendererID; };
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		unsigned int m_RendererID;
	};

}