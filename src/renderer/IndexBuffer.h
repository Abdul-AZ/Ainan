#pragma once

namespace Ainan {

	class IndexBuffer 
	{
	public:
		virtual ~IndexBuffer() {};

		virtual uint32_t GetUsedMemory() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};


	class IndexBufferNew
	{
	public:
		uint32_t Identifier;

		void UpdateData(int32_t offset, int32_t size, void* data);
		uint32_t GetUsedMemory() const;
		uint32_t GetCount() const;

		friend class Renderer;
	};

	struct IndexBufferDataView
	{
		uint64_t Identifier;
		uint32_t Size;
		uint32_t Count;
	};
}