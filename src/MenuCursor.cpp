#include "MenuCursor.h"
#include "PlatformBase.h"
#include "ContentManager.h"

using namespace BlackMagic;

MenuCursor::MenuCursor(Transform & myTransform, AssetPointer<Mesh> mesh, AssetPointer<Material> material, Transform * transforms) : Entity(myTransform.GetPosition(), myTransform.GetRotation(),
	(mesh),
	*material)
{
	this->transforms = transforms;
}

void MenuCursor::Update(float deltaTime)
{
	auto inputData = PlatformBase::GetSingleton()->GetInputData();
	
	if (inputData->GetButton(3) && !prevUp)
	{
		currentIndex -= 1;
	}
	else if (inputData->GetButton(5) && !prevDown)
	{
		currentIndex += 1;
	}
	currentIndex = abs(currentIndex) % 2;
	printf("%d\n", currentIndex);
	prevUp = inputData->GetButton(3);
	prevDown = inputData->GetButton(5);

	this->_transform.MoveTo(transforms[currentIndex].GetPosition());

}


MenuCursor::~MenuCursor()
{
}
