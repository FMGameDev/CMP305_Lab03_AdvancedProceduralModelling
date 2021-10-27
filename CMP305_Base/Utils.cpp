#include "Utils.h"

#include <cstdlib>
#include <ctime>


float Utils::GetRandom(Range range)
{
	return Utils::GetRandom(range.min, range.max);
}

float Utils::GetRandom(float from, float to)
{
	int min, max;
	float randomNum;

	if (from < to)
	{
		min = from;
		max = to;
	}
	else
	{
		min = to;
		max = from;
	}
	randomNum = float(std::rand() % (max + 1 - min) + min);// (rand() % (max + 1 - min) + min);//(min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min))));
	return randomNum;
}