struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D projectedTex : register(t0);
Texture2D positionMap : register(t1);
SamplerState mainSampler : register(s0);

cbuffer PerInstance : register(b0)
{
	matrix vp;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 pos = positionMap.Sample(mainSampler, input.uv);
	float4 projectedPos = mul(float4(pos, 1.0f), vp);
	float2 uv = projectedPos.xy / projectedPos.w;
	uv.xy = uv.xy * 0.5f + 0.5f;
	uv.y = 1 - uv.y;
	uv.xy = uv.xy / 2;
	float2 inBounds = uv >= 0.0f && uv <= 1.0f;
	return float4(projectedTex.Sample(mainSampler, uv).rgb, saturate(dot(inBounds, inBounds) - 1));

}