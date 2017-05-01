#pragma once
#include "Scene.h"
class RacingScene : public BlackMagic::Scene
{
public:
	RacingScene();
	~RacingScene();

	virtual void Update(float deltaTime);
	virtual void Draw(float deltaTime);
	virtual void ProcessType(uint16_t tag, Transform transform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material);
};

