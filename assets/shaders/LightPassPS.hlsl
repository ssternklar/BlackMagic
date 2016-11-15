#define NUM_LIGHTS 1

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
	matrix lightProjection;
	matrix lightView;
	DirectionalLight sceneLight;
	float3 cameraPosition;
};

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D normalMap : register(t3);
Texture2D shadowMap : register(t4);
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

	float4 lightspacePos = mul(float4(input.position, 1.0f), mul(lightView, lightProjection));
	lightspacePos /= lightspacePos.w;


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

//TODO: Use optimization from http://vec3.ca/code/graphics/deferred-shading-tricks/ to reduce size of 
//position buffer
float4 main(VertexToPixel input) : SV_TARGET
{
	GBuffer buffer;
	buffer.diffuse = diffuseMap.Sample(mainSampler, input.uv);
	buffer.specular = specularMap.Sample(mainSampler, input.uv);
	buffer.normal = decompressNormal(normalMap.Sample(mainSampler, input.uv));
	buffer.position = positionMap.Sample(mainSampler, input.uv);

	float4 lightspacePos = mul(float4(buffer.position, 1.0f), mul(lightView, lightProjection));
	float4 shadowCoord = lightspacePos / 2 + 0.5f;
	float visibility = ceil(shadowMap.Sample(shadowSampler, shadowCoord.xy).r - shadowCoord.z)/2 + 0.5;

	return float4(shadowMap.Sample(shadowSampler, shadowCoord.xy).r * float3(1, 1, 1), 1.0f); //float4(visibility*colorFromScenelight(buffer), 1.0f); 
}