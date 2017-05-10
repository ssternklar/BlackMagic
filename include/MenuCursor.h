#pragma once
#include "Entity.h"
#include "ContentClasses.h"
#include "WAVFile.h"
class MenuCursor :
	public BlackMagic::Entity
{
public:
	//Array of size 4
	bool prevUp = false;
	bool prevDown = false;
	int currentIndex = 0;
	Transform* transforms;
	BlackMagic::AssetPointer<BlackMagic::WAVFile>* sounds;
	MenuCursor(Transform& myTransform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material, Transform* transforms);
	void Init(BlackMagic::AssetPointer<BlackMagic::WAVFile>* sounds);
	virtual void Update(float deltaTime) override;
	~MenuCursor();
};

