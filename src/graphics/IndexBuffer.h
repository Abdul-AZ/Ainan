#pragma once

namespace ALZ {

	class IndexBuffer 
	{
	public:
		virtual unsigned int GetRendererID() = 0;
		virtual int GetCount() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}