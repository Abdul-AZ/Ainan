#pragma once

namespace Ainan {

	class IndexBuffer
	{
	public:
		uint32_t Identifier;

		void UpdateData(int32_t offset, int32_t size, void* data);
		uint32_t GetUsedMemory() const;
		uint32_t GetCount() const;
	};

	struct IndexBufferDataView
	{
		uint64_t Identifier;
		uint32_t Size;
		uint32_t Count;
		bool Deleted = false;
	};
}