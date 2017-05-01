struct VertexToPixel
{
	float4 position : SV_POSITION;
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 uv : TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(input.normal.xyz, 1);
}