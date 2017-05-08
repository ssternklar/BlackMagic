#pragma once
#include "Entity.h"
#include "ContentClasses.h"
class MenuCursor :
	public BlackMagic::Entity
{
public:
	//Array of size 4
	bool prevUp = false;
	bool prevDown = false;
	int currentIndex = 0;
	Transform* transforms;
	MenuCursor(Transform& myTransform, BlackMagic::AssetPointer<BlackMagic::Mesh> mesh, BlackMagic::AssetPointer<BlackMagic::Material> material, Transform* transforms);
	virtual void Update(float deltaTime);
	~MenuCursor();
};

