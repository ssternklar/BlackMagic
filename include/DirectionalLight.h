#pragma once
#include "BMMath.h"

struct DirectionalLight
{
	BlackMagic::Vector4 AmbientColor;
	BlackMagic::Vector4 DiffuseColor;
	BlackMagic::Vector3 Direction;
	BlackMagic::Vector3 Up;
};
