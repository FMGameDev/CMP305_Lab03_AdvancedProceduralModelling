#include "Utils.h"

#include <cstdlib>
#include <ctime>


int Utils::GetRandom(int from, int to)
{
	int min, max;

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

	return std::rand() % (max + 1 - min) + min;// (rand() % (max + 1 - min) + min);//(min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min))));
}