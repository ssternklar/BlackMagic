struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D lightMap : register(t0);
SamplerState mainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return lightMap.Sample(mainSampler, input.uv).rgba;
}