#pragma once

namespace Ainan 
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {};

		virtual void UpdateData(void* data) = 0;
		virtual void Bind(uint32_t index) = 0;
	};
}