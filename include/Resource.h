#pragma once

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "PlatformResourceTypes.h"

namespace BlackMagic
{
	class Renderer;
	class Resource
	{
	public:
		Resource();
		Resource(BlackMagic::Renderer* renderer, ResourceHandle* resource);
		Resource(const Resource& r);
		virtual ~Resource();

		Resource& operator=(const Resource& r);

		template<typename T>
		T* As() const { return static_cast<T*>(_resource); };
	protected:
		Renderer* _renderer;
		ResourceHandle* _resource;
	};
}