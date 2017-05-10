#pragma once
#include "Entity.h"
#include "BMMath.h"
#include "Spline.h"
class Machine : public BlackMagic::Entity
{
public:
	BlackMagic::Vector3 velocity;
	BlackMagic::SplineControlPoint lastTrackControlPoint;
	BlackMagic::AssetPointer<BlackMagic::Spline> spline;
	float maxVelocity;
	bool lastPositionInTrack;
	Machine(const BlackMagic::Vector3& pos, const BlackMagic::Quaternion& rot, const BlackMagic::AssetPointer<BlackMagic::Mesh>& mesh, const BlackMagic::Material& mat);
	void Init(BlackMagic::AssetPointer<BlackMagic::Spline> spline);
	virtual void Update(float deltaTime) override;
	virtual ~Machine();
};

