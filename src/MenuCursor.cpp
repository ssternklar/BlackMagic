#include "MenuCursor.h"
#include "PlatformBase.h"
#include "ContentManager.h"
#include "SceneBasedGame.h"
#include "RacingScene.h"
using namespace BlackMagic;

MenuCursor::MenuCursor(Transform & myTransform, AssetPointer<Mesh> mesh, AssetPointer<Material> material, Transform * transforms) : Entity(myTransform.GetPosition(), myTransform.GetRotation(),
	(mesh),
	*material)
{
	this->transforms = transforms;
}

void MenuCursor::Init(BlackMagic::AssetPointer<BlackMagic::WAVFile>* sounds)
{
	this->sounds = sounds;
}

void MenuCursor::Update(float deltaTime)
{
	auto inputData = PlatformBase::GetSingleton()->GetInputData();
	
	if (inputData->GetButton(3) && !prevUp)
	{
		currentIndex -= 1;
		//PlatformBase::GetSingleton()->GetAudioManager()->PlayOneShot(sounds[0].get(), 1);
	}
	else if (inputData->GetButton(5) && !prevDown)
	{
		currentIndex += 1;
		//PlatformBase::GetSingleton()->GetAudioManager()->PlayOneShot(sounds[0].get(), 1);
	}
	currentIndex = abs(currentIndex) % 2;
	prevUp = inputData->GetButton(3);
	prevDown = inputData->GetButton(5);

	if (inputData->GetButton(8))
	{
		PlatformBase::GetSingleton()->GetAudioManager()->PlayOneShot(sounds[1].get(), 1);
		SceneBasedGame<RacingScene>::GetSingleton()->StartSceneLoad("scenes/easyRace.scene");
	}

	this->_transform = (transforms[currentIndex]);

}


MenuCursor::~MenuCursor()
{
}
