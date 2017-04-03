#pragma once
#include "Resource.h"
#include "PlatformResourceTypes.h"

namespace BlackMagic
{
	struct Buffer : public Resource
	{
		enum Type
		{
			VERTEX_BUFFER,
			INDEX_BUFFER
		};
		
		Buffer(Renderer* r, BufferHandle* buffer)
			: Resource(r, buffer) {}
		Buffer() = default;
		Buffer(const Buffer& b) { *this = b; }

		Buffer& operator=(const Buffer& b) { Resource::operator=(b); return *this;  }
		operator BufferHandle*() { return static_cast<BufferHandle*>(_resource); }
	};

	struct Sampler : public Resource
	{
		Sampler() = default;
		Sampler(Renderer* r, SamplerHandle* handle)
			: Resource(r, handle) {}

		operator SamplerHandle*() { return static_cast<SamplerHandle*>(_resource); }
	};
}

#undef _BM_GRAPHICS_TYPE_INFO(X)
