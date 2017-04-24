struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D lightMap : register(t0);
SamplerState mainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color = lightMap.Sample(mainSampler, input.uv).rgba;
	float3 tonemappedColor = color.rgb / (color.rgb + 1.0);
	return float4(pow(tonemappedColor, 1 / 2.2), color.a);
}