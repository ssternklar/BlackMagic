#include "Machine.h"

using namespace BlackMagic;

Machine::Machine(const Vector3 & pos, const Quaternion & rot, const AssetPointer<BlackMagic::Mesh>& mesh, const Material & mat) :
	Entity(pos, rot, mesh, mat), spline(nullptr)
{
}

void Machine::Init(AssetPointer<Spline> spline)
{
	this->spline = spline;
	lastTrackControlPoint = spline->GuessNearestPoint(GetTransform().GetPosition());
	Vector3 finalPos = lastTrackControlPoint.position + (lastTrackControlPoint.rotation * CreateVector3(0, 1, 0));
	GetTransform().MoveTo(finalPos);
	GetTransform().SetRotation(lastTrackControlPoint.rotation);
	maxVelocity = 3;
}

void Machine::Update(float deltaTime)
{
	lastTrackControlPoint = spline->GuessNearestPoint(GetTransform().GetPosition());
	bool withinBounds = lastTrackControlPoint
}

Machine::~Machine()
{
}
