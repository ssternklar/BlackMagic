

struct Vertex
{ 
	float3 position	: POSITION;  
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 uv : TEXCOORD;
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

cbuffer frameData
{
	matrix view;
	matrix projection;
};

cbuffer instanceData
{
	matrix world;
};

VertexToPixel main( Vertex input )
{
	matrix mv = mul(world, view);
	matrix mvp = mul(mv, projection);
	float3x3 rs = (float3x3) world;

	VertexToPixel output;
	output.position = mul(float4(input.position, 1.0), mvp);
	output.worldPos = mul(float4(input.position, 1.0), world).xyz;
	output.normal = mul(input.normal, rs);
	output.tangent = mul(input.tangent, rs);
	output.binormal = mul(input.binormal, rs);
	output.uv = input.uv;
	return output;
}