#pragma once
#include "Scene.h"
#include "Entity.h"
#include "MenuCursor.h"
#include "allocators\AllocatorSTLAdapter.h"
#include "Camera.h"
#include "Spline.h"
#include "Machine.h"
#include "WAVFile.h"
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

	BlackMagic::AssetPointer<BlackMagic::Material> lightMats[3];
	BlackMagic::Entity* lights[3];
	BlackMagic::AssetPointer<BlackMagic::WAVFile> startBoops[2];
	BlackMagic::AssetPointer<BlackMagic::WAVFile> bgm;
	bool gameIsStarted = false;
	int lap = 1;
	float start;
	float halfPoint;
	float lastT;
	bool crossedHalf;
	virtual void Update(float deltaTime) override;
	virtual void Draw(float deltaTime) override;
	virtual void ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material) override;
	virtual void Start() override;
};

