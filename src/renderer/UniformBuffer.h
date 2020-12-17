#pragma once

#include "RendererAPI.h"

namespace Ainan 
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {};

		//the data parameter is tightly packed, alignment is done internally
		//the order of uniforms is as specified in the layout on uniform creation
		//size of data is the size of the uniform buffer passed on creation
		virtual void UpdateData(void* data) = 0;

		virtual std::string GetName() const = 0;
		virtual uint32_t GetPackedSize() const = 0;
		virtual uint32_t GetAlignedSize() const = 0;

	private:
		virtual void UpdateDataUnsafe(void* data) = 0;

		friend class Renderer;
	};

	struct UniformBufferDataView
	{
		uint32_t Identifier = 0;
		std::string Name = "";
		VertexLayout Layout;
		uint32_t PackedSize = 0;
		uint32_t AlignedSize = 0;
		uint8_t* BufferMemory = nullptr;
	};

	class UniformBufferNew
	{
	public:
		//This is used by the renderer to interact with the abstracted uniform buffer
		uint32_t Identifier = 0;

		//the data parameter is tightly packed, alignment is done internally
		void UpdateData(void* data, uint32_t packedDataSizeofBuffer);

		friend class Renderer;
	};
}