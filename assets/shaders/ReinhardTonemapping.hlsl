struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}