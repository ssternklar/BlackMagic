
cbuffer frameData
{
	float4 inverseProjection;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 clipPos : MY_CLIP_POS;
	float2 uv : TEXCOORD;
};

VertexToPixel main( float2 input : POSITION )
{
	VertexToPixel output;
	output.position = float4(input, 0.0, 1.0);
	output.clipPos = input * inverseProjection.xy;
	output.uv = ((input + 1) / 2);
	output.uv.y = 1 - output.uv.y;
	return output;
}