#include "SplineControlPoint.h"

void SplineControlPoint::GetClosestPointOnPlane(DirectX::XMFLOAT3& InPoint, DirectX::XMFLOAT3* OutPoint)
{
	using namespace DirectX;
	auto iop = XMLoadFloat3(&InPoint);
	auto tp = XMLoadFloat3(&position);
	auto n = XMLoadFloat3(&normal);
	XMStoreFloat3(OutPoint, tp - (XMVector3Dot(iop - tp, n) * n));
}

bool SplineControlPoint::IsInPlaneBounds(DirectX::XMFLOAT3& InPoint)
{
	using namespace DirectX;
	XMFLOAT3 closestPoint;
	GetClosestPointOnPlane(InPoint, &closestPoint);
	float sc = powf((scale.x / 2), 2);
	auto diff = (XMLoadFloat3(&closestPoint) - XMLoadFloat3(&position));
	return XMVector3Dot(diff, diff).m128_f32[0] < sc;
}

bool SplineControlPoint::IsCloseToPlane(DirectX::XMFLOAT3& InPoint, float limit)
{
	using namespace DirectX;
	auto dot = XMVectorAbs(XMVector3Dot(XMLoadFloat3(&InPoint) - XMLoadFloat3(&position), XMLoadFloat3(&normal)));
	
	return XMVectorGetX(dot) < limit;
}

void SplineControlPoint::GetControlPoint(DirectX::XMFLOAT3& OutPoint)
{
	using namespace DirectX;
	auto cp = XMLoadFloat3(&position) + (XMLoadFloat3(&tangent));
	XMStoreFloat3(&OutPoint, cp);
}