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
	float maxVelocity = 60;
	float boostVelocity = 72;
	float internalMaxVelocity;
	float health = 100;
	bool lastPositionInTrack = true;
	bool isBoosting = false;
	Machine();
};