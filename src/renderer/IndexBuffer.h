#pragma once

namespace Ainan {

	class IndexBuffer 
	{
	public:
		virtual ~IndexBuffer() {};

		virtual uint32_t GetCount() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}