
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage

#define NUM_LIGHTS 2

struct DirectionalLight
{
	float4 AmbientColor : COLOR;
	float4 DiffuseColor : COLOR;
	float3 Direction	: NORMAL;
};

cbuffer externalData : register(b0)
{
	DirectionalLight directionalLights[NUM_LIGHTS];
	uint UseNormalMap;
};

Texture2D mainTex : register(t0);
Texture2D normalMap : register(t1);
SamplerState mainSampler : register(s0);

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position : SV_POSITION; // XYZW position (System Value Position)
	float4 worldPos : POSITIONT;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 uv : TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 color = float3(0,0,0);
	float3 ambient = float3(0,0,0);
	float3 specColor = float3(0, 0, 0);
	float3 texColor = pow(mainTex.Sample(mainSampler, input.uv), 2.2);

	float3x3 tbn = float3x3(input.tangent, input.binormal, input.normal);
	float3 normal = normalMap.Sample(mainSampler, input.uv);
	normal = normal * 2 - 1;
	normal = mul(normal, tbn);

	if (UseNormalMap == 0)
		normal = input.normal;

	float3 v = -normalize(input.worldPos);
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		float3 l = -normalize(directionalLights[i].Direction);
		float nDotL = saturate(dot(normal, l));

		float3 h = (l + v)/2;
		float angle = max(dot(normal, h), 0.0);
		float spec = pow(angle, 128);

		color += nDotL * directionalLights[i].DiffuseColor.rgb;
		specColor += spec * float3(1, 1, 1);
		ambient += directionalLights[i].AmbientColor.rgb;
	}

	ambient /= NUM_LIGHTS;
	color /= NUM_LIGHTS;

	return float4(pow((ambient+color)*texColor, 1/2.2), 1.0);
}