#pragma once
#include "GameAbstraction.h"

#include "allocators\BadBestFitAllocator.h"
#include "DirectionalLight.h"
#include "Entity.h"
#include "FirstPersonCamera.h"

class TestGame : public BlackMagic::GameAbstraction
{
public:
	TestGame(BlackMagic::PlatformBase* platformBase);
	virtual void Init(BlackMagic::byte* gameMemory, size_t memorySize) override;
	virtual void Update(float deltaTime) override;
	virtual void Draw(float deltaTime) override;
	virtual void Destroy() override;
private:
	//Load meshes, textures, shaders, etc.
	void LoadContent();

	BlackMagic::BestFitAllocator allocator;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	BlackMagic::FirstPersonCamera _camera;
	DirectionalLight _globalLight;
	std::vector<BlackMagic::Entity*> _objects;
};
