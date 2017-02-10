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

void CameraSystem::receive(ECS::World* world, const ECS::Events::OnComponentAssigned<Camera>& e)
{
	
}


void CameraSystem::tick(ECS::World* world, ECS::DefaultTickData data)
{
	
}

