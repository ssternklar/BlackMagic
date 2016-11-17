#include "SplineControlPoint.h"

void SplineControlPoint::GetClosestPointOnPlane(DirectX::XMFLOAT3& InPoint, DirectX::XMFLOAT3* OutPoint)
{
	using namespace DirectX;
	auto inPointV = XMLoadFloat3(&InPoint);
	auto splinePointPosition = XMLoadFloat3(&position);
	auto splineNormal = XMLoadFloat3(&normal);
	XMStoreFloat3(OutPoint, inPointV - (XMVector3Dot(inPointV - splinePointPosition, splineNormal) * splineNormal));
}

bool SplineControlPoint::IsInPlaneBounds(DirectX::XMFLOAT3& InPoint)
{
	using namespace DirectX;
	XMFLOAT3 closestPoint;
	GetClosestPointOnPlane(InPoint, &closestPoint);
	auto diff = (XMLoadFloat3(&closestPoint) - XMLoadFloat3(&position));
	return XMVectorGetX(XMVector3Length(diff)) < scale.x / 2;
}

bool SplineControlPoint::IsCloseToPlane(DirectX::XMFLOAT3& InPoint, float limit)
{
	using namespace DirectX;
	auto dot = XMVector3Dot(XMLoadFloat3(&InPoint) - XMLoadFloat3(&position), XMLoadFloat3(&normal));
	
	return XMVectorGetX(XMVectorAbs(dot)) < limit;
}

void SplineControlPoint::GetControlPoint(DirectX::XMFLOAT3& OutPoint)
{
	using namespace DirectX;
	auto cp = XMLoadFloat3(&position) + (XMLoadFloat3(&tangent));
	XMStoreFloat3(&OutPoint, cp);
}