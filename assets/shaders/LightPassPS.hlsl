#define NUM_SHADOW_CASCADES 5
#define ZNEAR 0.1f
#define ZFAR 100.0f
#define SPLIT_SIZE ((ZFAR - ZNEAR)/NUM_SHADOW_CASCADES)
#define GEN_SHADOW_MAPS 1
#define PI 3.141592654

#include "Lights.h"
#include "PBRFuncs.hlsli"
#include "ShaderUtils.hlsli"

struct GBuffer
{
	float4 albedo;
	float3 position;
	float roughness;
	float3 normal;
	float cavity;
	float metal;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer perFrame : register(b0)
{
	matrix lightView[NUM_SHADOW_CASCADES];
	matrix lightProjection[NUM_SHADOW_CASCADES];
	float3 cameraPosition;
	uint numActiveDirectionalLights;
	uint numActivePointLights;
	DirectionalLight sceneLight;
};

cbuffer DLights : register(b1)
{
	DirectionalLight directionalLights[MAX_DIR_LIGHTS];
}

cbuffer PLights : register(b2)
{
	PointLight pointLights[MAX_POINT_LIGHTS];
}

Texture2D albedoMap : register(t0);
Texture2D positionMap : register(t1);
Texture2D roughnessMap: register(t2);
Texture2D normalMap : register(t3);
Texture2DArray shadowMap : register(t4);
Texture2D depth : register(t5);
Texture2D metalnessMap : register(t6);
Texture2D cavityMap : register(t7);
TextureCube skyboxEnvMap : register(t8);
TextureCube skyboxRadianceMap : register(t9);
TextureCube skyboxIrradianceMap : register(t10);
Texture2D cosLookup : register(t11);
SamplerState mainSampler : register(s0);
SamplerComparisonState shadowSampler : register(s1);
SamplerState envSampler : register(s2);

float3 ApproximateIBL(float3 specColor, float r, float3 n, float3 v)
{
	float nDotV = saturate(dot(n, v));
	float3 dir = 2 * dot(n, v) * n - v;
	uint numLevels = 0;
	uint _ = 0;
	skyboxRadianceMap.GetDimensions(0, _, _, numLevels);
	float2 brdf = pow(cosLookup.Sample(mainSampler, float2(nDotV, 1 - r)), 2.2).rg;
	float3 filteredEnvColor = pow(skyboxRadianceMap.SampleLevel(envSampler, dir, r * numLevels), 2.2);
	return filteredEnvColor * (specColor * brdf.r + brdf.g);
}

float SampleShadowMap(float3 pos, float3 normal, float cascade)
{
	float4 lightspacePos = mul(float4(pos, 1.0f), mul(lightView[cascade], lightProjection[cascade]));
	lightspacePos /= lightspacePos.w;
	float4 shadowCoord = lightspacePos / 2 + 0.5f;
	shadowCoord.y = 1 - shadowCoord.y;

	//8x8 PCF
	float sMap = 0.0f;
	float center = shadowMap.SampleCmpLevelZero(shadowSampler, float3(shadowCoord.xy, cascade), lightspacePos.z, int2(0, 0));
	for (int y = -3; y <= 3; y++)
	{
		for (int x = -3; x <= 3; x++)
		{
			sMap += shadowMap.SampleCmpLevelZero(shadowSampler, float3(shadowCoord.xy, cascade), lightspacePos.z, int2(x, y));
		}
	}
	sMap /= 48;
	return sMap;
}

float SampleBlendedShadowMap(float3 pos, float3 normal, float cascade, float linearDepth)
{
	//Based on MJP's CSM blending implementation
	float distToNextCascade = (ZNEAR + (cascade + 1) * SPLIT_SIZE) / ZFAR - linearDepth;
	float shadow = SampleShadowMap(pos, normal, cascade);
	//TODO: Get rid of this branch
	if (distToNextCascade <= 0.1f && cascade < NUM_SHADOW_CASCADES - 1)
	{
		float nextShadow = SampleShadowMap(pos, normal, cascade + 1);
		float t = smoothstep(0.0f, 0.1f, distToNextCascade);
		shadow = lerp(nextShadow, shadow, t);
	}

	return shadow;
}

float3 ColorFromDirectionalLights(GBuffer input)
{
    float3 v = normalize(cameraPosition - input.position);
    float3 l = -normalize(directionalLights[0].Direction.xyz);
    float3 dir = 2 * dot(input.normal, v) * input.normal - v;
    float f0 = lerp(0.04, length(input.albedo.rgb), input.metal);

    float3 diffuseColor = lerp(input.albedo.rgb, 0, input.metal);
    float3 specColor = lerp(0.04, input.albedo.rgb, input.metal);
    float specIntensity = CT_BRDF(v, l, input.normal, input.roughness, f0);
    float diffuseIntensity = 1-f0;

    float3 directDiffuse = DiffuseBRDF(diffuseColor) * diffuseIntensity;
    float3 directSpecular = specColor * specIntensity;
    float3 indirectDiffuse = (pow(skyboxIrradianceMap.Sample(envSampler, dir).rgb * diffuseColor, 2.2));
    float3 indirectSpecular = ApproximateIBL(specColor, input.roughness, input.normal, v);

    return directionalLights[0].DiffuseColor.rgb * saturate(dot(input.normal, l)) * (directDiffuse + directSpecular) + indirectDiffuse + indirectSpecular;
}

//TODO: Use optimization from http://vec3.ca/code/graphics/deferred-shading-tricks/ to reduce size of 
//position buffer
float4 main(VertexToPixel input) : SV_TARGET
{
	GBuffer buffer;
	buffer.albedo = albedoMap.Sample(mainSampler, input.uv);
	buffer.normal = DecompressNormal(normalMap.Sample(mainSampler, input.uv).xy);
	buffer.metal = metalnessMap.Sample(mainSampler, input.uv);
	buffer.position = positionMap.Sample(mainSampler, input.uv);
	buffer.roughness = roughnessMap.Sample(mainSampler, input.uv);
	buffer.cavity = cavityMap.Sample(mainSampler, input.uv);

	float linearDepth = LinearizeDepth((depth.Sample(mainSampler, input.uv).r), ZNEAR, ZFAR);
	float depthID = floor(linearDepth * NUM_SHADOW_CASCADES);
	/*float3 shadowColor = float3(0, 0, 0);
	if (depthID <= 1.0)
		shadowColor = float3(1, 0, 0);
	else if (depthID <= 2.0)
		shadowColor = float3(0, 1, 0);
	else if (depthID <= 3.0)
		shadowColor = float3(0, 0, 1);
	else if (depthID <= 4.0)
		shadowColor = float3(1, 1, 0);
	else
		shadowColor = float3(0, 1, 1); */

	float shadow = 1.0;
#if GEN_SHADOW_MAPS == 1
	shadow = SampleBlendedShadowMap(
		buffer.position, 
		buffer.normal,
		depthID,
		linearDepth
	);
#endif
	return float4(pow(ColorFromDirectionalLights(buffer)*(shadow/2 + 0.5), 1), linearDepth < 1);
}