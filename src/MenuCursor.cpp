#include "MenuCursor.h"
#include "PlatformBase.h"
#include "ContentManager.h"

using namespace BlackMagic;

MenuCursor::MenuCursor(Transform & myTransform, AssetPointer<Mesh> mesh, AssetPointer<Material> material, Transform * transforms) : Entity(myTransform.GetPosition(), myTransform.GetRotation(),
	PlatformBase::GetSingleton()->GetContentManager()->ConvertToSharedPtr(mesh),
	*material)
{

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
	while (GetX(transforms[currentIndex].GetPosition()) == 0)
	{
		currentIndex++;
		currentIndex = ((currentIndex % 4) + currentIndex) % 4;
	}
	
	prevUp = inputData->GetButton(3);
	prevDown = inputData->GetButton(5);

	this->_transform.MoveTo(transforms[currentIndex].GetPosition());

}


MenuCursor::~MenuCursor()
{
}
