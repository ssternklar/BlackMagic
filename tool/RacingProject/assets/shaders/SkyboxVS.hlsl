struct Vertex
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 binormal : BINORMAL;
	float4 uv : TEXCOORD;
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
	output.position = mul(float4(input.position.xyz + camPos, 1.0f), mul(view, proj)).xyww;
	output.textureCoord = input.position.xyz;
	return output;
}
