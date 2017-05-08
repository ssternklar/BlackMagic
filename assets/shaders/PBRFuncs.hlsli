#pragma once

float3 DiffuseBRDF(float3 albedo)
{
	return albedo / PI;
}

float GGX_TR_D(float3 n, float3 h, float r)
{
	float a = pow(r, 4);
	return a / (PI*pow(pow(saturate(dot(n, h)), 2) * (a - 1) + 1, 2));
}

float SchlickG1(float3 n, float3 v, float k)
{
	return 1.0 / (saturate(dot(n, v))*(1 - k) + k);
}

float SchlickG(float3 n, float3 v, float3 l, float r)
{
	float k = pow(r + 1, 2) / 8;
	return SchlickG1(n, v, k) * SchlickG1(n, l, k);
}

float SchlickGaussianF(float3 v, float3 h, float f0)
{
	float vh = saturate(dot(v, h));
	return f0 + (1 - f0)*pow(2, (-5.55473*vh - 6.98316)*vh);
}

float3 CT_BRDF(float3 v, float3 l, float3 n, float r, float f0)
{
	float3 h = normalize((l + v) / 2);
	float D = GGX_TR_D(n, h, r);
	float F = SchlickGaussianF(v, h, f0);
	float G = SchlickG(n, v, l, r);
	return D * F * G / 4.0;
}

