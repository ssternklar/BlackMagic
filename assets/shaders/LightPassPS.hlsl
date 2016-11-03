#define NUM_LIGHTS 2

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
	DirectionalLight directionalLights[NUM_LIGHTS];
	float3 cameraPosition;
};

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D positionMap : register(t2);
Texture2D normalMap : register(t3);
SamplerState mainSampler : register(s0);

float3 colorFromDirectionalLights(GBuffer input)
{
	float3 ambient, diffuse, specular;
	ambient = diffuse = specular = float3(0, 0, 0);

	float3 v = normalize(cameraPosition - input.position);
	for (uint i = 0; i < 1; i++)
	{
		float3 l = -normalize(directionalLights[i].Direction);
		float nDotL = saturate(dot(input.normal, l));
		diffuse += nDotL * directionalLights[i].DiffuseColor.xyz;

		float3 h = normalize(l + v);
		float spec = pow(max(dot(input.normal, h), 0), 32);
		specular += spec * input.specular.rgb;

		ambient += directionalLights[i].AmbientColor.xyz;
	}

	float3 texColor = input.diffuse;
	return pow((ambient + diffuse) * texColor + specular, 1 / 2.2);
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

	return float4(colorFromDirectionalLights(buffer), 1.0);
}