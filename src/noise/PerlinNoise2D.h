#pragma once

#include "glm/glm.hpp"

class PerlinNoise2D 
{
public:
	//Not threaded
	void Init(int maxSizeX = 100, int maxSizeY = 100);
	PerlinNoise2D() {};
	~PerlinNoise2D();

	PerlinNoise2D(const PerlinNoise2D& noise) = delete;
	PerlinNoise2D operator=(const PerlinNoise2D& noise) = delete;


	float Noise(float x, float y);

private:
	float DotGridGradient(int ix, int iy, float x, float y);

	//TODO make this customizable
	float lerp(float a0, float a1, float w) {
		return (1.0f - w)*a0 + w * a1;
	};

private:
	glm::ivec2 m_GradientSize = { 0,0 };
	glm::vec2** m_VectorGradient = nullptr; //2D array
};