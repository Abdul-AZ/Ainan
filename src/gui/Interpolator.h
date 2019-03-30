#pragma once

#include <string>

enum class InterpolatorMode 
{
	Fixed,
	Linear,
	Cubic,
	Smoothstep
};

template<typename T>
class Interpolator 
{
public:
	Interpolator(InterpolatorMode mode, T start, T end) :
		Mode(mode),
		startPoint(start),
		endPoint(end)
	{}

	T Interpolate(const float& t){
		T diffrence = endPoint - startPoint;

		switch (Mode)
		{
		case InterpolatorMode::Fixed:
			return startPoint;

		case InterpolatorMode::Linear:
			return startPoint + diffrence * t;

		case InterpolatorMode::Cubic:
			return startPoint + diffrence * t * t * t;

		case InterpolatorMode::Smoothstep:
			return startPoint + diffrence * Smoothstep(t);
		default:
			return startPoint;
		}
	}

	float Smoothstep(const float& t) {
		if (t <= 0)
			return 0;
		if (t >= 1)
			return 1;
		return 3 * (t * t) - 2 * (t * t * t);
	}

	std::string InterpolateModeStr(InterpolatorMode mode) {
		switch (mode)
		{
		case InterpolatorMode::Fixed:
			return "Fixed";

		case InterpolatorMode::Linear:
			return "Linear";

		case InterpolatorMode::Cubic:
			return "Cubic";

		case InterpolatorMode::Smoothstep:
			return "Smoothstep";

		default:
			return "Fixed";
		}
	}

	InterpolatorMode Mode;

	T startPoint;
	T endPoint;
};