#pragma once
#include "GameAbstraction.h"

#include "allocators\BadBestFitAllocator.h"

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "DirectionalLight.h"
#include "ECS.h"
#include "GameAbstraction.h"

class TestGame : public BlackMagic::GameAbstraction
{
public:
	TestGame(BlackMagic::PlatformBase* platformBase);
	virtual void Init(byte* gameMemory, size_t memorySize) override;
	virtual void Update(float deltaTime) override;
	virtual void Draw(float deltaTime) override;
	virtual void Destroy() override;
private:
	//Load meshes, textures, shaders, etc.
	void LoadContent();

	BlackMagic::BestFitAllocator allocator;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
	ECS::ComponentHandle<Camera> _camera;
	DirectionalLight _globalLight;
	ECS::World* _gameWorld;
};