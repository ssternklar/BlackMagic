#pragma once

#include "BMMath.h"
#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	BlackMagic::Vector3 Position; // The position of the vertex
	BlackMagic::Vector3 Normal;
	BlackMagic::Vector3 Tangent;
	BlackMagic::Vector3 Binormal;
	BlackMagic::Vector2 UV;
};
