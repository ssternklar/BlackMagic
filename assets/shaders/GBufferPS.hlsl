
Texture2D mainTex : register(t0);
Texture2D normalMap : register(t1);
SamplerState mainSampler : register(s0);

struct GBuffer
{
	float4 diffuse : SV_TARGET0;
	float4 specular : SV_TARGET1;
	float3 normal : SV_TARGET2;
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

GBuffer main(VertexToPixel input) : SV_TARGET
{
	GBuffer output;

	output.diffuse = float4(mainTex.Sample(mainSampler, input.uv).rgb, 1.0);

	output.specular = float4(1, 1, 1, 32);

	float3x3 tbn = float3x3(input.tangent, input.binormal, input.normal);
	input.normal = normalMap.Sample(mainSampler, input.uv) * 2 - 1;
	output.normal = mul(input.normal, tbn);

	return output;
}