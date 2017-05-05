#include "Lights.h"

StructuredBuffer<DirectionalLight> DirectionalLights;
StructuredBuffer<PointLight> PointLights;

float4 main() : SV_TARGET
{
	return float4(1.0, 0.0, 0.0, 1.0);
}