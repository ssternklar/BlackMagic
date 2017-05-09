#pragma once
#include "BMMath.h"
#include "Renderer.h"

namespace BlackMagic
{
	struct SplineControlPoint
	{
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;
		Vector3 tangent;
		Vector3 normal;
		Vector3 GetClosestPointOnPlane(Vector3 InPoint);
		bool IsInPlaneBounds(Vector3 InPoint);
		bool IsCloseToPlane(Vector3 InPoint, float limit);
		Vector3 GetControlPoint();
	};

	struct SplinePiece
	{
		SplineControlPoint start;
		SplineControlPoint end;
		Vector3 GetSplinePoint(float t);
		Vector3 GetSplineDerivative(float t);
		SplineControlPoint GetPoint(float t);
		float GuessNearestPoint(Vector3 point, float& outDistanceSquared);
	};

	class Spline
	{
	public:
		unsigned int segmentCount;
		SplinePiece* segments;
		SplineControlPoint GetPoint(float t);
		float GuessNearestPoint(Vector3 point, float* outDistanceSquared);
		SplineControlPoint GuessNearestPoint(Vector3 point);
	};
}
