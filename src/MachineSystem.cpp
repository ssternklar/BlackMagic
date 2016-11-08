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
	auto machine = event.component;
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
	using namespace DirectX;
	Spline* s = track.get();
	world->each<Transform, Machine>([deltaTime, s](ECS::Entity* ent, ECS::ComponentHandle<Transform> transform, ECS::ComponentHandle<Machine> machine) {
		XMFLOAT3 position = transform->GetPosition();
		auto velocity = XMLoadFloat3(&machine->velocity);
		s->GuessNearestPoint(position, machine->lastTrackControlPoint);
		bool withinBounds = machine->lastTrackControlPoint.IsInPlaneBounds(position);
		bool heightGood = machine->lastTrackControlPoint.IsCloseToPlane(position, .15f);
		bool hasWalls = true;
		if (withinBounds && heightGood)
		{
			MachineSystem::SetTransformRotation(&transform.get(), &machine->lastTrackControlPoint);
			XMFLOAT3 planePos;
			machine->lastTrackControlPoint.GetClosestPointOnPlane(transform->GetPosition(), &planePos);
			XMStoreFloat3(&position, XMLoadFloat3(&position) + XMLoadFloat3(&machine->lastTrackControlPoint.normal) * .1f);
			transform->MoveTo(position);
		}
		else if (hasWalls && machine->lastPositionInTrack && heightGood)
		{
			MachineSystem::SetTransformRotation(&transform.get(), &machine->lastTrackControlPoint);
			XMFLOAT3 closestPointOnPlane;
			machine->lastTrackControlPoint.GetClosestPointOnPlane(position, &closestPointOnPlane);
			auto pointPosV = XMLoadFloat3(&machine->lastTrackControlPoint.position);
			auto closestPointV = XMLoadFloat3(&closestPointOnPlane);
			XMFLOAT3 pos;
			XMStoreFloat3(&position, pointPosV + XMVector3ClampLength(closestPointV - pointPosV, 0, (machine->lastTrackControlPoint.scale.x / 2) * .98f) + (XMLoadFloat3(&machine->lastTrackControlPoint.normal) * .1f));
			auto slowAmt = XMVector3Dot(XMLoadFloat3(&machine->lastTrackControlPoint.tangent), XMLoadFloat3(&transform->GetForward()));
			float healthDecreaseAmt;
			XMStoreFloat(&healthDecreaseAmt, XMVector3Length(velocity * slowAmt * 10));
			velocity = velocity * slowAmt;
			machine->health - healthDecreaseAmt;
			withinBounds = true;
		}
		else
		{
			//FALL
		}

		//INPUT


	});
}

void MachineSystem::SetTransformRotation(Transform* transform, SplineControlPoint* point)
{
	using namespace DirectX;
	auto tangent = XMLoadFloat3(&(point->tangent));
	auto normal = XMLoadFloat3(&(point->normal));
	auto binormal = XMVector3Cross(tangent, normal);
	XMMATRIX mat;
	mat.r[0] = binormal;
	mat.r[1] = normal;
	mat.r[2] = tangent;
	XMFLOAT4 quat;
	XMStoreFloat4(&quat, XMQuaternionRotationMatrix(mat));
	transform->SetRotation(quat);
}