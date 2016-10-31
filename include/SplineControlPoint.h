#pragma once

#include <DirectXMath.h>

class SplineControlPoint
{
public:
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT3 normal;
	inline void GetClosestPointOnPlane(DirectX::XMFLOAT3& InPoint, DirectX::XMFLOAT3* OutPoint);
	inline bool IsInPlaneBounds(DirectX::XMFLOAT3& InPoint);
	inline bool IsCloseToPlane(DirectX::XMFLOAT3& InPoint, float limit);
	inline void GetControlPoint(DirectX::XMFLOAT3& OutPoint);
};