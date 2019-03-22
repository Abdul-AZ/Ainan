#pragma once

#include <Interpolator.h>

class ScaleCustomizer 
{
public:
	ScaleCustomizer();
	void DisplayGUI();

	Interpolator<float>& GetScaleInterpolator();

private:
	//starting scale
	bool RandomScale = true;
	float definedScale = 2.0f;
	float minScale = 1.0f;
	float maxScale = 3.0f;

	//scale over time
	Interpolator<float> m_Interpolator;
	float endScale = definedScale;

	//random number generator
	std::mt19937 mt;
};