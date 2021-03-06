
Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D metalnessMap : register(t3);
SamplerState mainSampler: register(s0);

struct GBuffer
{
	float4 albedo : SV_TARGET0;
	float3 position : SV_TARGET1;
	float roughness : SV_TARGET2;
	float2 normal : SV_TARGET3;
	float cavity : SV_TARGET4;
	float metal : SV_TARGET5;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 uv : TEXCOORD;
};

GBuffer main(VertexToPixel input)
{
	GBuffer output;
	output.albedo = float4(albedoMap.Sample(mainSampler, input.uv).rgb, 1.0);

	float3x3 tbn = float3x3(
		normalize(input.tangent), 
		normalize(input.binormal), 
		normalize(input.normal));
	input.normal = normalMap.Sample(mainSampler, input.uv) * 2 - 1;
	input.normal = normalize(mul(normalize(input.normal), tbn));

	float f = sqrt(2 / (1 - input.normal.z));
	output.normal = input.normal.xy * f;

	output.position = input.worldPos;

	output.roughness = roughnessMap.Sample(mainSampler, input.uv);
	output.cavity = 1.0f;
	output.metal = metalnessMap.Sample(mainSampler, input.uv);

	return output;
}