#include <new>
#include "TransformData.h"
#include "Transform.h"

using namespace BlackMagic;

TransformData* TransformData::singletonRef = nullptr;

TransformData::TransformData() : matrixAllocator(
	64,
	sizeof(Mat4) * 400,
	sizeof(Mat4),
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
	Mat4* alloc = matrixAllocator.allocate<Mat4>();
	TransformID res = alloc - _matrices;
	_matrices[res].data[15] = 1;
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
	memset(&(_matrices[id]), 0, sizeof(Mat4));
	matrixAllocator.deallocate<Mat4>(&(_matrices[id]));
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
		auto r = _rotations[i];
		auto p = _positions[i];
		auto s = _scales[i];
		auto rS = CreateVector3Zero();
		auto mat = CreateAffineTransformation(
			_scales[i],
			rS,
			_rotations[i],
			_positions[i]
		);

		_matrices[i] = Transpose(mat);
	}
}

Mat4* TransformData::GetMatrix(TransformID id)
{
	return &_matrices[id];
}


