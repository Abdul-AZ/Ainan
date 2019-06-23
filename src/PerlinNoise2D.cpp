#include <pch.h>

#include "PerlinNoise2D.h"

namespace ALZ {

	void PerlinNoise2D::Init(int maxSizeX, int maxSizeY)
	{
		//initilize vector gradient
		m_GradientSize.x = maxSizeX;
		m_GradientSize.y = maxSizeY;
		m_VectorGradient = new glm::vec2*[maxSizeX];
		for (int i = 0; i < m_GradientSize.x; i++)
		{
			m_VectorGradient[i] = new glm::vec2[maxSizeX];
		}

		std::mt19937 randomEngine;
		std::uniform_real_distribution<float> range(-1.0f, 1.0f);

		for (int x = 0; x < m_GradientSize.x; x++)
		{
			for (int y = 0; y < m_GradientSize.y; y++)
			{
				m_VectorGradient[x][y].x = range(randomEngine);
				m_VectorGradient[x][y].y = sqrt(1 - pow(m_VectorGradient[x][y].x, 2));
			}
		}

		m_Selector.Type = InterpolationType::Smoothstep;
	}

	PerlinNoise2D::~PerlinNoise2D()
	{
		if (m_VectorGradient)
		{
			for (int i = 0; i < m_GradientSize.x; i++)
			{
				delete[] m_VectorGradient[i];
			}
			delete[] m_VectorGradient;
		}
	}

	float PerlinNoise2D::Noise(float x, float y)
	{
		x = abs(x);
		y = abs(y);

		int x0 = (int)x;
		int x1 = x0 + 1;
		int y0 = (int)y;
		int y1 = y0 + 1;

		float sx = x - (float)x0;
		float sy = y - (float)y0;

		float n0, n1, ix0, ix1, value;

		n0 = DotGridGradient(x0, y0, x, y);
		n1 = DotGridGradient(x1, y0, x, y);

		m_Selector.startPoint = n0;
		m_Selector.endPoint = n0;
		ix0 = m_Selector.Interpolate(sx);

		n0 = DotGridGradient(x0, y1, x, y);
		n1 = DotGridGradient(x1, y1, x, y);

		m_Selector.startPoint = n0;
		m_Selector.endPoint = n1;
		ix1 = m_Selector.Interpolate(sx);

		m_Selector.startPoint = ix0;
		m_Selector.endPoint = ix1;
		value = m_Selector.Interpolate(sy);

		return value;
	}

	float PerlinNoise2D::DotGridGradient(int ix, int iy, float x, float y)
	{
		int ix2 = (int)abs(ix) % m_GradientSize.x;
		int iy2 = (int)abs(iy) % m_GradientSize.y;

		float dx = x - ix;
		float dy = y - iy;


		return dx * m_VectorGradient[ix2][iy2].x + dy * m_VectorGradient[ix2][iy2].y;
	}
}