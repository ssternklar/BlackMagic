#pragma once
#include "SplinePiece.h"
#include "GraphicsDevice.h"

class Spline
{
public:
	unsigned int segmentCount;
	SplinePiece* segments;
	void GetPoint(float t, SplineControlPoint& outPoint);
	float GuessNearestPoint(DirectX::XMFLOAT3& point, float* outDistanceSquared = nullptr);
	void GuessNearestPoint(DirectX::XMFLOAT3& point, SplineControlPoint& outPoint);
	void GenerateMesh(GraphicsDevice* device, Mesh* mesh);
};