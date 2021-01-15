#pragma once

#include "VertexBuffer.h"

namespace Ainan 
{
	class UniformBuffer
	{
	public:
		//This is used by the renderer to interact with the abstracted uniform buffer
		uint32_t Identifier = 0;

		//the data parameter is tightly packed, alignment is done internally
		void UpdateData(void* data, uint32_t packedDataSizeofBuffer);
	};

	struct UniformBufferDataView
	{
		uint64_t Identifier = 0;
		std::string Name = "";
		VertexLayout Layout;
		uint32_t PackedSize = 0;
		uint32_t AlignedSize = 0;
		uint8_t* BufferMemory = nullptr;
		bool Deleted = false;
	};
}