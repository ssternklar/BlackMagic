#pragma once

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace BlackMagic
{
	class GraphicsDevice;
	class IResource
	{
	public:
		BlackMagic::GraphicsDevice* device;
		IResource();
		IResource(BlackMagic::GraphicsDevice* device);
	};
}