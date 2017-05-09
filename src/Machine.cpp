#include "Machine.h"
#include <cmath>
#include "PlatformBase.h"
using namespace BlackMagic;

Machine::Machine(const Vector3 & pos, const Quaternion & rot, const AssetPointer<BlackMagic::Mesh>& mesh, const Material & mat) :
	Entity(pos, rot, mesh, mat), spline(nullptr)
{
}

void SetTransformRotation(Transform& transform, SplineControlPoint* point)
{
	Vector3 newFwd = transform.GetForward() - (Dot(transform.GetForward(), point->normal) * point->normal);
	transform.SetRotation(CreateQuaternion(newFwd, point->normal));
}

void Machine::Init(AssetPointer<Spline> spline)
{
	this->spline = spline;
	lastTrackControlPoint = spline->GuessNearestPoint(GetTransform().GetPosition());
	Vector3 finalPos = lastTrackControlPoint.position + (lastTrackControlPoint.normal * .9f);
	GetTransform().MoveTo(finalPos);
	SetTransformRotation(GetTransform(), &lastTrackControlPoint);
	maxVelocity = 3;
	lastPositionInTrack = true;
}

void Machine::Update(float deltaTime)
{
	Transform& transform = GetTransform();
	Vector3 pos = GetTransform().GetPosition();
	lastTrackControlPoint = spline->GuessNearestPoint(pos);
	bool withinBounds = lastTrackControlPoint.IsInPlaneBounds(pos);
	bool heightGood = lastTrackControlPoint.IsCloseToPlane(pos, .15f);
	if (withinBounds && heightGood)
	{
		SetTransformRotation(transform, &lastTrackControlPoint);
		Vector3 planePos = lastTrackControlPoint.GetClosestPointOnPlane(pos);
		pos = planePos + lastTrackControlPoint.normal * .1f;
	}
	else if (lastPositionInTrack)
	{
		SetTransformRotation(transform, &lastTrackControlPoint);
		Vector3 planePos = lastTrackControlPoint.GetClosestPointOnPlane(pos);
		Vector3 diff = planePos - lastTrackControlPoint.position;
		float maxDiff = GetX(lastTrackControlPoint.scale / 2) * .98f;
		
		if (Magnitude(diff) > maxDiff)
		{
			diff = Normalize(diff) * maxDiff;
		}

		pos = lastTrackControlPoint.position + diff + (lastTrackControlPoint.normal * .1f);

		float slowAmt = abs(Dot(lastTrackControlPoint.tangent, transform.GetForward()));
		velocity = velocity * slowAmt;
		withinBounds = true;
	}
	else
	{
		Vector3 fwd = transform.GetForward();
		fwd = CreateVector3(GetX(fwd), 0, GetZ(fwd));
		Quaternion quat = CreateQuaternion(fwd, CreateVector3(0, 1, 0));
		quat = Slerp(quat, transform.GetRotation(), .1f * deltaTime);
		transform.SetRotation(quat);
		pos = pos + CreateVector3(0, -1, 0) * deltaTime;
	}

	transform.MoveTo(pos);
	BlackMagic::InputData* data = BlackMagic::PlatformBase::GetSingleton()->GetInputData();
	float inputPitch = (((data->GetButton(4)) ? 1 : 0) + ((data->GetButton(4)) ? -1 : 0));
	Quaternion rot = CreateQuaternion(0, M_PI / 180.0f * inputPitch, 0);
	transform.Rotate(rot);

	float inputFwd = (data->GetButton(3) ? 1 : 0) + (data->GetButton(5) ? -.5f : 0);

	Vector3 frameVelocity = CreateVector3(0, 0, inputFwd * maxVelocity * 30 * deltaTime);
	velocity = velocity + frameVelocity;
	
	if (Magnitude(velocity) > maxVelocity)
	{
		velocity = Normalize(velocity) * maxVelocity;
	}

	//friction
	velocity = velocity - (velocity * .01f * 60 * deltaTime);

	Vector3 localVelocity = Rotate(velocity * deltaTime, transform.GetRotation());
	transform.Move(localVelocity);
	lastPositionInTrack = withinBounds && heightGood;
}

Machine::~Machine()
{
}
