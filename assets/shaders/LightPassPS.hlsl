#define NUM_LIGHTS 1
#define NUM_SHADOW_CASCADES 5
#define ZNEAR 0.1f
#define ZFAR 100.0f
#define SPLIT_SIZE ((ZFAR - ZNEAR)/NUM_SHADOW_CASCADES)
#define GEN_SHADOW_MAPS 0
#define PI 3.141592654

struct DirectionalLight
{
	float4 AmbientColor : COLOR;
	float4 DiffuseColor : COLOR;
	float4 Direction	: NORMAL;
	float4 Up			: NORMAL;
};

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
	DirectionalLight sceneLight;
	float3 cameraPosition;
};

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
	float k = pow(r+1, 2) / 8;
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

float3 ApproximateIBL(float3 specColor, float r, float3 n, float3 v)
{
	float nDotV = saturate(dot(n, v));
	float3 dir = 2 * dot(n, v) * n - v;
	uint numLevels = 0;
	uint _ = 0;
	skyboxRadianceMap.GetDimensions(0, _, _, numLevels);
	float2 brdf = cosLookup.Sample(mainSampler, float2(nDotV, 1-r)).rg;
	float3 filteredEnvColor = pow(skyboxRadianceMap.SampleLevel(envSampler, dir, r * numLevels), 2.2);
    return filteredEnvColor * (specColor * brdf.r + brdf.g);
}

float3 colorFromScenelight(GBuffer input)
{
    float3 v = normalize(cameraPosition - input.position);
    float3 l = -normalize(sceneLight.Direction.xyz);
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

    return saturate(dot(input.normal, l)) * (directDiffuse + directSpecular) + indirectDiffuse + indirectSpecular;
}

//Using Lambert azimuthal equal-area projection to encode normals
float3 decompressNormal(float2 compressedNormal)
{
	float n = dot(compressedNormal, compressedNormal) / 4;
	float s = sqrt(1 - n);
	return normalize(float3(s * compressedNormal, n * 2 - 1));
}

float linearizeDepth(float logDepth, float n, float f)
{
	return (2 * n) / (f + n - logDepth * (f - n));
}

float sampleShadowMap(float3 pos, float cascade)
{
	float4 lightspacePos = mul(float4(pos, 1.0f), mul(lightView[cascade], lightProjection[cascade]));
	lightspacePos /= lightspacePos.w;
	float4 shadowCoord = lightspacePos / 2 + 0.5f;
	shadowCoord.y = 1 - shadowCoord.y;
	
	//8x8 PCF
	float sMap = 0.0f;
	for (int y = -3; y <= 3; y++)
	{
		for (int x = -3; x <= 3; x++)
		{
			sMap += shadowMap.SampleCmpLevelZero(shadowSampler, float3(shadowCoord.xy, cascade), lightspacePos.z, int2(x,y));
		}
	}
	sMap /= 64;
	return sMap;
}

//TODO: Use optimization from http://vec3.ca/code/graphics/deferred-shading-tricks/ to reduce size of 
//position buffer
float4 main(VertexToPixel input) : SV_TARGET
{
	GBuffer buffer;
	buffer.albedo = albedoMap.Sample(mainSampler, input.uv);
	buffer.normal = decompressNormal(normalMap.Sample(mainSampler, input.uv).xy);
	buffer.metal = metalnessMap.Sample(mainSampler, input.uv);
	buffer.position = positionMap.Sample(mainSampler, input.uv);
	buffer.roughness = roughnessMap.Sample(mainSampler, input.uv);
	buffer.cavity = cavityMap.Sample(mainSampler, input.uv);

	float linearDepth = linearizeDepth((depth.Sample(mainSampler, input.uv).r), ZNEAR, ZFAR);
	float depthID = floor(linearDepth * NUM_SHADOW_CASCADES);

#if GEN_SHADOW_MAPS == 1
	//Based on MJP's CSM blending implementation
	float distToNextCascade = (ZNEAR + (depthID+1) * SPLIT_SIZE)/ZFAR-linearDepth;
	float shadow = sampleShadowMap(buffer.position, depthID);
	//TODO: Get rid of this branch
	if (distToNextCascade <= 0.1f && depthID < NUM_SHADOW_CASCADES-1)
	{
		float nextShadow = sampleShadowMap(buffer.position, depthID+1);
		float t = smoothstep(0.0f, 0.1f, distToNextCascade);
		shadow = lerp(nextShadow, shadow, t);
	}
#endif
	return float4(pow(colorFromScenelight(buffer), 1/2.2), linearDepth < 1);
}