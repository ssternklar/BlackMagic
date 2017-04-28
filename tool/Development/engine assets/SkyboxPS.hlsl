struct VertexToPixel
{
	float4 position : SV_POSITION;
	float3 textureCoord : POSITION;
};

TextureCube skyboxTex : register(t0);
SamplerState mainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return skyboxTex.Sample(mainSampler, input.textureCoord);
}