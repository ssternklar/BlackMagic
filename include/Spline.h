#pragma once
#include "SplinePiece.h"

class Spline
{
public:
	int segmentCount;
	SplinePiece* segments;
	void GetPoint(float t, SplineControlPoint& outPoint);
	float GuessNearestPoint(DirectX::XMFLOAT3& point, float* outDistanceSquared = nullptr);
	void GuessNearestPoint(DirectX::XMFLOAT3& point, SplineControlPoint& outPoint);
};