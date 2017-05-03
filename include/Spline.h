#pragma once
#include "BMMath.h"
#include "Renderer.h"

namespace BlackMagic
{
	struct SplinePoint
	{
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;
	};
	
	struct SplineControlPoint : public SplinePoint
	{
		Vector3 tangent;
		Vector3 normal;
		Vector3 GetClosestPointOnPlane(Vector3& InPoint);
		bool IsInPlaneBounds(Vector3& InPoint);
		bool IsCloseToPlane(Vector3& InPoint, float limit);
		Vector3 GetControlPoint();
	};

	struct SplinePiece
	{
		SplineControlPoint start;
		SplineControlPoint end;
		Vector3 GetSplinePoint(float t);
		Vector3 GetSplineDerivative(float t);
		SplineControlPoint GetPoint(float t);
		float GuessNearestPoint(Vector3& point, float& outDistanceSquared);
	};

	class Spline
	{
	public:
		unsigned int segmentCount;
		SplinePiece* segments;
		SplinePoint GetPoint(float t);
		float GuessNearestPoint(Vector3& point, float* outDistanceSquared = nullptr);
		SplinePoint GuessNearestPoint(Vector3& point);
	};
}
