#pragma once

namespace ALZ {

	//IMPORTANT: this should only be created using the CreateVertexArray function inside the Renderer
	//This is an interface that each graphics api should implement
	class VertexArray 
	{
	public:
		virtual unsigned int GetRendererID() = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};

}