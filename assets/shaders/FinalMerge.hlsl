struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D albedoMap : register(t0);
Texture2D positionMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D normalMap : register(t3);
Texture2D depth : register(t5);
Texture2D metalnessMap : register(t6);
Texture2D cavityMap : register(t7);
SamplerState mainSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return albedoMap.Sample(mainSampler, input.uv).rgba;
}