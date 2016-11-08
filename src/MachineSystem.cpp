#include "MachineSystem.h"

void MachineSystem::configure(ECS::World* world)
{
	world->subscribe<ECS::Events::OnComponentAssigned<Machine>>(this);
}

void MachineSystem::unconfigure(ECS::World* world)
{
	world->unsubscribeAll(this);
}

MachineSystem::MachineSystem(std::shared_ptr<Spline> track)
{
	this->track = track;
}

void MachineSystem::receive(ECS::World* world, const ECS::Events::OnComponentAssigned<Machine>& event)
{
	using namespace DirectX;
	auto transform = event.entity->get<Transform>();
	auto& machine = event.component;
	track->GuessNearestPoint(transform->GetPosition(), machine->lastTrackControlPoint);
	SetTransformRotation(&(transform.get()), &(machine->lastTrackControlPoint));
	XMFLOAT3 planePos;
	XMFLOAT3 finalPos;
	machine->lastTrackControlPoint.GetClosestPointOnPlane(transform->GetPosition(), &planePos);
	XMStoreFloat3(&finalPos, XMLoadFloat3(&transform->GetPosition()) + XMLoadFloat3(&machine->lastTrackControlPoint.normal) * .1f);
	transform->MoveTo(finalPos);
}

void MachineSystem::tick(ECS::World* world, float deltaTime)
{
	world->each<Transform, Machine>([deltaTime](ECS::Entity* ent, ECS::ComponentHandle<Transform> transform, ECS::ComponentHandle<Machine> machine) {
		
	});
}

void MachineSystem::SetTransformRotation(Transform* transform, SplineControlPoint* point)
{
	using namespace DirectX;
}