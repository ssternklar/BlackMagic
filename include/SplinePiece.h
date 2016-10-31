#pragma once

#include "SplineControlPoint.h"

class SplinePiece
{
private:

	DirectX::XMFLOAT3 GetSplinePoint(float t);
	DirectX::XMFLOAT3 GetSplineDerivative(float t);

	SplineControlPoint startPoint;
	SplineControlPoint endPoint;
public:
	void GetPoint(float t, SplineControlPoint& outPoint);
	float GuessNearestPoint(DirectX::XMFLOAT3& point, float& outDistanceSquared);
};