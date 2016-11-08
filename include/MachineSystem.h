#pragma once
#include<memory>
#include "ECS.h"
#include "Machine.h"
class MachineSystem : public ECS::EntitySystem
{
	std::shared_ptr<Spline> track;
	MachineSystem(std::shared_ptr<Spline> track);
	virtual void configure(ECS::World* world) override;
	virtual void unconfigure(ECS::World* world) override;
	virtual void tick(ECS::World* world, float deltaTime) override;
	virtual void receive(ECS::World* world, const ECS::Events::OnComponentAssigned<Machine>& event);
	void SetTransformRotation(Transform* transform, SplineControlPoint* point);
};