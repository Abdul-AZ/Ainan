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
	};
}