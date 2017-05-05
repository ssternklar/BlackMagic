#pragma once

#ifdef DXSHADER
#define Vector4 float4
#define Vector3 float4
#define alignas(x)
#else
#include "BMMath.h"
#define Vector4 BlackMagic::Vector4
#define Vector3 BlackMagic::Vector3
#endif

#define MAX_DIR_LIGHTS 16
#ifndef DXSHADER

#endif
struct alignas(16) DirectionalLight
{
	Vector4 AmbientColor;
	Vector4 DiffuseColor;
	Vector3 Direction;
	Vector3 Up;
};

#define MAX_POINT_LIGHTS 256
struct alignas(16) PointLight
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