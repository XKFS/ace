#include "tween_math.h"

namespace tweeny
{
float square(float x, int n)
{
	for(int i = 0; i < n; i++)
	{
		x *= x;
	}
	return x;
}

float flip(float x)
{
	return 1.0f - x;
}

float mix(float a, float b, float weight, float t)
{
	const float mix = ((1.0f - weight) * a) + ((1.0f - weight) * b);
	return mix * t;
}

float crossfade(float a, float b, float t)
{
	return ((1.0f - t) * a) + (t * b);
}

float scale(float a, float t)
{
	return a * t;
}

float reverse_scale(float a, float t)
{
    return a * (1.0f - t);
}

float arch(float t)
{
	return t * (1.0f - t);
}

} //end of namespace tweeny
