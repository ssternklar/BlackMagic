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
	XMFLOAT3 planePos = machine->lastTrackControlPoint.position;
	XMFLOAT3 finalPos;
	XMStoreFloat3(&finalPos, XMLoadFloat3(&planePos) + XMLoadFloat3(&machine->lastTrackControlPoint.normal) * .1f);
	transform->MoveTo(finalPos);
	SetTransformRotation(&(transform.get()), &(machine->lastTrackControlPoint));

	machine->maxVelocity = 3;
	machine->internalMaxVelocity = 3.6f;

	auto camera = event.entity->get<Camera>();
	if (camera.isValid())
	{
		camera->Update(&transform.get());
	}
}

inline DirectX::XMFLOAT4 QuatLookRotation(DirectX::XMFLOAT3& lookAt, DirectX::XMFLOAT3& up)
{
	using namespace DirectX;
	auto forwardV = XMVector3Normalize(XMLoadFloat3(&lookAt));
	auto upV = XMVector3Normalize(XMLoadFloat3(&up));
	auto rightV = XMVector3Normalize(XMVector3Cross(upV, forwardV));
	upV = XMVector3Normalize(XMVector3Cross(forwardV, rightV));
	XMMATRIX mat;
	mat.r[0] = rightV;
	mat.r[1] = upV;
	mat.r[2] = forwardV;
	mat.r[3] = XMVectorSet(0, 0, 0, 1);
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
			machine->lastTrackControlPoint.GetClosestPointOnPlane(position, &planePos);
			XMStoreFloat3(&position, XMLoadFloat3(&planePos) + XMLoadFloat3(&machine->lastTrackControlPoint.normal) * .1f);
		}
		else if (hasWalls && machine->lastPositionInTrack && heightGood)
		{
			MachineSystem::SetTransformRotation(&transform.get(), &machine->lastTrackControlPoint);
			XMFLOAT3 closestPointOnPlane;
			machine->lastTrackControlPoint.GetClosestPointOnPlane(position, &closestPointOnPlane);
			auto pointPosV = XMLoadFloat3(&machine->lastTrackControlPoint.position);
			auto closestPointV = XMLoadFloat3(&closestPointOnPlane);
			XMStoreFloat3(&position, pointPosV + XMVector3ClampLength(closestPointV - pointPosV, 0, (machine->lastTrackControlPoint.scale.x / 2) * .98f) + (XMLoadFloat3(&machine->lastTrackControlPoint.normal) * .1f));
			auto slowAmt = XMVectorAbs(XMVector3Dot(XMLoadFloat3(&machine->lastTrackControlPoint.tangent), XMLoadFloat3(&transform->GetForward())));
			float healthDecreaseAmt;
			XMStoreFloat(&healthDecreaseAmt, XMVector3Length(velocity * slowAmt * 10));
			velocity = velocity * slowAmt;
			machine->health -= healthDecreaseAmt;
			withinBounds = true;
		}
		else
		{
			auto fwd = transform->GetForward();
			fwd.y = 0;
			XMFLOAT4 quat = QuatLookRotation(fwd, XMFLOAT3{ 0,1,0 });
			XMStoreFloat4(&quat, XMQuaternionSlerp(XMLoadFloat4(&quat), XMLoadFloat4(&transform->GetRotation()), .1f * deltaTime));
			transform->SetRotation(quat);
			XMStoreFloat3(&position, XMLoadFloat3(&position) + XMVectorSet(0, -1, 0, 0) * deltaTime);
		}
		transform->MoveTo(position);
		XMFLOAT4 rot;
		XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, (XM_PI / 180) * ((KEYPRESSED('A') ? -1 : 0) + (KEYPRESSED('D') ? 1 : 0)) * 90 * deltaTime, 0));
		transform->Rotate(rot);
		XMFLOAT3 tempVelocity = {0, 0, ((KEYPRESSED('S') ? -.5f : 0) + (KEYPRESSED('W') ? 1 : 0)) * machine->maxVelocity * 30 * deltaTime};
		if (machine->isBoosting)
		{
			tempVelocity.z += .1f * deltaTime;
		}
		velocity += XMLoadFloat3(&tempVelocity);
		velocity = XMVector3ClampLength(velocity, 0, machine->internalMaxVelocity);
		velocity += -velocity * (.01f * 60) * deltaTime;
		XMStoreFloat3(&machine->velocity, velocity);
		XMFLOAT3 localVelocity;
		XMStoreFloat3(&localVelocity, XMVector3Rotate(velocity * deltaTime, XMLoadFloat4(&transform->GetRotation())));
		transform->Move(localVelocity);
		machine->lastPositionInTrack = withinBounds && heightGood;
	});
	world->each<Transform, Machine, Camera>([](ECS::Entity* ent, ECS::ComponentHandle<Transform> transform, ECS::ComponentHandle<Machine> machine, ECS::ComponentHandle<Camera> camera) {
		camera->Update(&transform.get());
	});
}

void MachineSystem::SetTransformRotation(Transform* transform, SplineControlPoint* point)
{
	using namespace DirectX;
	auto transformFwdV = XMLoadFloat3(&transform->GetForward());
	auto normalV = XMLoadFloat3(&point->normal);
	auto diff = (XMVector3Dot(transformFwdV, normalV) * normalV);
	XMFLOAT3 fwd;
	XMStoreFloat3(&fwd, XMVector3Normalize(transformFwdV - diff));
	transform->SetRotation(QuatLookRotation(fwd, point->normal));
}