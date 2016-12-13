struct VertexToPixel
{
	float4 position : SV_POSITION;
	float3 textureCoord : POSITION;
};

TextureCube skyboxTex : register(t0);
SamplerState mainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	float3x3 rotation = {
		0, 0, 1.0f,
		1.0f, 0, 0,
		0, 1.0f, 0
	};
	float3 fixedCoord = mul(input.textureCoord, rotation);
	return skyboxTex.Sample(mainSampler, fixedCoord);
}