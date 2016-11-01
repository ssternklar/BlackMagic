
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
	float3 cameraPos;
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
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 uv : TEXCOORD;
};

float3 colorFromDirectionalLights(VertexToPixel input, float2 uv)
{
	float3 ambient, diffuse, specular;
	ambient = diffuse = specular = float3(0, 0, 0);

	float3 v = normalize(cameraPos - input.worldPos);
	for (uint i = 0; i < 1; i++)
	{
		float3 l = -normalize(directionalLights[i].Direction);
		float diff = saturate(dot(input.normal, l));
		diffuse += diff * directionalLights[i].DiffuseColor.xyz;

		float3 h = normalize(l + v);
		float spec = pow(max(dot(input.normal, h), 0), 64);
		specular += spec * float3(1, 1, 1);

		ambient += directionalLights[i].AmbientColor.xyz;
	}

	float3 texColor = pow(mainTex.Sample(mainSampler, uv), 2.2);
	return pow((ambient + diffuse) * texColor + specular, 1 / 2.2);
	
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float3x3 tbn = float3x3(
		normalize(input.tangent), 
		normalize(input.binormal),
		normalize(input.normal)
	);

	input.normal = (normalMap.Sample(mainSampler, input.uv).rgb * 2 - 1);
	input.normal = normalize(mul(input.normal, tbn));

	float3 finalColor = colorFromDirectionalLights(input, input.uv);

	return float4(finalColor, 1.0);
}