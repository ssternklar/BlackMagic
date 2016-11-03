struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VertexToPixel main( float2 input : POSITION )
{
	VertexToPixel output;
	output.position = float4(input, 0.0, 1.0);
	output.uv = ((input + 1) / 2);
	output.uv.y = 1 - output.uv.y;
	return output;
}