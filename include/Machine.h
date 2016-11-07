#pragma once

#include <memory>
#include "Spline.h"
#include "DXCore.h"
#include "Transform.h"

class Machine
{
public:
	DirectX::XMFLOAT3 velocity = {0,0,0};
	SplineControlPoint lastTrackControlPoint;
	float maxVelocity = 1.f;
	float boostVelocity = 1.2f;
	float internalMaxVelocity;
	bool lastPositionInTrack = true;
	bool isBoosting = false;
	Machine();
};