#include "Spline.h"
using namespace BlackMagic;

Vector3 SplineControlPoint::GetClosestPointOnPlane(Vector3& InPoint)
{
	return InPoint - (BlackMagic::Dot(InPoint - position, normal) * normal);
}

bool SplineControlPoint::IsInPlaneBounds(Vector3 & InPoint)
{
	auto closestPoint = GetClosestPointOnPlane(InPoint);
	auto diff = closestPoint - position;
	return Magnitude(diff) < GetX(scale) / 2;
}

bool BlackMagic::SplineControlPoint::IsCloseToPlane(Vector3 & InPoint, float limit)
{
	return Dot(InPoint - position, normal) < limit;
}

Vector3 BlackMagic::SplineControlPoint::GetControlPoint()
{
	return position + tangent;
}

Vector3 SplinePiece::GetSplinePoint(float t)
{
	if (t > 1) t = 1;
	if (t < 0) t = 0;
	float oneMinusT = 1 - t;
	return
		(oneMinusT * oneMinusT * oneMinusT * start.position) +
		(3 * oneMinusT * oneMinusT * t * start.GetControlPoint()) +
		(3 * oneMinusT * t * t * end.GetControlPoint()) +
		(t * t * t * end.position);
}

Vector3 SplinePiece::GetSplineDerivative(float t)
{
	float oneMinusT = 1 - t;
	auto startControlPoint = start.GetControlPoint();
	auto endControlPoint = end.GetControlPoint();
	return
		(3 * oneMinusT * oneMinusT * (startControlPoint - start.position) +
		6 * oneMinusT * t * (endControlPoint - startControlPoint) +
		3 * t * t * (end.position - endControlPoint));
}

SplineControlPoint SplinePiece::GetPoint(float t)
{
	auto tangent = Normalize(GetSplineDerivative(t));
	auto upRotator = Slerp(start.rotation, end.rotation, t);
	auto normal = Normalize(Rotate(CreateVector3(0, 1, 0), upRotator));
	SplineControlPoint ret;
	ret.position = GetSplinePoint(t);
	ret.rotation = CreateQuaternion(tangent, normal);
	ret.scale = Lerp(start.scale, end.scale, t);
	ret.tangent = tangent;
	ret.normal = ret.rotation * CreateVector3(0, 1, 0);
	return ret;
}

float BlackMagic::SplinePiece::GuessNearestPoint(Vector3& point, float& outDistanceSquared)
{
	const int refinementIterations = 3;
	const float scale = .75f;
	float tVals[] = { 0, .5f, 1 };
	Vector3 initialVectors[] = { start.position, GetSplinePoint(.5f), end.position };
	float distSquared[] = { 0,0,0 };
	for (int i = 0; i < 3; i++)
	{
		auto foundPoint = initialVectors[i];
		float lastMove = 1;
		for (int iter = 0; iter < refinementIterations; iter++)
		{
			auto lastBestTangent = GetSplineDerivative(tVals[i]);
			float tm = Dot(lastBestTangent, point - foundPoint) / SquareMagnitude(lastBestTangent);
			tVals[i] += tm;
			foundPoint = GetSplinePoint(tVals[i]);
		}
		distSquared[i] = Magnitude(foundPoint - point);
	}
	
	if (distSquared[0] <= distSquared[1] && distSquared[0] <= distSquared[2])
	{
		outDistanceSquared = distSquared[0];
		return tVals[0];
	}
	
	if (distSquared[1] <= distSquared[2])
	{
		outDistanceSquared = distSquared[1];
		return tVals[1];
	}
	
	outDistanceSquared = distSquared[2];
	return tVals[2];
}

SplinePoint BlackMagic::Spline::GetPoint(float t)
{
	float modifiedT = t * segmentCount;
	int segment = modifiedT;
	float indivT = modifiedT - segment;
	return segments[segment % segmentCount].GetPoint(indivT);
}

float BlackMagic::Spline::GuessNearestPoint(Vector3 & point, float * outDistanceSquared)
{
	float closest = 0;
	float closestDistSquared = INFINITY;
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

SplinePoint BlackMagic::Spline::GuessNearestPoint(Vector3 & point)
{
	return GetPoint(GuessNearestPoint(point, nullptr));
}