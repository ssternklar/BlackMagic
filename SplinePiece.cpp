#include "SplinePiece.h"

DirectX::XMFLOAT3 SplinePiece::GetSplinePoint(float t)
{
	using namespace DirectX;
	//asdf
	float oneMinusT = 1 - t;
	auto p0 = XMLoadFloat3(&(startPoint.position));
	XMFLOAT3 cp;
	startPoint.GetControlPoint(cp);
	auto p1 = XMLoadFloat3(&cp);
	endPoint.GetControlPoint(cp);
	auto p2 = XMLoadFloat3(&cp);
	auto p3 = XMLoadFloat3(&(endPoint.position));
	XMFLOAT3 ret;
	auto pv =
		t * t * t * p3 +
		oneMinusT * t * t * p2 +
		oneMinusT * oneMinusT * t * p1 +
		oneMinusT * oneMinusT * oneMinusT * p0;
	XMStoreFloat3(&ret, pv);
	return ret;
}


DirectX::XMFLOAT3 SplinePiece::GetSplineDerivative(float t)
{
	using namespace DirectX;
	//asdf
	float oneMinusT = 1 - t;
	auto p0 = XMLoadFloat3(&(startPoint.position));
	XMFLOAT3 cp;
	startPoint.GetControlPoint(cp);
	auto p1 = XMLoadFloat3(&cp);
	endPoint.GetControlPoint(cp);
	auto p2 = XMLoadFloat3(&cp);
	auto p3 = XMLoadFloat3(&(endPoint.position));
	XMFLOAT3 ret;
	auto pv =
		3 * t * t * (p3 - p2) +
		6 * oneMinusT * t * (p2 - p1) +
		3 * oneMinusT * oneMinusT * (p1 - p0);
	XMStoreFloat3(&ret, pv);
	return ret;
}

void SplinePiece::GetPoint(float t, SplineControlPoint& outPoint)
{
	using namespace DirectX;
	outPoint.position = GetSplinePoint(t);
	XMStoreFloat4(&(outPoint.rotation), XMQuaternionSlerp(XMLoadFloat4(&(startPoint.rotation)), XMLoadFloat4(&(endPoint.rotation))));
	XMStoreFloat3(&(outPoint.scale), XMVector3Dot(XMLoadFloat3(&(startPoint.scale)), XMLoadFloat3(&(endPoint.scale))));
}