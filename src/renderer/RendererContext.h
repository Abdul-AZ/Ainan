#pragma once

namespace Ainan {

	enum class RendererType
	{
#ifdef PLATFORM_WINDOWS
		D3D11,
#endif
		OpenGL
	};
	
	//this holds context data for each API
	//to access the data you must include one the inheriting classes
	//for example include D3D11RendererContext and cast it to that class
	class RendererContext
	{
	public:
		virtual RendererType GetType() const = 0;
	};
}