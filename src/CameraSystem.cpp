#include "CameraSystem.h"

using namespace ECS;

void CameraSystem::configure(ECS::World* world)
{
	world->subscribe<Events::OnComponentAssigned<Camera>>(this);
}

void CameraSystem::unconfigure(ECS::World* world)
{
	world->unsubscribeAll(this);
}
