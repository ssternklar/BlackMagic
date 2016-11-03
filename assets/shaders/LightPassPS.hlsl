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
	float depth;
};


struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 clipPos : MY_CLIP_POS;
	float2 uv : TEXCOORD;
};

cbuffer perFrame : register(b0)
{
	DirectionalLight directionalLights[NUM_LIGHTS];
	float4 inverseProjection;
};

Texture2D diffuseMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D depth : register(t3);
SamplerState mainSampler : register(s0);

/*float3 colorFromDirectionalLights(VertexToPixel input)
{
	float3 ambient, diffuse, specular;
	ambient = diffuse = specular = float3(0, 0, 0);

	float3 v = normalize(cameraPos - input.worldPos);
	for (uint i = 0; i < 1; i++)
	{
		float3 l = -normalize(directionalLights[i].Direction);
		float nDotL = saturate(dot(input.normal, l));
		diffuse += nDotL * directionalLights[i].DiffuseColor.xyz;

		float3 h = normalize(l + v);
		float spec = pow(max(dot(input.normal, h), 0), 64);
		specular += spec * float3(1, 1, 1) * nDotL;

		ambient += directionalLights[i].AmbientColor.xyz;
	}

	float3 texColor = pow(mainTex.Sample(mainSampler, input.uv), 2.2);
	return pow((ambient + diffuse) * texColor + specular, 1 / 2.2);
	
}
	 */


float4 main(VertexToPixel input) : SV_TARGET
{
	GBuffer buffer;
	buffer.diffuse = diffuseMap.Sample(mainSampler, input.uv);
	buffer.specular = diffuseMap.Sample(mainSampler, input.uv);
	buffer.normal = float3(normalMap.Sample(mainSampler, input.uv).xy, 0.0);
	buffer.normal.z = sqrt(1 - dot(buffer.normal.xy, buffer.normal.xy));

	float clipDepth = depth.Sample(mainSampler, input.uv).r;
	//float3 pos = float3(input.clipPos, -1) / (clipDepth * inverseProjection.z + inverseProjection.w);

	return float4(clipDepth, 0.0, 0.0, 1.0);
}