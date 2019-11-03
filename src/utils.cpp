#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>
#include "utils.h"

using namespace cy;

Vec3f RandomInUnitSphere()
{
	Vec3f result;
	do
	{
		result = 2.0f * Vec3f(
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)),
			(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)))
			- Vec3f(1.0f, 1.0f, 1.0f);
	} while (result.LengthSquared() >= 1.0f);

	return result;
}

Vec2f NonUniformRandomPointInCircle(float radius)
{
	float r = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* radius;
	float theta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* Pi<float>() * 2;

	float x = r * cos(theta);
	float y = r * sin(theta);

	return Vec2f(x, y);
}

Vec2f RandomPointInCircle(float radius)
{
	// generate a random value between 0 to Radius as the value of Cumulative Distribution Function
	float S = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	// S = r2 / R2, choose r based on F
	float r = sqrtf(S) * radius;
	float theta = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))* Pi<float>() * 2.0f;

	float x = r * cos(theta);
	float y = r * sin(theta);

	return Vec2f(x, y);
}