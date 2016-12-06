#include <new>
#include "TransformData.h"
#include "Transform.h"

using namespace DirectX;

//5 pointers + an ID
size_t TransformData::Size = sizeof(XMFLOAT4X4*) + sizeof(XMFLOAT4*) + 2*sizeof(XMFLOAT3*) + sizeof(bool*) + sizeof(TransformID);
XMFLOAT4X4 TransformData::_matrices[500];
XMFLOAT4 TransformData::_rotations[500];
XMFLOAT3 TransformData::_positions[500];
XMFLOAT3 TransformData::_scales[500];
//std::stack<TransformID> TransformData::_availableTransforms;
TransformID TransformData::_nextAvailableTransform = 0;


void TransformData::Init(size_t allocCount, void* mem)
{
}

TransformID TransformData::AllocateTransform()
{
	size_t res;
	//if (_availableTransforms.empty())
	{
		static size_t last;
		last = _nextAvailableTransform;
		res = _nextAvailableTransform++;
	}
	/*else
	{
		res = _availableTransforms.top();
		_availableTransforms.pop();
	}*/
	return res;
}

void TransformData::DeallocateTransform(TransformID id)
{
}

void TransformData::UpdateTransforms()
{
	for(size_t i = 0; i < _nextAvailableTransform; i++)
	{
		auto r = XMLoadFloat4(&_rotations[i]);
		auto p = XMLoadFloat3(&_positions[i]);
		auto s = XMLoadFloat3(&_scales[i]);

		auto mat = XMMatrixAffineTransformation(s, XMVectorZero(), r, p);
		XMStoreFloat4x4(&_matrices[i], XMMatrixTranspose(mat));
	}
}

const DirectX::XMFLOAT4X4* TransformData::GetMatrix(TransformID id)
{
	return &_matrices[id];
}


