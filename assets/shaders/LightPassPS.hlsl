#define NUM_LIGHTS 1
#define NUM_SHADOW_CASCADES 5

struct DirectionalLight
{
	float4 AmbientColor : COLOR;
	float4 DiffuseColor : COLOR;
	float3 Direction	: NORMAL;
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
SamplerState shadowSampler : register(s1);

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

	float3 texColor = input.diffuse;
	return pow((ambient + diffuse + specular) * texColor, 1 / 2.2);
}

//Using Lambert azimuthal equal-area projection to encode normals
float3 decompressNormal(float2 compressedNormal)
{
	float n = dot(compressedNormal, compressedNormal) / 4;
	float s = sqrt(1 - n);
	return float3(s * compressedNormal, n * 2 - 1);
}

float linearizeDepth(float logDepth, float n, float f)
{
	return (2 * n) / (f + n - logDepth * (f - n));

}

//TODO: Use optimization from http://vec3.ca/code/graphics/deferred-shading-tricks/ to reduce size of 
//position buffer
float4 main(VertexToPixel input) : SV_TARGET
{
	GBuffer buffer;
	buffer.diffuse = diffuseMap.Sample(mainSampler, input.uv);
	buffer.specular = specularMap.Sample(mainSampler, input.uv);
	buffer.normal = decompressNormal(normalMap.Sample(mainSampler, input.uv));
	buffer.position = positionMap.Sample(mainSampler, input.uv);

	float depthID = linearizeDepth((depth.Sample(mainSampler, input.uv).r), 0.1f, 100.0f);
	depthID = floor(depthID * NUM_SHADOW_CASCADES);
	float4 lightspacePos = mul(float4(buffer.position, 1.0f), mul(lightView[depthID], lightProjection[depthID]));
	//float4 shadowCoord = lightspacePos / 2 + 0.5f;
	//float visibility = ceil(shadowMap.Sample(shadowSampler, float3(shadowCoord.xy, depthID)).r - shadowCoord.z)/2 + 0.5;

	return float4(lightspacePos.xyy, 1.0f); //float4(visibility*colorFromScenelight(buffer), 1.0f); 
}