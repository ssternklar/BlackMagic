#pragma once
#include "Scene.h"
#include "Entity.h"
#include "MenuCursor.h"
#include "allocators\AllocatorSTLAdapter.h"
#include "Camera.h"
#include "Spline.h"
#include "Machine.h"
class RacingScene : public BlackMagic::Scene
{
public:
	RacingScene(BlackMagic::BestFitAllocator* allocator);
	~RacingScene();

	//Generic Stuff
	Camera* camera;
	BMList<BlackMagic::Entity*> entities;
	DirectionalLight _globalLight;

	//Menu Stuff
	Transform menuPositions[2];
	MenuCursor* cursor;

	//Game Stuff
	BlackMagic::AssetPointer<BlackMagic::Spline> spline;
	Machine* machine = nullptr;

	virtual void Update(float deltaTime) override;
	virtual void Draw(float deltaTime) override;
	virtual void ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material) override;
	virtual void Start() override;
};

