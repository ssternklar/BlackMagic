#include "MachineSystem.h"
#include <Windows.h>

#define KEYPRESSED(char) (GetAsyncKeyState(char) & 0x8000)

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

inline DirectX::XMFLOAT4 QuatLookRotation(DirectX::XMFLOAT3& lookAt, DirectX::XMFLOAT3& up)
{
	using namespace DirectX;
	auto tangent = XMLoadFloat3(&lookAt);
	auto normal = XMLoadFloat3(&up);
	auto binormal = XMVector3Cross(tangent, normal);
	normal = XMVector3Cross(tangent, binormal);
	XMMATRIX mat;
	mat.r[0] = binormal;
	mat.r[1] = normal;
	mat.r[2] = tangent;
	XMFLOAT4 quat;
	XMStoreFloat4(&quat, XMQuaternionRotationMatrix(mat));
	return quat;
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
			auto fwd = transform->GetForward();
			fwd.y = 0;
			XMFLOAT4 quat = QuatLookRotation(fwd, XMFLOAT3{ 0,1,0 });
			XMStoreFloat4(&quat, XMQuaternionSlerp(XMLoadFloat4(&quat), XMLoadFloat4(&transform->GetRotation()), .1f));
			transform->SetRotation(quat);
			XMFLOAT3 pos;
			XMStoreFloat3(&pos, XMVectorSet(0, -1, 0, 0) * 2.5f * deltaTime);
			transform->MoveTo(pos);
		}
		XMFLOAT4 rot;
		XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, (XM_PI / 180) * (KEYPRESSED('A') ? -1 : 0) + (KEYPRESSED('D') ? 1 : 0) * .1f * deltaTime, 0));
		transform->Rotate(rot);
		XMFLOAT3 tempVelocity = {0, (KEYPRESSED('S') ? -.5f : 0) + (KEYPRESSED('W') ? 1 : 0), 0};
		if (machine->isBoosting)
		{
			tempVelocity.y += .05f * deltaTime;
		}
		velocity += XMLoadFloat3(&tempVelocity);
		velocity += -velocity * .01f;
		XMStoreFloat3(&machine->velocity, velocity);
		XMFLOAT3 localVelocity;
		XMStoreFloat3(&localVelocity, XMVector3Rotate(velocity, XMLoadFloat4(&transform->GetRotation())));
		transform->Move(localVelocity);
		machine->lastPositionInTrack = withinBounds && heightGood;
	});
}

void MachineSystem::SetTransformRotation(Transform* transform, SplineControlPoint* point)
{
	using namespace DirectX;
	auto transformFwdV = XMLoadFloat3(&transform->GetForward());
	auto normalV = XMLoadFloat3(&point->normal);
	XMFLOAT3 fwd;
	XMStoreFloat3(&fwd, transformFwdV - (XMVector3Dot(transformFwdV, normalV) * normalV));
	transform->SetRotation(QuatLookRotation(fwd, point->normal));
}