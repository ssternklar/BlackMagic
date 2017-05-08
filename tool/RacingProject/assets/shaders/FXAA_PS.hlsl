// FXAA Implementation based on https://github.com/mattdesl/glsl-fxaa/blob/master/fxaa.glsl

#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_REDUCE_MUL (1.0 / 8.0)
#define FXAA_SPAN_MAX 8.0

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D inputMap : register(t0);
SamplerState mainSampler : register(s0);

cbuffer perFrame : register(b0)
{
	int width;
	int height;
};

float calcLuma(float3 rgb)
{
	return rgb.y * (0.587 / 0.299) + rgb.x;
}

float2 calcOffset(float2 pix, float2 dir)
{
	return (pix + dir) * float2(1.0 / width, 1.0 / height);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float4 color;

	float2 pixPos = input.uv * float2(width, height);

	float inverseVP = float2(1.0 / width, 1.0 / height);

	float2 cNW = calcOffset(pixPos, float2(-1.0, -1.0));
	float2 cNE = calcOffset(pixPos, float2(1.0, -1.0));
	float2 cSW = calcOffset(pixPos, float2(-1.0, 1.0));
	float2 cSE = calcOffset(pixPos, float2(1.0, 1.0));
	float2 cM = input.uv;

	float3 colNW = inputMap.Sample(mainSampler, cNW).xyz;
	float3 colNE = inputMap.Sample(mainSampler, cNE).xyz;
	float3 colSW = inputMap.Sample(mainSampler, cSW).xyz;
	float3 colSE = inputMap.Sample(mainSampler, cSE).xyz;
	float4 texColor = inputMap.Sample(mainSampler, cM);
	float3 colM = texColor.xyz;

	float3 luma = float3(0.299, 0.587, 0.114);
	float lumaNW = dot(colNW, luma);
	float lumaNE = dot(colNE, luma);
	float lumaSW = dot(colSW, luma);
	float lumaSE = dot(colSE, luma);
	float lumaM = dot(colM, luma);

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	float2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseVP;

	float3 rgbA = 0.5 * (
		inputMap.Sample(mainSampler, input.uv + dir * (1.0 / 3.0 - 0.5)).xyz +
		inputMap.Sample(mainSampler, input.uv + dir * (2.0 / 3.0 - 0.5)).xyz
		);
	float3 rgbB = rgbA * 0.5 + 0.25 * (
		inputMap.Sample(mainSampler, input.uv + dir * -0.5).xyz +
		inputMap.Sample(mainSampler, input.uv + dir * 0.5).xyz
		);

	float lumaB = dot(rgbB, luma);
	if (lumaB < lumaMin || lumaB > lumaMax)
		color = float4(rgbA, texColor.a);
	else
		color = float4(rgbB, texColor.a);

	return color;
}