#pragma once

#include "BMMath.h"

struct Vertex
{
	BlackMagic::Vector3 Position; // The position of the vertex
	BlackMagic::Vector3 Normal;
	BlackMagic::Vector3 Tangent;
	BlackMagic::Vector3 Binormal;
	BlackMagic::Vector2 UV;
};
