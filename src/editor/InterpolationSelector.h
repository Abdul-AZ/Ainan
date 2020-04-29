#pragma once

#include "math/Interpolation.h"

namespace Ainan {
	
	//convertes InterpolationType enum to string just to make the code cleaner
	constexpr const char* InterpolationTypeToString(InterpolationType e) noexcept
	{
#define X(name) case(name): return #name;
		switch (e)
		{
			X(Fixed)
			X(Linear)
			X(Cubic)
			X(Smoothstep)
			X(Custom)
		}
#undef X
		return "";
	}

	//you should or (||) these together if you want multiple flags
	enum class InterpolationSelectorFlags : int
	{
		None              = 0b0000,
		NoLinearMode      = 0b0001,
		NoCubicMode       = 0b0010,
		NoSmoothstepMode  = 0b0100,
		NoCustomMode      = 0b1000
	};

	void DisplayInterpolationTypeSelector(InterpolationType& type, InterpolationSelectorFlags flags, void* id);
}