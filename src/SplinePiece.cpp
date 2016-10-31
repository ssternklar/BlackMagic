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
	auto quat = XMQuaternionSlerp(XMLoadFloat4(&(startPoint.rotation)), XMLoadFloat4(&(endPoint.rotation)), t);
	XMStoreFloat4(&(outPoint.rotation), quat);
	XMStoreFloat3(&(outPoint.scale), XMVectorLerp(XMLoadFloat3(&(startPoint.scale)), XMLoadFloat3(&(endPoint.scale)), t));
	outPoint.tangent = GetSplinePoint(t);
	XMStoreFloat3(&(outPoint.normal), quat * XMVectorSet(0, 1, 0, 0));
}

float SplinePiece::GuessNearestPoint(DirectX::XMFLOAT3& point, float& outDistanceSquared)
{
	using namespace DirectX;
	auto pointVec = XMLoadFloat3(&point);
	const int refinementIterations = 3;
	const float scale = .75f;
	float tVals[] = { 0, .5f, 1 };
	SplineControlPoint tmp;
	GetPoint(.5f, tmp);
	XMVECTOR initialVectors[] = { XMLoadFloat3(&(startPoint.position)), XMLoadFloat3(&(tmp.position)), XMLoadFloat3(&(endPoint.position)) };
	float distSquared[] = { 0, 0, 0 };
	for (int i = 0; i < 3; i++)
	{
		auto foundPoint = initialVectors[i];
		float lastMove = 1;
		for (int iter = 0; iter < refinementIterations; iter++)
		{
			XMVECTOR lastBestTangent = XMLoadFloat3(&(SplinePiece::GetSplineDerivative(tVals[i])));
			XMVECTOR delta = pointVec - foundPoint;
			XMVECTOR move = XMVector3Dot(lastBestTangent, delta) / XMVector3LengthSq(lastBestTangent);
			float tm;
			XMStoreFloat(&tm, move);
			tVals[i] += tm;
			foundPoint = XMLoadFloat3(&(SplinePiece::GetSplinePoint(tVals[i])));
		}
		XMStoreFloat(&(distSquared[i]), XMVector3LengthSq(foundPoint - pointVec));
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