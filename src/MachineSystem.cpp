#include "MachineSystem.h"

void MachineSystem::configure(ECS::World* world)
{
	//world->subscribe<ECS::Events::OnComponentAssigned<Machine>>(this);
}

void MachineSystem::unconfigure(ECS::World* world)
{
	world->unsubscribeAll(this);
}

MachineSystem::MachineSystem(std::shared_ptr<Spline> track)
{
	this->track = track;
}

void MachineSystem::tick(ECS::World* world, float deltaTime)
{
	/*world->each<Transform, Machine>([deltaTime](ECS::Entity* ent, Transform* transform, Machine* machine) {

	});*/
}

void MachineSystem::SetTransformRotation(Transform* transform, SplineControlPoint* point)
{
	using namespace DirectX;
}