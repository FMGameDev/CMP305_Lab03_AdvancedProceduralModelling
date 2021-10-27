#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

struct Range
{
	Range()
	{
		min = 0.0f;
		max = 10.0f;
	}

	float min;
	float max;
};

class Utils
{
public:
	static float GetRandom(Range range);
	static float GetRandom(float from, float to);
};