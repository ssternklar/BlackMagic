#pragma once

#include "Camera.h"
#include "ECS.h"

class CameraSystem : public ECS::EntitySystem, public ECS::EventSubscriber<ECS::Events::OnComponentAssigned<Camera>>
{
public:
	void configure(ECS::World*) override;
	void unconfigure(ECS::World*) override;
	void receive(ECS::World*, const ECS::Events::OnComponentAssigned<Camera>&) override;
	void tick(ECS::World*, ECS_TICK_TYPE) override;

private:
};