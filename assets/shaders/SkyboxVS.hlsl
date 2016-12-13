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
	float3 textureCoord : POSITION;
};

cbuffer PerFrame : register(b0)
{
	float3 camPos;
	matrix view;
	matrix proj;
}

VertexToPixel main(Vertex input)
{
	VertexToPixel output;
	output.position = mul(float4(input.position + camPos, 1.0f), mul(view, proj)).xyww;
	output.textureCoord = input.position;
	return output;
}
