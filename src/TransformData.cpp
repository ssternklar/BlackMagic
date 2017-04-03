#include <new>
#include "TransformData.h"
#include "Transform.h"

using namespace DirectX;
using namespace BlackMagic;

TransformData* TransformData::singletonRef = nullptr;

TransformData::TransformData() : matrixAllocator(
	64,
	sizeof(XMFLOAT4X4) * 400,
	sizeof(XMFLOAT4X4),
	(byte*)&(_matrices[0]))
{
	singletonRef = this;
}

TransformData* TransformData::GetSingleton()
{
	return singletonRef;
}

TransformID TransformData::AllocateTransform()
{
	XMFLOAT4X4* alloc = matrixAllocator.allocate<XMFLOAT4X4>();
	TransformID res = alloc - _matrices;
	_matrices[res]._44 = 1;
	if (res > highestAllocated)
	{
		highestAllocated = res;
	}
	/*size_t res;
	if (_availableTransforms.empty())
	{
		static size_t last;
		last = _nextAvailableTransform;
		res = _nextAvailableTransform++;
	}
	else
	{
		res = _availableTransforms.top();
		_availableTransforms.pop();
	}*/
	return res;
}

void TransformData::DeallocateTransform(TransformID id)
{
	memset(&(_matrices[id]), 0, sizeof(XMFLOAT4X4));
	matrixAllocator.deallocate<XMFLOAT4X4>(&(_matrices[id]));
	if (id == highestAllocated)
	{
		//decrease it. Figure out who has the next highest.
		//check the bottom row?
	}
}

void TransformData::UpdateTransforms()
{
	for(size_t i = 0; i < highestAllocated + 1; i++)
	{
		//perhaps don't process invalid transforms?
		auto r = XMLoadFloat4(&_rotations[i]);
		auto p = XMLoadFloat3(&_positions[i]);
		auto s = XMLoadFloat3(&_scales[i]);

		auto mat = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
		XMStoreFloat4x4(&_matrices[i], XMMatrixTranspose(mat));
	}
}

DirectX::XMFLOAT4X4* TransformData::GetMatrix(TransformID id)
{
	return &_matrices[id];
}


