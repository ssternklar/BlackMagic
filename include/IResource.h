#pragma once
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