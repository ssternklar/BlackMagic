#pragma once

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace BlackMagic
{
	class Renderer;
	class IResource
	{
	public:
		BlackMagic::Renderer* device;
		IResource();
		IResource(BlackMagic::Renderer* device);
	};
}