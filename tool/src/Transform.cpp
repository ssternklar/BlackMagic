#include "Transform.h"

#include <string>

using namespace DirectX;

TransformData* TransformData::ptr = nullptr;

void TransformData::Init(size_t numTransforms)
{
	if (!ptr)
		ptr = new TransformData(numTransforms);
}

void TransformData::ShutDown()
{
	delete ptr;
}

TransformData::TransformData(size_t numTransforms)
{
	this->numTransforms = numTransforms;
	transforms = new Transform[numTransforms];
	matrices = new XMFLOAT4X4[numTransforms];
	metaData = new char[numTransforms + 1];

	memset(metaData, 0, numTransforms + 1);
}

TransformData::~TransformData()
{
	delete[] transforms;
	delete[] matrices;
	delete[] metaData;
}

TransformID TransformData::newTransform()
{
	TransformID ret = nextFreeID;

	if (nextFreeID == numTransforms)
		printf("!!!OUT OF TRANSFORMS!!!\n");
	else
	{
		transforms[ret].pos = { 0, 0, 0 };
		transforms[ret].rot = { 0, 0, 0, 1 };
		transforms[ret].scale = { 1, 1, 1 };
		XMStoreFloat4x4(matrices + ret, XMMatrixIdentity());

		if (ret > highestID)
			highestID = ret;

		metaData[ret] |= 0x1;

		do
		{
			++nextFreeID;
		} while (nextFreeID < numTransforms && metaData[nextFreeID] & 0x1);
	}
	
	return ret;
}

void TransformData::deleteTransform(TransformID id)
{
	if (id >= numTransforms || id > highestID)
		return;

	metaData[id] &= ~0x1;
	
	if (id < nextFreeID)
		nextFreeID = id;
	if (id == highestID)
		--highestID;
}

void TransformData::UpdateTransforms()
{
	for (size_t i = 0; i < highestID + 1; i++)
	{
		if ((metaData[i] & 0x3) == 0x3) // if dirty and used
		{
			XMVECTOR r = XMLoadFloat4(&transforms[i].rot);
			XMVECTOR p = XMLoadFloat3(&transforms[i].pos);
			XMVECTOR s = XMLoadFloat3(&transforms[i].scale);

			XMMATRIX mat = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
			XMStoreFloat4x4(&matrices[i], XMMatrixTranspose(mat));
			metaData[i] &= ~0x2;
		}
	}
}

const DirectX::XMFLOAT4X4* TransformData::GetMatrix(TransformID id)
{
	return matrices + id;
}

void TransformData::Move(TransformID id, XMFLOAT3 dp)
{
	transforms[id].pos.x += dp.x;
	transforms[id].pos.y += dp.y;
	transforms[id].pos.z += dp.z;
	metaData[id] |= 0x2;
}

void TransformData::MoveTo(TransformID id, XMFLOAT3 pos)
{
	transforms[id].pos.x = pos.x;
	transforms[id].pos.y = pos.y;
	transforms[id].pos.z = pos.z;
	metaData[id] |= 0x2;
}

void TransformData::Rotate(TransformID id, XMFLOAT4 quaternion)
{
	XMVECTOR current = XMLoadFloat4(&transforms[id].rot);
	XMVECTOR quat = XMLoadFloat4(&quaternion);
	XMStoreFloat4(&transforms[id].rot, XMQuaternionMultiply(current, quat));
	metaData[id] |= 0x2;
}

void TransformData::Rotate(TransformID id, XMFLOAT3 axis, float angle)
{
	XMVECTOR axisVec = XMLoadFloat3(&axis);
	XMVECTOR quat = XMQuaternionRotationAxis(axisVec, angle);
	XMVECTOR current = XMLoadFloat4(&transforms[id].rot);
	XMStoreFloat4(&transforms[id].rot, XMQuaternionMultiply(current, quat));
	metaData[id] |= 0x2;
}

void TransformData::SetRotation(TransformID id, XMFLOAT4 quaternion)
{
	transforms[id].rot = quaternion;
	metaData[id] |= 0x2;
}

void TransformData::SetScale(TransformID id, XMFLOAT3 scale)
{
	transforms[id].scale = scale;
	metaData[id] |= 0x2;
}

XMFLOAT3 TransformData::GetPosition(TransformID id)
{
	return transforms[id].pos;
}

XMFLOAT3 TransformData::GetScale(TransformID id)
{
	return transforms[id].scale;
}

XMFLOAT4 TransformData::GetRotation(TransformID id)
{
	return transforms[id].rot;
}

XMFLOAT3 TransformData::GetForward(TransformID id)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&transforms[id].rot)));
	return ret;
}

XMFLOAT3 TransformData::GetUp(TransformID id)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMLoadFloat4(&transforms[id].rot)));
	return ret;
}

XMFLOAT3 TransformData::GetRight(TransformID id)
{
	XMFLOAT3 ret;
	XMStoreFloat3(&ret, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMLoadFloat4(&transforms[id].rot)));
	return ret;
}

Transform TransformData::GetTransform(TransformID id)
{
	return transforms[id];
}
