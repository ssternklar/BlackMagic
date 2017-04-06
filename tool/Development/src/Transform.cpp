#include "Transform.h"

#include <string>

using namespace DirectX;

TransformData::Handle TransformData::Get()
{
	Handle h = ProxyHandler::Get();

	h->pos = { 0, 0, 0 };
	h->rot = { 0, 0, 0, 1 };
	h->scale = 1;
	XMStoreFloat4x4(&h->matrix, XMMatrixIdentity());

	return h;
}

void TransformData::Revoke(Handle handle)
{
	ProxyHandler::Revoke(handle);
}

void TransformData::UpdateTransforms()
{
	for (size_t i = 0; i < size; i++)
	{
		XMVECTOR r = XMLoadFloat4(&data[i].rot);
		XMVECTOR p = XMLoadFloat3(&data[i].pos);
		XMVECTOR s = XMLoadFloat3(&XMFLOAT3(data[i].scale, data[i].scale, data[i].scale));

		XMMATRIX mat = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
		XMStoreFloat4x4(&data[i].matrix, XMMatrixTranspose(mat));
	}
}

void TransformData::Move(Handle handle, XMFLOAT3 dp)
{
	handle->pos.x += dp.x;
	handle->pos.y += dp.y;
	handle->pos.z += dp.z;
}

void TransformData::Rotate(Handle handle, XMFLOAT4 quaternion)
{
	XMVECTOR current = XMLoadFloat4(&handle->rot);
	XMVECTOR quat = XMLoadFloat4(&quaternion);
	XMStoreFloat4(&handle->rot, XMQuaternionMultiply(current, quat));
}

void TransformData::Rotate(Handle handle, XMFLOAT3 axis, float angle)
{
	XMVECTOR axisVec = XMLoadFloat3(&axis);
	XMVECTOR quat = XMQuaternionRotationAxis(axisVec, angle);
	XMVECTOR current = XMLoadFloat4(&handle->rot);
	XMStoreFloat4(&handle->rot, XMQuaternionMultiply(current, quat));
}

XMFLOAT3 TransformData::GetForward(Handle handle)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&handle->rot)));
	return ret;
}

XMFLOAT3 TransformData::GetUp(Handle handle)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMLoadFloat4(&handle->rot)));
	return ret;
}

XMFLOAT3 TransformData::GetRight(Handle handle)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMLoadFloat4(&handle->rot)));
	return ret;
}
