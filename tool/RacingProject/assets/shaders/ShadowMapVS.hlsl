cbuffer PerFrame : register(b0)
{
	matrix view, projection;
}

cbuffer PerInstance : register(b1)
{
	matrix model;
}

struct Vertex
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 uv : TEXCOORD;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
};

VertexToPixel main(Vertex input)
{
	VertexToPixel output;
	output.position = mul(float4(input.position, 1.0), mul(model, mul(view, projection)));
	return output;
}