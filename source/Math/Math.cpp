#include "Math.h"
#include <cmath>

float DegreesToRadian(float degreeVal)
{
	return degreeVal * (PI / 180.f);
}

double RadiansToDegrees(const float& radVal)
{
	return radVal * (180.f / PI);
}

double Acos(double value)
{
	return acos((value < -1.0) ? -1.0 : ((value < 1.0) ? value : 1.0));
}
