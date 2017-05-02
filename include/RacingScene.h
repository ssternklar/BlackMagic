#pragma once
#include "Scene.h"
#include "Entity.h"
#include "MenuCursor.h"
#include "allocators\AllocatorSTLAdapter.h"
#include "Camera.h"
class RacingScene : public BlackMagic::Scene
{
public:
	RacingScene(BlackMagic::BestFitAllocator* allocator);
	~RacingScene();

	//Menu Stuff
	Transform menuPositions[4];
	MenuCursor* cursor;

	Camera* camera;
	BMList<BlackMagic::Entity*> entities;

	virtual void Update(float deltaTime) override;
	virtual void Draw(float deltaTime) override;
	virtual void ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material) override;
};

