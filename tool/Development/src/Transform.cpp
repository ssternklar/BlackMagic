#include "Transform.h"

#include <string>

using namespace DirectX;

TransformData::TransformData()
{
	numTransforms = 16;
	transformCount = 0;

	transforms = new Transform[numTransforms];
}

TransformData::~TransformData()
{
	delete[] transforms;
}

TransformHandle TransformData::newTransform()
{
	if (transformCount == numTransforms)
	{
		numTransforms += 16;
		Transform* newTransforms = new Transform[numTransforms];

		memcpy_s(newTransforms, numTransforms, transforms, numTransforms - 16);
		proxy.move(transforms, newTransforms, numTransforms - 16);

		delete[] transforms;
		transforms = newTransforms;
	}

	transforms[transformCount].pos = { 0, 0, 0 };
	transforms[transformCount].rot = { 0, 0, 0, 1 };
	transforms[transformCount].scale = { 1, 1, 1 };
	XMStoreFloat4x4(&transforms[transformCount].matrix, XMMatrixIdentity());

	return proxy.track(&transforms[transformCount++]);
}

void TransformData::deleteTransform(TransformHandle handle)
{
	if (!transformCount)
		return;

	size_t index = handle.ptr() - transforms;

	proxy.relinquish(handle.ptr());

	if (index != --transformCount)
	{
		proxy.move(transforms + transformCount, transforms + index);
		transforms[index] = transforms[transformCount];
	}
}

void TransformData::UpdateTransforms()
{
	for (size_t i = 0; i < transformCount; i++)
	{
		XMVECTOR r = XMLoadFloat4(&transforms[i].rot);
		XMVECTOR p = XMLoadFloat3(&transforms[i].pos);
		XMVECTOR s = XMLoadFloat3(&transforms[i].scale);

		XMMATRIX mat = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
		XMStoreFloat4x4(&transforms[i].matrix, XMMatrixTranspose(mat));
	}
}

void TransformData::Move(TransformHandle handle, XMFLOAT3 dp)
{
	handle->pos.x += dp.x;
	handle->pos.y += dp.y;
	handle->pos.z += dp.z;
}

void TransformData::Rotate(TransformHandle handle, XMFLOAT4 quaternion)
{
	XMVECTOR current = XMLoadFloat4(&handle->rot);
	XMVECTOR quat = XMLoadFloat4(&quaternion);
	XMStoreFloat4(&handle->rot, XMQuaternionMultiply(current, quat));
}

void TransformData::Rotate(TransformHandle handle, XMFLOAT3 axis, float angle)
{
	XMVECTOR axisVec = XMLoadFloat3(&axis);
	XMVECTOR quat = XMQuaternionRotationAxis(axisVec, angle);
	XMVECTOR current = XMLoadFloat4(&handle->rot);
	XMStoreFloat4(&handle->rot, XMQuaternionMultiply(current, quat));
}

XMFLOAT3 TransformData::GetForward(TransformHandle handle)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&handle->rot)));
	return ret;
}

XMFLOAT3 TransformData::GetUp(TransformHandle handle)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMLoadFloat4(&handle->rot)));
	return ret;
}

XMFLOAT3 TransformData::GetRight(TransformHandle handle)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMLoadFloat4(&handle->rot)));
	return ret;
}
