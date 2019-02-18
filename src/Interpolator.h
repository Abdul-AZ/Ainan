#pragma once

#include <string>

enum class InterpolatorMode 
{
	Fixed,
	Linear
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

	T Interpolate(const float& t)
	{
		if (Mode == InterpolatorMode::Linear) {
			T diffrence = endPoint - startPoint;
			return startPoint + diffrence * t;
		}
		else {
			return startPoint;
		}
	}

	std::string InterpolateModeStr(InterpolatorMode mode) {
		switch (mode)
		{
		case InterpolatorMode::Fixed:
			return "Fixed";
			break;
		case InterpolatorMode::Linear:
			return "Linear";
			break;
		default:
			return "Fixed";
			break;
		}
	}

	InterpolatorMode Mode;

private:
	T startPoint;
	T endPoint;
};