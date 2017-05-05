#pragma once

#ifdef DXSHADER
#define Vector4 float4
#define Vector3 float4
#else
#include "BMMath.h"
#define Vector4 BlackMagic::Vector4
#define Vector3 BlackMagic::Vector3
#endif

struct DirectionalLight
{
	Vector4 AmbientColor;
	Vector4 DiffuseColor;
	Vector3 Direction;
	Vector3 Up;
};

struct PointLight
{
	Vector4 Color;
	Vector3 Position;
	float Intensity;
	float SourceRadius;
	float AttenuationRadius;
	float _padding;
};

#undef Vector4
#undef Vector3