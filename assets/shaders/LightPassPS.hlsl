#define NUM_LIGHTS 1
#define NUM_SHADOW_CASCADES 5
#define ZNEAR 0.1f
#define ZFAR 100.0f
#define SPLIT_SIZE ((ZFAR - ZNEAR)/NUM_SHADOW_CASCADES)
#define GEN_SHADOW_MAPS 0

struct DirectionalLight
{
	float4 AmbientColor : COLOR;
	float4 DiffuseColor : COLOR;
	float3 Direction	: NORMAL;
	float padding;
	float3 Up			: NORMAL;
};

struct GBuffer
{
	float4 diffuse;
	float4 specular;
	float3 position;
	float3 normal;
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

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D normalMap : register(t3);
Texture2DArray shadowMap : register(t4);
Texture2D depth : register(t5);
SamplerState mainSampler : register(s0);
SamplerComparisonState shadowSampler : register(s1);


float3 colorFromScenelight(GBuffer input)
{
	float3 ambient, diffuse, specular;
	ambient = diffuse = specular = float3(0, 0, 0);

	float3 v = normalize(cameraPosition - input.position);
	
	float3 l = -normalize(sceneLight.Direction);
	float nDotL = saturate(dot(input.normal, l));
	diffuse += nDotL * sceneLight.DiffuseColor.xyz;

	float3 h = normalize(l + v);
	float spec = pow(max(dot(input.normal, h), 0), 32);
	specular += spec * input.specular.rgb;

	ambient += sceneLight.AmbientColor.xyz;

	float3 texColor = input.diffuse.rgb;
	return pow((ambient + diffuse + specular)*texColor, 1 / 2.2);
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
	buffer.diffuse = diffuseMap.Sample(mainSampler, input.uv);
	buffer.specular = specularMap.Sample(mainSampler, input.uv);
	buffer.normal = decompressNormal(normalMap.Sample(mainSampler, input.uv).xy);
	buffer.position = positionMap.Sample(mainSampler, input.uv);

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
	return float4(colorFromScenelight(buffer) , 1.0f);
}