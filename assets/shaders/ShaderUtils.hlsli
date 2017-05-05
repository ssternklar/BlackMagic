//Using Lambert azimuthal equal-area projection to encode normals
float3 DecompressNormal(float2 compressedNormal)
{
	float n = dot(compressedNormal, compressedNormal) / 4;
	float s = sqrt(1 - n);
	return normalize(float3(s * compressedNormal, n * 2 - 1));
}

float LinearizeDepth(float logDepth, float n, float f)
{
	return (2 * n) / (f + n - logDepth * (f - n));
}