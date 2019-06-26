#pragma once

namespace ALZ {

	class VertexBuffer
	{
	public:
		virtual unsigned int GetRendererID() = 0;

		//TODO: set vertex layout method

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

}