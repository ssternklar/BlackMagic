#include "Spline.h"

void Spline::GetPoint(float t, SplineControlPoint& outPoint)
{
	float modifiedT = t * segmentCount;
	int segment = modifiedT;
	float indivT = modifiedT - segment;
	segments[segment % segmentCount].GetPoint(indivT, outPoint);
}

float Spline::GuessNearestPoint(DirectX::XMFLOAT3& point, float* outDistanceSquared)
{
	float closest = 0;
	float closestDistSquared = FLT_MAX;
	int segID = 0;
	for (int i = 0; i < segmentCount; i++)
	{
		float guessDsq;
		float guess = segments[i].GuessNearestPoint(point, guessDsq);
		if (closestDistSquared > guessDsq)
		{
			closest = guess;
			closestDistSquared = guessDsq;
			segID = i;
		}
	}
	closest = (closest + segID) / segmentCount;
	if (outDistanceSquared)
	{
		*outDistanceSquared = closestDistSquared;
	}
	return closest;
}

void Spline::GuessNearestPoint(DirectX::XMFLOAT3& point, SplineControlPoint& outPoint)
{
	return GetPoint(GuessNearestPoint(point), outPoint);
}