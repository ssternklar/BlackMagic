#pragma once
#include "GameAbstraction.h"

#include "allocators\BadBestFitAllocator.h"

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

#include "Camera.h"
#include "DirectionalLight.h"
#include "Spline.h"
#include "Machine.h"
#include "MachineSystem.h"
#include "ECS.h"

class FZERO : public BlackMagic::GameAbstraction
{
public:
	FZERO(BlackMagic::PlatformBase* platformBase) : BlackMagic::GameAbstraction(platformBase) {};
	~FZERO();
	virtual void Init(byte* gameMemory, size_t memorySize) override;
	virtual void Update(float deltaTime) override;
	virtual void Draw(float deltaTime) override;
private:
	//Load meshes, textures, shaders, etc.
	void LoadContent();

	BlackMagic::BestFitAllocator allocator;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	ECS::ComponentHandle<Camera> _camera;

	std::vector<DirectionalLight> _directionalLights;

	std::shared_ptr<Mesh> splineMesh;
	std::shared_ptr<Spline> _spline;
	MachineSystem* sys;
	// ECS Stuff
	ECS::World* gameWorld;
};
